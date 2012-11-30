#include <glib.h>
#include "dif.h"

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
    int lastSample = 999999;

    GList *tmp = g_list_first(dive->samples);
    GList *needPressure = NULL;
    while (tmp != NULL && initialPressure < 1.0) {
        dif_sample_t *sample = tmp->data;
        dif_subsample_t *ss = dif_sample_get_subsample(sample, DIF_SAMPLE_PRESSURE);
        if (ss != NULL) {
            gdouble pressure = ss->value.pressure.value;
            if (pressure > 1.0) {
                initialPressure = pressure;
                initialTank = ss->value.pressure.tank;
            }
        }

        /* account for double comparison problems */
        if (initialPressure < 1.0) {
            needPressure = g_list_append(needPressure, tmp->data);
        }
        tmp = g_list_next(tmp);
    }
    if (initialPressure > 1.0) {
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

dif_dive_t *dif_alg_dive_truncate_dive(dif_dive_t *dive) {
    return dive;
}
