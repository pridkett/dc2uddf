#ifndef DC2UDDF_DIF_H
#define DC2UDDF_DIF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <glib.h>

/**
 * a dive collection is used for a set of dives downloaded from a comuter
 */
typedef struct dif_dive_collection_t {
    GList *dives; /* a list of dif_dive_t */
} dif_dive_collection_t;

/**
 * a single dive consists of a sequence of samples
 */
typedef struct dif_dive_t {
    guint year, month, day, hour, minute, second;
    guint duration;
    gdouble maxdepth;
    GList *gasmixes; /* a list of dif_gasmix_t */
    GList *samples; /* a list of dif_sample_t */
} dif_dive_t;

typedef struct dif_gasmix_t {
    guint id;
    gdouble helium;
    gdouble oxygen;
    gdouble nitrogen;
    gdouble argon;
    gdouble hydrogen;
} dif_gasmix_t;

typedef enum dif_sample_type_t {
    DIF_SAMPLE_UNDEFINED,
    DIF_SAMPLE_TIME,
    DIF_SAMPLE_DEPTH,
    DIF_SAMPLE_PRESSURE,
    DIF_SAMPLE_TEMPERATURE,
    DIF_SAMPLE_EVENT,
    DIF_SAMPLE_RBT,
    DIF_SAMPLE_HEARTBEAT,
    DIF_SAMPLE_BEARING,
    DIF_SAMPLE_VENDOR
} dif_sample_type_t;

typedef enum dif_sample_event_t {
    DIF_SAMPLE_EVENT_NONE,
    DIF_SAMPLE_EVENT_DECOSTOP,
    DIF_SAMPLE_EVENT_RBT,
    DIF_SAMPLE_EVENT_ASCENT,
    DIF_SAMPLE_EVENT_CEILING,
    DIF_SAMPLE_EVENT_WORKLOAD,
    DIF_SAMPLE_EVENT_TRANSMITTER,
    DIF_SAMPLE_EVENT_VIOLATION,
    DIF_SAMPLE_EVENT_BOOKMARK,
    DIF_SAMPLE_EVENT_SURFACE,
    DIF_SAMPLE_EVENT_SAFETYSTOP,
    DIF_SAMPLE_EVENT_GASCHANGE,
    DIF_SAMPLE_EVENT_SAFETYSTOP_VOLUNTARY,
    DIF_SAMPLE_EVENT_SAFETYSTOP_MANDATORY,
    DIF_SAMPLE_EVENT_DEEPSTOP,
    DIF_SAMPLE_EVENT_CEILING_SAFETYSTOP,
    DIF_SAMPLE_EVENT_UNKNOWN,
    DIF_SAMPLE_EVENT_DIVETIME,
    DIF_SAMPLE_EVENT_MAXDEPTH,
    DIF_SAMPLE_EVENT_OLF,
    DIF_SAMPLE_EVENT_PO2,
    DIF_SAMPLE_EVENT_AIRTIME,
    DIF_SAMPLE_EVENT_RGBM,
    DIF_SAMPLE_EVENT_HEADING,
    DIF_SAMPLE_EVENT_TISSUELEVEL
} dif_sample_event_t;

/*
 * The actual values needed for each subsample vary
 */
typedef union dif_sample_value_t {
    double depth;
    struct {
        unsigned int tank;
        double value;
    } pressure;
    double temperature;
    struct {
        dif_sample_event_t type;
        unsigned int time;
        unsigned int flags;
        unsigned int value;
    } event;
    unsigned int rbt;
    unsigned int heartbeat;
    unsigned int bearing;
    struct {
        unsigned int type;
        unsigned int size;
        const void *data;
    } vendor;
} dif_sample_value_t;


/*
 * a sample is really a bit of a misnomer as it can be a set of events or
 * samples that all occurred at the same timestamp of the dive
 */
typedef struct dif_sample_t {
    unsigned int timestamp;
    GList *subsamples; /* a list of dif_subsample_t */
} dif_sample_t;

/*
 * An individual element of a sample.
 */
typedef struct dif_subsample_t {
    dif_sample_type_t type;
    dif_sample_value_t value;
} dif_subsample_t;


dif_dive_collection_t *dif_dive_collection_alloc();
void dif_dive_collection_free(dif_dive_collection_t *dc);
dif_dive_collection_t *dif_dive_collection_add_dive(dif_dive_collection_t *dc, dif_dive_t *dive);
dif_dive_t *dif_dive_alloc();
void dif_dive_free(dif_dive_t *dive);
dif_dive_t *dif_dive_add_sample(dif_dive_t *dive, dif_sample_t *sample);
dif_dive_t *dif_dive_add_gasmix(dif_dive_t *dive, dif_gasmix_t *gasmix);
dif_dive_t *dif_dive_set_datetime(dif_dive_t *dive, guint year, guint month, guint day, guint hour, guint minute, guint second);
dif_dive_t *dif_dive_set_duration(dif_dive_t *dive, guint duration);
dif_dive_t *dif_dive_set_maxdepth(dif_dive_t *dive, gdouble maxdepth);
dif_gasmix_t *dif_gasmix_alloc();
void dif_gasmix_free(dif_gasmix_t *gasmix);
dif_sample_t *dif_sample_alloc();
void dif_sample_free(dif_sample_t *sample);
dif_sample_t *dif_sample_add_subsample(dif_sample_t *sample, dif_subsample_t *subsample);
dif_subsample_t *dif_subsample_alloc();
void dif_subsample_free(dif_subsample_t *subsample);
void dif_save_dive_collection_uddf(dif_dive_collection_t *dc, gchar* filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DC2UDDF_DIF_H */
