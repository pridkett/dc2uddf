#include <glib.h>
#include "dif.h"

/* fudge factor for doubles */
#define EPSILON 0.001
/* minimum pressure */
#define PRESSURE_MIN 1.0
/* minimum depth */
#define DEPTH_MIN 1.0

/*
 * search through the dive to see the last point where depth is 0.0
 * then check to see if there is any point where depth is > 0 after that
 * point
 */
dif_dive_collection_t *dif_alg_dc_initial_pressure_fix(dif_dive_collection_t *dc) {
    GList *diveList = g_list_first(dc->dives);
    while (diveList != NULL) {
        dif_dive_t *dive = diveList->data;
        dif_alg_dive_initial_pressure_fix(dive);
        diveList = g_list_next(diveList);
    }
    return dc;
}

dif_dive_t *dif_alg_dive_initial_pressure_fix(dif_dive_t *dive) {
    gdouble initialPressure = 0.0;
    guint initialTank = 1;

    GList *tmp = g_list_first(dive->samples);
    GList *needPressure = NULL;
    while (tmp != NULL && initialPressure < PRESSURE_MIN) {
        dif_sample_t *sample = tmp->data;
        dif_subsample_t *ss = dif_sample_get_subsample(sample, DIF_SAMPLE_PRESSURE);
        if (ss != NULL) {
            gdouble pressure = ss->value.pressure.value;
            if (pressure > PRESSURE_MIN) {
                initialPressure = pressure;
                initialTank = ss->value.pressure.tank;
            }
        }

        /* account for double comparison problems */
        if (initialPressure < PRESSURE_MIN) {
            needPressure = g_list_append(needPressure, tmp->data);
        }
        tmp = g_list_next(tmp);
    }
    if (initialPressure > PRESSURE_MIN) {
        while (needPressure != NULL) {
            dif_sample_t *sample = needPressure->data;
            dif_subsample_t *ss = dif_sample_get_subsample(sample, DIF_SAMPLE_PRESSURE);
            if (ss != NULL) {
                ss->value.pressure.value = initialPressure;
            } else {
                ss = dif_subsample_alloc();
                ss->type = DIF_SAMPLE_PRESSURE;
                ss->value.pressure.tank = initialTank;
                ss->value.pressure.value = initialPressure;
                dif_sample_add_subsample(sample, ss);
            }
            needPressure = g_list_next(needPressure);
        }
    }
    /* just free the list, not the elements */
    g_list_free(needPressure);
    return dive;
}

dif_dive_collection_t *dif_alg_dc_truncate_dives(dif_dive_collection_t *dc) {
    GList *diveList = g_list_first(dc->dives);
    while (diveList != NULL) {
        dif_dive_t *dive = diveList->data;
        dif_alg_dive_truncate_dive(dive);
        diveList = g_list_next(diveList);
    }
    return dc;
}

/**
 * scans through a dive looking at the depth samples. Truncates all points
 * after the last time the surface is 0 and there is never another time the
 * depth goes below 0.0
 */
dif_dive_t *dif_alg_dive_truncate_dive(dif_dive_t *dive) {
    GList *samples = g_list_first(dive->samples);
    GList *currentLast = NULL;
    while (samples != NULL) {
        dif_sample_t *sample = samples->data;
        dif_subsample_t *ss = dif_sample_get_subsample(sample, DIF_SAMPLE_DEPTH);

        if (ss != NULL) {
            if (ss->value.depth >= DEPTH_MIN) {
                currentLast = NULL;
            } else if (currentLast == NULL && ss->value.depth - EPSILON < 0) {
                currentLast = samples;
            }
        }
        samples = g_list_next(samples);
    }

    if (currentLast != NULL) {
        currentLast = g_list_next(currentLast);
        while (currentLast != NULL) {
            /* keep that last sample */
            GList *thisLink = currentLast;
            currentLast = g_list_next(currentLast);
            dif_sample_free(thisLink->data);
            thisLink->data = NULL;
            g_list_delete_link(currentLast, thisLink);
        }
    }
    return dive;
}
