#include <glib.h>
#include "dif.h"

dif_dive_collection_t *dif_dive_collection_alloc() {
    dif_dive_collection_t *dc;
    dc = g_malloc(sizeof(dif_dive_collection_t));
    dc->dives = NULL;
    return dc;
}

void dif_dive_collection_free(dif_dive_collection_t *dc) {
    // FIXME: this IS NOT THE WAY TO DO THIS!
    // FIXME: this leaks memory right now
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
    return dive;
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
    dive->year = year;
    dive->month = month;
    dive->day = day;
    dive->hour = hour;
    dive->minute = minute;
    dive->second = second;
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

dif_gasmix_t *dif_gasmix_alloc() {
    dif_gasmix_t *gasmix;
    gasmix = g_malloc(sizeof(dif_gasmix_t));
    gasmix->oxygen = 0.0;
    gasmix->helium = 0.0;
    gasmix->nitrogen = 0.0;
    gasmix->argon = 0.0;
    gasmix->hydrogen = 0.0;
    return gasmix;
}
