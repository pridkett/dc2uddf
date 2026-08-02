#include <string.h>

#include "dumpfile.h"

#define UWATEC_SMART_HEADER_SIZE 8

static const guint8 UWATEC_SMART_MAGIC[4] = {0xA5, 0xA5, 0x5A, 0x5A};

GQuark dumpfile_error_quark(void) {
    return g_quark_from_static_string("dumpfile-error-quark");
}

gint dumpfile_foreach_uwatec_smart(const guint8 *buf, gsize len,
                                   dumpfile_record_fn cb, gpointer userdata,
                                   GError **err) {
    gsize offset = 0;
    guint index = 0;

    if (buf == NULL || len == 0) {
        g_set_error(err, DUMPFILE_ERROR, DUMPFILE_ERROR_EMPTY,
                    "dump file is empty");
        return -1;
    }

    while (offset < len) {
        if (len - offset < UWATEC_SMART_HEADER_SIZE ||
            memcmp(buf + offset, UWATEC_SMART_MAGIC,
                   sizeof(UWATEC_SMART_MAGIC)) != 0) {
            if (offset == 0) {
                g_set_error(err, DUMPFILE_ERROR, DUMPFILE_ERROR_BAD_MAGIC,
                            "file does not look like a Uwatec Smart dive dump "
                            "(no A5A55A5A record header at start); note that "
                            "raw memory images (e.g. from --dump-memory) are a "
                            "different format and cannot be replayed");
            } else {
                g_set_error(err, DUMPFILE_ERROR, DUMPFILE_ERROR_BAD_MAGIC,
                            "invalid record header at byte offset %" G_GSIZE_FORMAT
                            " (after %u valid records)", offset, index);
            }
            return -1;
        }

        guint32 length = (guint32)buf[offset + 4] |
                         ((guint32)buf[offset + 5] << 8) |
                         ((guint32)buf[offset + 6] << 16) |
                         ((guint32)buf[offset + 7] << 24);

        if (length <= UWATEC_SMART_HEADER_SIZE || length > len - offset) {
            g_set_error(err, DUMPFILE_ERROR, DUMPFILE_ERROR_BAD_LENGTH,
                        "invalid record length %u at byte offset %" G_GSIZE_FORMAT
                        " (%" G_GSIZE_FORMAT " bytes remaining)",
                        length, offset, len - offset);
            return -1;
        }

        if (!cb(buf + offset, length, index, userdata)) {
            return index + 1;
        }

        offset += length;
        index++;
    }

    return index;
}
