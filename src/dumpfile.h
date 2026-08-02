#ifndef DUMPFILE_H
#define DUMPFILE_H

#include <glib.h>

/**
 * Splitting of on-disk dive-data dump files into individual dive records.
 *
 * A Uwatec Smart dive-data dump is a sequence of back-to-back records,
 * each framed as:
 *
 *   [A5 A5 5A 5A][uint32 LE length][payload]
 *
 * where the length INCLUDES the 8-byte header. This is the exact byte
 * stream the device transfers and that libdivecomputer's dive callback
 * receives per dive, so concatenating live dive records reproduces the
 * format and splitting a dump recovers them.
 */

#define DUMPFILE_ERROR dumpfile_error_quark()

typedef enum {
    DUMPFILE_ERROR_EMPTY,
    DUMPFILE_ERROR_BAD_MAGIC,
    DUMPFILE_ERROR_BAD_LENGTH
} DumpfileError;

GQuark dumpfile_error_quark(void);

/**
 * Callback invoked for each dive record found in a dump.
 *
 * @param record: pointer to the FULL record, including the 8-byte header
 *                (this matches what libdivecomputer's dive callback gets)
 * @param size: record size in bytes, including the header
 * @param index: zero-based record index within the dump
 * @param userdata: opaque pointer passed through from the caller
 * @return TRUE to continue iterating, FALSE to stop early (not an error)
 */
typedef gboolean (*dumpfile_record_fn)(const guint8 *record, gsize size,
                                       guint index, gpointer userdata);

/**
 * Iterates over all Uwatec Smart dive records in a memory buffer.
 *
 * @return the number of records visited, or -1 with *err set if the
 *         buffer does not conform to the framing
 */
gint dumpfile_foreach_uwatec_smart(const guint8 *buf, gsize len,
                                   dumpfile_record_fn cb, gpointer userdata,
                                   GError **err);

#endif /* DUMPFILE_H */
