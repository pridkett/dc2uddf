/*
 * Decoder for the warning/alarm bits in Uwatec Smart dive records.
 *
 * The sample walk below is a faithful port of uwatec_smart_parse() from
 * libdivecomputer v0.9.0 (src/uwatec_smart_parser.c, LGPL-2.1), restricted
 * to the models that share the Galileo sample bitstream. It exists because
 * libdivecomputer decodes the per-sample event bits but discards
 * EV_WARNING/EV_ALARM/EV_WORKLOAD_WARNING before the sample callback; a
 * long-term fix would be an upstream patch emitting them as SAMPLE_EVENTs,
 * at which point this module can be deleted.
 *
 * See llm/libdivecomputer-uwatec-galileo.md for the bitstream description.
 */

#include <glib.h>
#include "uwatec_smart_alarms.h"

/* libdivecomputer model numbers sharing the Galileo sample bitstream */
#define GALILEO           0x11
#define ALADIN2G          0x15
#define ALADINSPORTMATRIX 0x17
#define GALILEOTRIMIX     0x19
#define MERIDIAN          0x20
#define ALADINSQUARE      0x22
#define CHROMIS           0x24
#define ALADINA1          0x25
#define MANTIS2           0x26
#define ALADINA2          0x28
#define G2TEK             0x31
#define G2                0x32
#define G3                0x34
#define G2HUD             0x42
#define LUNA2AI           0x50
#define LUNA2             0x51

#define NBITS 8

#define HEADERSIZE_GALILEO 152
#define HEADERSIZE_TRIMIX   84

/* offset of the settings word within each header layout */
#define SETTINGS_GALILEO 92
#define SETTINGS_TRIMIX  68
#define FREEDIVE 0x00000080

/* sample types, as in uwatec_smart_parser.c (only the ones the walk needs
 * to treat specially; the rest just advance the offset) */
typedef enum {
    SAMPLE_RBT,
    SAMPLE_TEMPERATURE,
    SAMPLE_PRESSURE,
    SAMPLE_DEPTH,
    SAMPLE_HEARTRATE,
    SAMPLE_BEARING,
    SAMPLE_ALARMS,
    SAMPLE_TIME,
    SAMPLE_APNEA,
    SAMPLE_MISC
} sample_type_t;

typedef struct sample_info_t {
    sample_type_t type;
    guint absolute;
    guint index;      /* alarm byte index for SAMPLE_ALARMS */
    guint ntypebits;
    guint ignoretype;
    guint extrabytes;
} sample_info_t;

/* uwatec_smart_galileo_samples, verbatim from libdivecomputer v0.9.0 */
static const sample_info_t galileo_samples[] = {
    {SAMPLE_DEPTH,       0, 0, 1, 0, 0}, // 0ddd dddd
    {SAMPLE_RBT,         0, 0, 3, 0, 0}, // 100d dddd
    {SAMPLE_PRESSURE,    0, 0, 4, 0, 0}, // 1010 dddd
    {SAMPLE_TEMPERATURE, 0, 0, 4, 0, 0}, // 1011 dddd
    {SAMPLE_TIME,        1, 0, 4, 0, 0}, // 1100 dddd
    {SAMPLE_HEARTRATE,   0, 0, 4, 0, 0}, // 1101 dddd
    {SAMPLE_ALARMS,      1, 0, 4, 0, 0}, // 1110 dddd
    {SAMPLE_ALARMS,      1, 1, 8, 0, 1}, // 1111 0000 dddddddd
    {SAMPLE_DEPTH,       1, 0, 8, 0, 2}, // 1111 0001 dddddddd dddddddd
    {SAMPLE_RBT,         1, 0, 8, 0, 1}, // 1111 0010 dddddddd
    {SAMPLE_TEMPERATURE, 1, 0, 8, 0, 2}, // 1111 0011 dddddddd dddddddd
    {SAMPLE_PRESSURE,    1, 0, 8, 0, 2}, // 1111 0100 dddddddd dddddddd
    {SAMPLE_PRESSURE,    1, 1, 8, 0, 2}, // 1111 0101 dddddddd dddddddd
    {SAMPLE_PRESSURE,    1, 2, 8, 0, 2}, // 1111 0110 dddddddd dddddddd
    {SAMPLE_HEARTRATE,   1, 0, 8, 0, 1}, // 1111 0111 dddddddd
    {SAMPLE_BEARING,     1, 0, 8, 0, 2}, // 1111 1000 dddddddd dddddddd
    {SAMPLE_ALARMS,      1, 2, 8, 0, 1}, // 1111 1001 dddddddd
    {SAMPLE_APNEA,       1, 0, 8, 0, 0}, // 1111 1010 (8 bytes)
    {SAMPLE_MISC,        1, 0, 8, 0, 1}, // 1111 1011 dddddddd (n-1 bytes)
};

GQuark uwatec_smart_alarms_error_quark(void) {
    return g_quark_from_static_string("uwatec-smart-alarms-error-quark");
}

/* uwatec_galileo_identify, verbatim from libdivecomputer v0.9.0 */
static guint _galileo_identify(guint8 value) {
    // Bits: 0ddd dddd
    if ((value & 0x80) == 0)
        return 0;

    // Bits: 100d dddd
    if ((value & 0xE0) == 0x80)
        return 1;

    // Bits: 1XXX dddd
    if ((value & 0xF0) != 0xF0)
        return (value & 0x70) >> 4;

    // Bits: 1111 XXXX
    return (value & 0x0F) + 7;
}

/**
 * decode the alarm bits from a raw Uwatec Smart (Galileo bitstream) record
 *
 * Walks the sample stream exactly as libdivecomputer's uwatec_smart_parse()
 * does, but instead of dropping the warning/alarm/workload-warning bits it
 * reports them per emitted waypoint. Mirroring upstream's bookmark handling,
 * the bits are treated as state: they stay in effect until the next
 * events-sample changes them, so a long warning is reported at every
 * waypoint it spans.
 */
GArray *uwatec_smart_alarms_decode(const guint8 *data, gsize size,
                                   guint model, GError **err) {
    guint headersize;
    guint settingsOffset;

    switch (model) {
    case GALILEO:
    case GALILEOTRIMIX:
        /* the Galileo family uses the trimix header layout when the
         * trimix bit in the header is set */
        if (size < 44) {
            g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                        UWATEC_SMART_ALARMS_ERROR_TRUNCATED,
                        "record too short (%" G_GSIZE_FORMAT " bytes) for a Galileo header", size);
            return NULL;
        }
        if (data[43] & 0x80) {
            headersize = HEADERSIZE_TRIMIX;
            settingsOffset = SETTINGS_TRIMIX;
        } else {
            headersize = HEADERSIZE_GALILEO;
            settingsOffset = SETTINGS_GALILEO;
        }
        break;
    case ALADIN2G:
    case MERIDIAN:
    case CHROMIS:
    case MANTIS2:
    case ALADINSQUARE:
        headersize = HEADERSIZE_GALILEO;
        settingsOffset = SETTINGS_GALILEO;
        break;
    case G2:
    case G2HUD:
    case G2TEK:
    case G3:
    case ALADINSPORTMATRIX:
    case ALADINA1:
    case ALADINA2:
    case LUNA2AI:
    case LUNA2:
        headersize = HEADERSIZE_TRIMIX;
        settingsOffset = SETTINGS_TRIMIX;
        break;
    default:
        /* Smart Pro/Com/Tec/Z and Aladin Tec use different bitstreams
         * that this decoder does not (yet) implement */
        g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                    UWATEC_SMART_ALARMS_ERROR_UNSUPPORTED_MODEL,
                    "model 0x%02X does not use the Galileo sample bitstream", model);
        return NULL;
    }

    if (size < headersize) {
        g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                    UWATEC_SMART_ALARMS_ERROR_TRUNCATED,
                    "record too short (%" G_GSIZE_FORMAT " bytes) for header size %u",
                    size, headersize);
        return NULL;
    }

    /* freedives sample every second instead of every four */
    guint interval = 4;
    if (settingsOffset + 4 <= size) {
        guint32 settings = data[settingsOffset]
            | (data[settingsOffset + 1] << 8)
            | (data[settingsOffset + 2] << 16)
            | ((guint32) data[settingsOffset + 3] << 24);
        if (settings & FREEDIVE) {
            interval = 1;
        }
    }

    GArray *result = g_array_new(FALSE, FALSE, sizeof(uwatec_alarm_event_t));

    const guint entries = G_N_ELEMENTS(galileo_samples);
    const sample_info_t *table = galileo_samples;

    guint time = 0;
    guint complete = 0;
    guint8 alarms = 0;     /* current UWATEC_ALARM_* state */

    gsize offset = headersize;
    while (offset < size) {
        guint id = _galileo_identify(data[offset]);
        if (id >= entries) {
            g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                        UWATEC_SMART_ALARMS_ERROR_INVALID_DATA,
                        "invalid type bits 0x%02X at offset %" G_GSIZE_FORMAT,
                        data[offset], offset);
            g_array_free(result, TRUE);
            return NULL;
        }

        /* skip the processed type bytes */
        offset += table[id].ntypebits / NBITS;

        /* process the data bits sharing the last type byte */
        guint value = 0;
        guint n = table[id].ntypebits % NBITS;
        if (n > 0) {
            value = data[offset] & (0xFF >> n);
            if (table[id].ignoretype) {
                value = 0;
            }
            offset++;
        }

        if (offset + table[id].extrabytes > size) {
            g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                        UWATEC_SMART_ALARMS_ERROR_TRUNCATED,
                        "incomplete sample data at offset %" G_GSIZE_FORMAT, offset);
            g_array_free(result, TRUE);
            return NULL;
        }

        for (guint i = 0; i < table[id].extrabytes; i++) {
            value <<= NBITS;
            value += data[offset];
            offset++;
        }

        switch (table[id].type) {
        case SAMPLE_ALARMS:
            /* galileo_events_0: 0x01 warning, 0x02 alarm, 0x04 workload
             * warning (0x08 is bookmark, 0x60 in byte 1 is gasmix - both
             * already delivered through libdivecomputer's sample callback,
             * so they are deliberately not decoded here) */
            if (table[id].index == 0) {
                alarms = value & (UWATEC_ALARM_WARNING |
                                  UWATEC_ALARM_ALARM |
                                  UWATEC_ALARM_WORKLOAD_WARNING);
            }
            break;
        case SAMPLE_DEPTH:
            complete = 1;
            break;
        case SAMPLE_TIME:
            complete = value;
            break;
        case SAMPLE_APNEA:
            if (offset + 8 > size) {
                g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                            UWATEC_SMART_ALARMS_ERROR_TRUNCATED,
                            "incomplete apnea sample at offset %" G_GSIZE_FORMAT, offset);
                g_array_free(result, TRUE);
                return NULL;
            }
            offset += 8;
            break;
        case SAMPLE_MISC:
            if (value < 1 || offset + value - 1 > size) {
                g_set_error(err, UWATEC_SMART_ALARMS_ERROR,
                            UWATEC_SMART_ALARMS_ERROR_TRUNCATED,
                            "incomplete misc sample at offset %" G_GSIZE_FORMAT, offset);
                g_array_free(result, TRUE);
                return NULL;
            }
            offset += value - 1;
            break;
        default:
            /* RBT/temperature/pressure/heartrate/bearing values are not
             * needed here; the offset bookkeeping above already skipped
             * their data bytes */
            break;
        }

        while (complete) {
            if (alarms != 0) {
                uwatec_alarm_event_t event;
                event.timestamp = time;
                event.alarms = alarms;
                g_array_append_val(result, event);
            }
            time += interval;
            complete--;
        }
    }

    return result;
}
