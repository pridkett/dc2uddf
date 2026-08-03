#ifndef DC2UDDF_UWATEC_SMART_ALARMS_H
#define DC2UDDF_UWATEC_SMART_ALARMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <glib.h>

/*
 * Decoder for the warning/alarm bits in Uwatec Smart dive records.
 *
 * libdivecomputer (up to and including 0.9 and current master) decodes
 * these bits inside uwatec_smart_parser.c but silently discards
 * EV_WARNING, EV_ALARM and EV_WORKLOAD_WARNING - only bookmarks and gas
 * switches reach the sample callback. Until upstream grows a way to emit
 * them (the clean fix would be a patch mapping them to SAMPLE_EVENTs),
 * this module re-walks the raw dive record and extracts just those bits.
 *
 * The sample-stream walk mirrors uwatec_smart_parse() from libdivecomputer
 * v0.9.0 exactly (same tables, same time-advance rule) so the reported
 * timestamps line up with the waypoints libdivecomputer produces from the
 * same bytes. glib-only, so it is unit-testable in check_dif.
 */

/** Warning (yellow buzzer) */
#define UWATEC_ALARM_WARNING            0x01
/** Alarm (red buzzer) */
#define UWATEC_ALARM_ALARM              0x02
/** Increased workload (lung symbol) */
#define UWATEC_ALARM_WORKLOAD_WARNING   0x04

typedef struct uwatec_alarm_event_t {
    guint timestamp;   /**< Seconds since the start of the dive */
    guint8 alarms;     /**< Bitmask of UWATEC_ALARM_* active at this waypoint */
} uwatec_alarm_event_t;

#define UWATEC_SMART_ALARMS_ERROR uwatec_smart_alarms_error_quark()
GQuark uwatec_smart_alarms_error_quark(void);

typedef enum {
    UWATEC_SMART_ALARMS_ERROR_UNSUPPORTED_MODEL,
    UWATEC_SMART_ALARMS_ERROR_TRUNCATED,
    UWATEC_SMART_ALARMS_ERROR_INVALID_DATA
} UwatecSmartAlarmsError;

/**
 * Decode the alarm bits from a raw Uwatec Smart dive record.
 *
 * @param data  A full dive record as passed to the libdivecomputer dive
 *              callback (A5A5 5A5A framing + LE32 length + header + samples).
 * @param size  Size of the record in bytes.
 * @param model libdivecomputer model number (e.g. 0x11 Galileo). Only models
 *              sharing the Galileo sample bitstream are supported.
 * @param err   Set on failure (unsupported model, truncated record,
 *              invalid type bits).
 * @return A GArray of uwatec_alarm_event_t (possibly empty; free with
 *         g_array_free(..., TRUE)), or NULL with err set.
 */
GArray *uwatec_smart_alarms_decode(const guint8 *data, gsize size,
                                   guint model, GError **err);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DC2UDDF_UWATEC_SMART_ALARMS_H */
