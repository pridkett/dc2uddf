#ifndef DC2UDDF_DIF_H
#define DC2UDDF_DIF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <glib.h>

#define GAS_EPSILON 0.1
#define SURFACE_INTERVAL_MAX 86400

/**
 * @brief A dive collection is used for a set of dives downloaded from a dive computer
 * 
 * This structure represents a collection of dives that can be exported to UDDF format.
 * It contains a list of individual dive records.
 */
typedef struct dif_dive_collection_t {
    GList *dives; /**< A list of dif_dive_t structures representing individual dives */
} dif_dive_collection_t;

/**
 * @brief A single dive consists of a sequence of samples
 * 
 * This structure represents one dive with all its associated data including
 * timestamp, duration, maximum depth, gas mixes used, and samples recorded.
 */
typedef struct dif_dive_t {
    GDateTime *datetime;      /**< Date and time of the dive */
    guint duration;           /**< Duration of the dive in seconds */
    gdouble maxdepth;         /**< Maximum depth reached during the dive in meters */
    GList *gasmixes;          /**< A list of dif_gasmix_t structures representing gas mixes used */
    GList *samples;           /**< A list of dif_sample_t structures representing dive samples */
    gint surfaceInterval;     /**< Number of seconds since the last dive, -1 if first dive */
} dif_dive_t;

/**
 * @brief Types of gas mixes used in diving
 * 
 * Enumeration of common gas mixes used in diving, including air, various 
 * nitrox mixes, and pure oxygen.
 */
typedef enum dif_gasmix_type_t {
    DIF_GASMIX_UNDEFINED,    /**< Undefined gas mix */
    DIF_GASMIX_AIR,           /**< Standard air (21% O2, 79% N2) */
    DIF_GASMIX_EANX30,        /**< EANx 30 (30% O2, 70% N2) */
    DIF_GASMIX_EANX31,        /**< EANx 31 (31% O2, 69% N2) */
    DIF_GASMIX_EANX32,        /**< EANx 32 (32% O2, 68% N2) */
    DIF_GASMIX_EANX33,        /**< EANx 33 (33% O2, 67% N2) */
    DIF_GASMIX_EANX34,        /**< EANx 34 (34% O2, 66% N2) */
    DIF_GASMIX_EANX35,        /**< EANx 35 (35% O2, 65% N2) */
    DIF_GASMIX_EANX36,        /**< EANx 36 (36% O2, 64% N2) */
    DIF_GASMIX_EANX37,        /**< EANx 37 (37% O2, 63% N2) */
    DIF_GASMIX_EANX38,        /**< EANx 38 (38% O2, 62% N2) */
    DIF_GASMIX_EANX39,        /**< EANx 39 (39% O2, 61% N2) */
    DIF_GASMIX_EANX40,        /**< EANx 40 (40% O2, 60% N2) */
    DIF_GASMIX_OXYGEN100,     /**< Pure oxygen (100% O2) */
    DIF_GASMIX_UNKNOWN,      /**< Unknown gas mix composition */
} dif_gasmix_type_t;

/**
 * @brief Gas mix information for a dive
 * 
 * Structure containing the composition of a gas mix used during diving,
 * including percentages of different gases and a type classification.
 */
typedef struct dif_gasmix_t {
    guint id;              /**< Identifier for the gas mix */
    gdouble helium;        /**< Percentage of helium in the mix */
    gdouble oxygen;        /**< Percentage of oxygen in the mix */
    gdouble nitrogen;      /**< Percentage of nitrogen in the mix */
    gdouble argon;         /**< Percentage of argon in the mix */
    gdouble hydrogen;      /**< Percentage of hydrogen in the mix */
    dif_gasmix_type_t type;/**< Type classification of the gas mix */
} dif_gasmix_t;

/**
 * @brief Types of samples that can be recorded during a dive
 * 
 * Enumeration of different types of data samples that can be captured
 * during a dive, such as depth, pressure, temperature, events, etc.
 */
typedef enum dif_sample_type_t {
    DIF_SAMPLE_UNDEFINED,    /**< Undefined sample type */
    DIF_SAMPLE_TIME,         /**< Time sample */
    DIF_SAMPLE_DEPTH,        /**< Depth sample in meters */
    DIF_SAMPLE_PRESSURE,     /**< Tank pressure sample in bar */
    DIF_SAMPLE_TEMPERATURE,  /**< Temperature sample in Celsius */
    DIF_SAMPLE_EVENT,        /**< Event sample */
    DIF_SAMPLE_RBT,          /**< Remaining bottom time sample */
    DIF_SAMPLE_HEARTBEAT,    /**< Heart rate sample */
    DIF_SAMPLE_BEARING,      /**< Compass bearing sample */
    DIF_SAMPLE_VENDOR        /**< Vendor specific sample */
} dif_sample_type_t;

/**
 * @brief Types of events that can occur during a dive
 * 
 * Enumeration of different events that can be recorded by a dive computer
 * during a dive, such as deco stops, gas changes, safety stops, etc.
 */
typedef enum dif_sample_event_t {
    DIF_SAMPLE_EVENT_NONE,                 /**< No event */
    DIF_SAMPLE_EVENT_DECOSTOP,             /**< Decompression stop */
    DIF_SAMPLE_EVENT_RBT,                  /**< Remaining bottom time warning */
    DIF_SAMPLE_EVENT_ASCENT,               /**< Ascent rate warning */
    DIF_SAMPLE_EVENT_CEILING,              /**< Ceiling violation */
    DIF_SAMPLE_EVENT_WORKLOAD,             /**< Workload warning */
    DIF_SAMPLE_EVENT_TRANSMITTER,          /**< Transmitter error */
    DIF_SAMPLE_EVENT_VIOLATION,            /**< Dive rule violation */
    DIF_SAMPLE_EVENT_BOOKMARK,             /**< User bookmark */
    DIF_SAMPLE_EVENT_SURFACE,              /**< Surface detected */
    DIF_SAMPLE_EVENT_SAFETYSTOP,            /**< Safety stop */
    DIF_SAMPLE_EVENT_GASCHANGE,            /**< Gas change */
    DIF_SAMPLE_EVENT_SAFETYSTOP_VOLUNTARY,  /**< Voluntary safety stop */
    DIF_SAMPLE_EVENT_SAFETYSTOP_MANDATORY, /**< Mandatory safety stop */
    DIF_SAMPLE_EVENT_DEEPSTOP,             /**< Deep stop */
    DIF_SAMPLE_EVENT_CEILING_SAFETYSTOP,   /**< Ceiling reached during safety stop */
    DIF_SAMPLE_EVENT_UNKNOWN,              /**< Unknown event type */
    DIF_SAMPLE_EVENT_DIVETIME,             /**< Dive time exceeded */
    DIF_SAMPLE_EVENT_MAXDEPTH,             /**< Maximum depth exceeded */
    DIF_SAMPLE_EVENT_OLF,                  /**< Oxygen liability fraction warning */
    DIF_SAMPLE_EVENT_PO2,                  /**< Partial oxygen pressure warning */
    DIF_SAMPLE_EVENT_AIRTIME,              /**< Air time warning */
    DIF_SAMPLE_EVENT_RGBM,                 /**< RGBM model warning */
    DIF_SAMPLE_EVENT_HEADING,              /**< Heading change */
    DIF_SAMPLE_EVENT_TISSUELEVEL,          /**< Tissue level warning */
    DIF_SAMPLE_EVENT_GASCHANGE2,           /**< Gas change (secondary) */
    DIF_SAMPLE_EVENT_NDL                   /**< No decompression limit warning */
} dif_sample_event_t;

/**
 * @brief Union for sample values
 * 
 * The actual values needed for each subsample vary based on the sample type.
 * This union allows storing different types of sample data in a memory-efficient way.
 */
typedef union dif_sample_value_t {
    gdouble depth;               /**< Depth in meters */
    struct {
        guint tank;              /**< Tank identifier */
        gdouble value;           /**< Pressure value in bar */
    } pressure;
    gdouble temperature;         /**< Temperature in Celsius */
    struct {
        dif_sample_event_t type; /**< Event type */
        guint time;             /**< Event time */
        guint flags;            /**< Event flags */
        guint value;            /**< Event value */
    } event;
    guint rbt;                   /**< Remaining bottom time in seconds */
    guint heartbeat;             /**< Heart rate in beats per minute */
    guint bearing;               /**< Compass bearing in degrees */
    struct {
        guint type;             /**< Vendor-specific type */
        guint size;             /**< Size of vendor data */
        const void *data;        /**< Pointer to vendor-specific data */
    } vendor;
} dif_sample_value_t;

/**
 * @brief A sample containing multiple subsamples at the same timestamp
 * 
 * A sample is really a bit of a misnomer as it can be a set of events or
 * samples that all occurred at the same timestamp of the dive.
 */
typedef struct dif_sample_t {
    guint timestamp;           /**< Timestamp in seconds since start of dive */
    GList *subsamples;        /**< A list of dif_subsample_t structures */
} dif_sample_t;

/**
 * @brief An individual element of a sample
 * 
 * Represents a single measurement or event at a specific timestamp.
 */
typedef struct dif_subsample_t {
    dif_sample_type_t type;    /**< Type of sample data */
    dif_sample_value_t value;  /**< Value of the sample data */
} dif_subsample_t;

/**
 * @brief Configuration settings for XML serializer
 * 
 * Structure containing options for UDDF XML serialization, such as 
 * output filename and whether to include non-standard elements.
 */
typedef struct xml_options_t {
    gchar *filename;           /**< Output filename for UDDF XML */
    gboolean useInvalidElements; /**< Whether to include non-standard XML elements for debugging */
} xml_options_t;

/* dif.c */
dif_dive_collection_t *dif_dive_collection_alloc();
void dif_dive_collection_free(dif_dive_collection_t *dc);
dif_dive_collection_t *dif_dive_collection_add_dive(dif_dive_collection_t *dc, dif_dive_t *dive);
dif_dive_collection_t *dif_dive_collection_sort_dives(dif_dive_collection_t *dc);
dif_dive_collection_t *dif_dive_collection_calculate_surface_interval(dif_dive_collection_t *dc);
dif_dive_t *dif_dive_alloc();
void dif_dive_free(dif_dive_t *dive);
dif_dive_t *dif_dive_add_sample(dif_dive_t *dive, dif_sample_t *sample);
dif_dive_t *dif_dive_add_gasmix(dif_dive_t *dive, dif_gasmix_t *gasmix);
dif_dive_t *dif_dive_set_datetime(dif_dive_t *dive, guint year, guint month, guint day, guint hour, guint minute, guint second);
dif_dive_t *dif_dive_set_datetime_utc(dif_dive_t *dive, guint year, guint month, guint day, guint hour, guint minute, guint second);
dif_dive_t *dif_dive_set_duration(dif_dive_t *dive, guint duration);
dif_dive_t *dif_dive_set_maxdepth(dif_dive_t *dive, gdouble maxdepth);
dif_dive_t *dif_dive_sort_samples(dif_dive_t *dive);
gdouble dif_dive_get_initial_pressure(dif_dive_t *dive, gint tank);
dif_gasmix_t *dif_gasmix_alloc();
void dif_gasmix_free(dif_gasmix_t *gasmix);
dif_gasmix_type_t dif_gasmix_type(dif_gasmix_t *gasmix);
gchar *dif_gasmix_name(dif_gasmix_t *gasmix);
gboolean dif_gasmix_is_valid(dif_gasmix_t *gasmix);
dif_sample_t *dif_sample_alloc();
void dif_sample_free(dif_sample_t *sample);
dif_sample_t *dif_sample_add_subsample(dif_sample_t *sample, dif_subsample_t *subsample);
dif_subsample_t *dif_sample_get_subsample(dif_sample_t *sample, dif_sample_type_t sampleType);
dif_subsample_t *dif_subsample_alloc();
void dif_subsample_free(dif_subsample_t *subsample);

/* uddf.c */
xml_options_t *dif_xml_options_alloc();
void dif_xml_options_free(xml_options_t *options);
void dif_save_dive_collection_uddf(dif_dive_collection_t *dc, gchar* filename);
void dif_save_dive_collection_uddf_options(dif_dive_collection_t *dc, xml_options_t *options);

/* algos.c */
dif_dive_collection_t *dif_alg_dc_initial_pressure_fix(dif_dive_collection_t *dc);
dif_dive_t *dif_alg_dive_initial_pressure_fix(dif_dive_t *dive);
dif_dive_collection_t *dif_alg_dc_truncate_dives(dif_dive_collection_t *dc);
dif_dive_t *dif_alg_dive_truncate_dive(dif_dive_t *dive);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DC2UDDF_DIF_H */
