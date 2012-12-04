#include <glib.h>
#include "dif.h"

dif_dive_collection_t *dif_dive_collection_alloc() {
    dif_dive_collection_t *dc;
    dc = g_malloc(sizeof(dif_dive_collection_t));
    dc->dives = NULL;
    return dc;
}

void dif_dive_collection_free(dif_dive_collection_t *dc) {
    g_list_free_full(dc->dives, (GDestroyNotify) dif_dive_free);
    g_free(dc);
}

dif_dive_collection_t *dif_dive_collection_add_dive(dif_dive_collection_t *dc, dif_dive_t *dive) {
    dc->dives = g_list_append(dc->dives, dive);
    return dc;
}

dif_dive_t *dif_dive_alloc() {
    dif_dive_t *dive;
    dive = g_malloc(sizeof(dif_dive_t));
    dive->samples = NULL;
    dive->gasmixes = NULL;
    dive->datetime = NULL;
    dive->surfaceInterval = -1;
    dive = dif_dive_set_datetime(dive, 2000,01,01,12,00,00);
    return dive;
}

void dif_dive_free(dif_dive_t *dive) {
    g_list_free_full(dive->samples, (GDestroyNotify) dif_sample_free);
    g_list_free_full(dive->gasmixes, (GDestroyNotify) dif_gasmix_free);
    if (dive->datetime != NULL) {
        g_date_time_unref(dive->datetime);
    }
    g_free(dive);
}

dif_dive_t *dif_dive_add_sample(dif_dive_t *dive, dif_sample_t *sample) {
    dive->samples = g_list_append(dive->samples, sample);
    return dive;
}

dif_dive_t *dif_dive_add_gasmix(dif_dive_t *dive, dif_gasmix_t *gasmix) {
    dive->gasmixes = g_list_append(dive->gasmixes, gasmix);
    return dive;
}

dif_dive_t *dif_dive_set_datetime(dif_dive_t *dive, guint year, guint month, guint day, guint hour, guint minute, guint second) {
    dive->datetime = g_date_time_new_local(year, month, day, hour, minute, second);
    return dive;
}

dif_dive_t *dif_dive_set_datetime_utc(dif_dive_t *dive, guint year, guint month, guint day, guint hour, guint minute, guint second) {
    dive->datetime = g_date_time_new_utc(year, month, day, hour, minute, second);
    return dive;
}

dif_dive_t *dif_dive_set_duration(dif_dive_t *dive, guint duration) {
    dive->duration = duration;
    return dive;
}

dif_dive_t *dif_dive_set_maxdepth(dif_dive_t *dive, gdouble maxdepth) {
    dive->maxdepth = maxdepth;
    return dive;
}

dif_sample_t *dif_sample_alloc() {
    dif_sample_t *sample;
    sample = g_malloc(sizeof(dif_sample_t));
    sample->subsamples = NULL;
    return sample;
}

void dif_sample_free(dif_sample_t *sample) {
    g_list_free_full(sample->subsamples, (GDestroyNotify) dif_subsample_free);
    g_free(sample);
}

dif_sample_t *dif_sample_add_subsample(dif_sample_t *sample, dif_subsample_t *subsample) {
    sample->subsamples = g_list_append(sample->subsamples, subsample);
    return sample;
}

dif_subsample_t *dif_subsample_alloc() {
    dif_subsample_t *subsample;
    subsample = g_malloc(sizeof(dif_subsample_t));
    subsample->type = DIF_SAMPLE_UNDEFINED;
    return subsample;
}

void dif_subsample_free(dif_subsample_t *subsample) {
    g_free(subsample);
}

/**
 * create a gasmix
 *
 * by default all gasmixes are air
 */
dif_gasmix_t *dif_gasmix_alloc() {
    dif_gasmix_t *gasmix;
    gasmix = g_malloc(sizeof(dif_gasmix_t));
    gasmix->oxygen = 21.0;
    gasmix->helium = 0.0;
    gasmix->nitrogen = 79.0;
    gasmix->argon = 0.0;
    gasmix->hydrogen = 0.0;
    gasmix->type = DIF_GASMIX_UNDEFINED;
    return gasmix;
}

void dif_gasmix_free(dif_gasmix_t *gasmix) {
    g_free(gasmix);
}

/**
 * checks to see if a given gasmix is the composition specified
 *
 * this is generally used as a helper function for dif_gasmix_type, which is
 * why it is not a public method
 */
gboolean _dif_is_gasmix(dif_gasmix_t *gasmix, gdouble o2, gdouble n2, gdouble he, gdouble ar, gdouble h2) {
    if (ABS(gasmix->oxygen - o2) < GAS_EPSILON &&
        ABS(gasmix->nitrogen - n2) < GAS_EPSILON &&
        ABS(gasmix->helium - he) < GAS_EPSILON &&
        ABS(gasmix->argon - ar) < GAS_EPSILON &&
        ABS(gasmix->hydrogen - h2) < GAS_EPSILON) {
        return TRUE;
    }
    return FALSE;
}

/**
 * given a gasmix, return the type of the gasmix
 */
dif_gasmix_type_t dif_gasmix_type(dif_gasmix_t *gasmix) {
    if (_dif_is_gasmix(gasmix, 21.0, 79.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_AIR;
    } else if (_dif_is_gasmix(gasmix, 30.0, 70.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX30;
    } else if (_dif_is_gasmix(gasmix, 31.0, 69.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX31;
    } else if (_dif_is_gasmix(gasmix, 32.0, 68.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX32;
    } else if (_dif_is_gasmix(gasmix, 33.0, 67.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX33;
    } else if (_dif_is_gasmix(gasmix, 34.0, 66.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX34;
    } else if (_dif_is_gasmix(gasmix, 35.0, 65.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX35;
    } else if (_dif_is_gasmix(gasmix, 36.0, 64.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX36;
    } else if (_dif_is_gasmix(gasmix, 37.0, 63.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX37;
    } else if (_dif_is_gasmix(gasmix, 38.0, 62.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX38;
    } else if (_dif_is_gasmix(gasmix, 39.0, 61.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX39;
    } else if (_dif_is_gasmix(gasmix, 40.0, 60.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_EANX40;
    } else if (_dif_is_gasmix(gasmix, 100.0, 0.0, 0.0, 0.0, 0.0)) {
        return DIF_GASMIX_OXYGEN100;
    }
    return DIF_GASMIX_UNKNOWN;
}

gchar *dif_gasmix_name(dif_gasmix_t *gasmix) {
    switch(dif_gasmix_type(gasmix)) {
    case DIF_GASMIX_AIR:
        return "air";
        break;
    case DIF_GASMIX_EANX30:
        return "eanx30";
        break;
    case DIF_GASMIX_EANX31:
        return "eanx31";
        break;
    case DIF_GASMIX_EANX32:
        return "eanx32";
        break;
    case DIF_GASMIX_EANX33:
        return "eanx33";
        break;
    case DIF_GASMIX_EANX34:
        return "eanx34";
        break;
    case DIF_GASMIX_EANX35:
        return "eanx35";
        break;
    case DIF_GASMIX_EANX36:
        return "eanx36";
        break;
    case DIF_GASMIX_EANX37:
        return "eanx37";
        break;
    case DIF_GASMIX_EANX38:
        return "eanx38";
        break;
    case DIF_GASMIX_EANX39:
        return "eanx39";
        break;
    case DIF_GASMIX_EANX40:
        return "eanx40";
        break;
    case DIF_GASMIX_OXYGEN100:
        return "pureoxygen";
        break;
    case DIF_GASMIX_UNKNOWN:
    default:
    {
        gchar *gasmixName = g_malloc(26);
        g_snprintf(gasmixName, 26, "mix_%02do2_%02dn2%02dhe%02dar%02dh2",
                (gint)gasmix->oxygen, (gint)gasmix->nitrogen,
                (gint)gasmix->helium, (gint)gasmix->argon,
                (gint)gasmix->hydrogen);
        return gasmixName;
        break;
    }
    }
    return NULL;
}

/**
 * the Uwatec Galileo Luna usually reports extra tanks as 100% nitrogen
 * this isn't a valid tank and no one would ever dive with a 100% n2 tank,
 * so we ignore those tanks.
 *
 * for right now we do allow tanks with gas mixes that don't add up to one
 * and tanks that are unbreatheable with other combinations (e.g. 100% Ar
 * might be valid for drysuits).
 */
gboolean dif_gasmix_is_valid(dif_gasmix_t *gasmix) {
    if (ABS(gasmix->nitrogen - 100) < GAS_EPSILON) {
        return FALSE;
    }
    return TRUE;
}

/**
 * given a sample, get the subsample that matches the particular type
 */
dif_subsample_t *dif_sample_get_subsample(dif_sample_t *sample, dif_sample_type_t sampleType) {
    if (sample == NULL) {
        return NULL;
    }
    GList *subsamples = g_list_first(sample->subsamples);
    while (subsamples != NULL) {
        dif_subsample_t *subsample = subsamples->data;
        if (subsample->type == sampleType) {
            return subsample;
        }
        subsamples = g_list_next(subsamples);
    }
    return NULL;
}

gint _dif_dive_compare(gconstpointer a, gconstpointer b) {
    dif_dive_t *dive1 = (dif_dive_t *)a;
    dif_dive_t *dive2 = (dif_dive_t *)b;
    GDateTime *time1 = dive1->datetime;
    GDateTime *time2 = dive2->datetime;
    if (time1 != NULL && time2 != NULL) {
        return g_date_time_compare(time1, time2);
    } else if (time1 != NULL && time2 == NULL) {
        return -1;
    } else if (time1 == NULL && time2 != NULL) {
        return 1;
    } else {
        return 0;
    }
    return 0;
}

/**
 * sorts dives in ascending order by their timestamp
 *
 * dives with no timestamp are put to the end of the list
 */
dif_dive_collection_t *dif_dive_collection_sort_dives(dif_dive_collection_t *dc) {
    GList *dives = dc->dives;
    dc->dives = g_list_sort(dives, _dif_dive_compare);
    return dc;
}

gint _dif_dive_sample_compare(gconstpointer a, gconstpointer b) {
    dif_sample_t *s1 = (dif_sample_t *)a;
    dif_sample_t *s2 = (dif_sample_t *)b;

    if (s1->timestamp < s2->timestamp) {
        return -1;
    }
    if (s1->timestamp > s2->timestamp) {
        return 1;
    }
    return 0;
}

/**
 * sorts samples in ascending order by their timestamp
 */
dif_dive_t *dif_dive_sort_samples(dif_dive_t *dive) {
    GList *samples = dive->samples;
    dive->samples = g_list_sort(samples, _dif_dive_sample_compare);
    return dive;
}

/**
 * calculate and assign the surface interval between dives
 */
dif_dive_collection_t *dif_dive_collection_calculate_surface_interval(dif_dive_collection_t *dc) {
    dc = dif_dive_collection_sort_dives(dc);
    GList *dives = dc->dives;
    dif_dive_t *previousDive = NULL;
    while (dives != NULL) {
        dif_dive_t *thisDive = dives->data;
        if (thisDive->duration == 0 && thisDive->samples) {
            thisDive = dif_dive_sort_samples(thisDive);
            dif_sample_t *firstSample = g_list_first(thisDive->samples)->data;
            dif_sample_t *lastSample = g_list_last(thisDive->samples)->data;
            thisDive->duration = lastSample->timestamp - firstSample->timestamp;
        }
        if (previousDive == NULL) {
            thisDive->surfaceInterval = -1;
        } else {
            GDateTime *previousTime = previousDive->datetime;
            previousTime = g_date_time_add_seconds(previousTime, previousDive->duration);
            GTimeSpan diff = g_date_time_difference(thisDive->datetime, previousTime);
            thisDive->surfaceInterval = diff/G_TIME_SPAN_SECOND;
            if (thisDive->surfaceInterval > SURFACE_INTERVAL_MAX) {
                thisDive->surfaceInterval = -1;
            }
        }
        previousDive = thisDive;
        dives = g_list_next(dives);
    }
    return dc;
}

/**
 * given a dive, get the first valid pressure
 *
 * @param dive: the dif_dive_t object
 * @param tank: the id of the tank to scan for. Using -1 gets the first valid tank
 * @return: the first valid pressure, or 0.0 if not found
 */
gdouble dif_dive_get_initial_pressure(dif_dive_t *dive, gint tank) {
    GList *samples = g_list_first(dive->samples);
    gdouble initialPressure = 0.0;
    while (samples != NULL && initialPressure < GAS_EPSILON) {
        dif_sample_t *sample = samples->data;
        dif_subsample_t *subsample = dif_sample_get_subsample(sample, DIF_SAMPLE_PRESSURE);
        if (subsample != NULL) {
            if ((tank < 0 || subsample->value.pressure.tank == tank) && subsample->value.pressure.value > GAS_EPSILON) {
                initialPressure = subsample->value.pressure.value;
            }
        }
        samples = g_list_next(samples);
    }
    return initialPressure;
}
