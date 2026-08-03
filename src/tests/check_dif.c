#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <check.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "dif/dif.h"
#include "dumpfile.h"

/**
 * helper function to create a couple of dives for testing of algorithms
 *
 * dive1: six samples
 *        samples 0,1 lack pressure and should be fixed by dif_alg_dive_fix_initial_pressure
 *
 * dive2: twelve samples
 *        samples 0,1 lack pressure and should be fixed by dif_alg_dive_fix_initial_pressure
 *        samples 8,9,10,11 are bobbing at the surface and should be fixed dif_alg_dive_truncate_dive
 */
dif_dive_collection_t *_create_simple_dive_collection() {
    dif_dive_collection_t *dc = dif_dive_collection_alloc();

    gdouble dive1_pressures[] =  {0.0, 0.0, 180.0, 179.0, 178.5, 177.5};
    gdouble dive1_depths[] =     {0.0, 1.0, 2.0,   2.0,   1.0,   0.0};
    gdouble dive1_temps[] =      {21.5, 21.0, 20.5, 20.0,  20.5,  21.0};
    guint   dive1_timestamps[] = {0,   30,  60,    90,    120,   150};
    guint dive1_num_samples = sizeof(dive1_pressures)/sizeof(dive1_pressures[0]);
    dif_dive_t *dive1 = dif_dive_alloc();
    dive1 = dif_dive_set_datetime_utc(dive1, 2012, 02, 01, 12, 00, 00);
    dc = dif_dive_collection_add_dive(dc, dive1);
    guint ctr = 0;
    for (ctr = 0; ctr < dive1_num_samples; ctr++) {
        dif_sample_t *sample = dif_sample_alloc();
        sample->timestamp = dive1_timestamps[ctr];
        dif_subsample_t *sspressure = dif_subsample_alloc();
        sspressure->type = DIF_SAMPLE_PRESSURE;
        sspressure->value.pressure.tank = 1;
        sspressure->value.pressure.value = dive1_pressures[ctr];

        dif_subsample_t *ssdepth = dif_subsample_alloc();
        ssdepth->type = DIF_SAMPLE_DEPTH;
        ssdepth->value.depth = dive1_depths[ctr];

        dif_subsample_t *sstemp = dif_subsample_alloc();
        sstemp->type = DIF_SAMPLE_TEMPERATURE;
        sstemp->value.temperature = dive1_temps[ctr];
        sample = dif_sample_add_subsample(sample, sspressure);
        sample = dif_sample_add_subsample(sample, ssdepth);
        sample = dif_sample_add_subsample(sample, sstemp);
        dive1 = dif_dive_add_sample(dive1, sample);
    }
    dif_gasmix_t *gasmix1 = dif_gasmix_alloc();
    gasmix1->oxygen = 32.0;
    gasmix1->nitrogen = 68.0;
    dif_dive_add_gasmix(dive1, gasmix1);

    gdouble dive2_pressures[] =  {0.0, 0.0, 180.0, 179.0, 178.5, 177.5, 176.5, 177.5, 177.5, 177.5, 177.5, 177.5};
    gdouble dive2_depths[] =     {0.0, 1.0, 2.0,   2.0,   1.0,   0.0,   2.0,   0.0,   0.2,   0.1,   0.0,   0.1};
    guint   dive2_timestamps[] = {0,   30,  60,    90,    120,   150,   180,   210,   240,   270,   300,   330};
    guint dive2_num_samples = sizeof(dive2_pressures)/sizeof(dive2_pressures[0]);

    dif_dive_t *dive2 = dif_dive_alloc();
    dive2 = dif_dive_set_datetime_utc(dive2, 2012, 02, 01, 14, 00, 00);
    dc = dif_dive_collection_add_dive(dc, dive2);
    for (ctr = 0; ctr < dive2_num_samples; ctr++) {
        dif_sample_t *sample = dif_sample_alloc();
        dif_subsample_t *sspressure = dif_subsample_alloc();
        sample->timestamp = dive2_timestamps[ctr];
        sspressure->type = DIF_SAMPLE_PRESSURE;
        sspressure->value.pressure.tank = 1;
        sspressure->value.pressure.value = dive2_pressures[ctr];

        dif_subsample_t *ssdepth = dif_subsample_alloc();
        ssdepth->type = DIF_SAMPLE_DEPTH;
        ssdepth->value.depth = dive2_depths[ctr];
        sample = dif_sample_add_subsample(sample, sspressure);
        sample = dif_sample_add_subsample(sample, ssdepth);
        dive2 = dif_dive_add_sample(dive2, sample);
    }
    dif_gasmix_t *gasmix2 = dif_gasmix_alloc();
    dif_dive_add_gasmix(dive2, gasmix2);

    gdouble dive3_pressures[] =  {0.0, 0.0, 180.0, 179.0, 178.5, 177.5, 176.5, 177.5, 177.5, 177.5, 177.5, 177.5};
    gdouble dive3_depths[] =     {0.0, 1.0, 2.0,   2.0,   1.0,   0.0,   2.0,   0.0,   0.2,   0.1,   0.0,   0.1};
    guint   dive3_timestamps[] = {0,   30,  60,    90,    120,   150,   180,   210,   240,   270,   300,   330};
    guint dive3_num_samples = sizeof(dive3_pressures)/sizeof(dive3_pressures[0]);

    dif_dive_t *dive3 = dif_dive_alloc();
    dive3 = dif_dive_set_datetime_utc(dive3, 2012, 02, 02, 12, 00, 00);
    dc = dif_dive_collection_add_dive(dc, dive3);
    for (ctr = 0; ctr < dive3_num_samples; ctr++) {
        dif_sample_t *sample = dif_sample_alloc();
        dif_subsample_t *sspressure = dif_subsample_alloc();
        sample->timestamp = dive3_timestamps[ctr];
        sspressure->type = DIF_SAMPLE_PRESSURE;
        sspressure->value.pressure.tank = 1;
        sspressure->value.pressure.value = dive3_pressures[ctr];

        dif_subsample_t *ssdepth = dif_subsample_alloc();
        ssdepth->type = DIF_SAMPLE_DEPTH;
        ssdepth->value.depth = dive3_depths[ctr];
        sample = dif_sample_add_subsample(sample, sspressure);
        sample = dif_sample_add_subsample(sample, ssdepth);
        dive3 = dif_dive_add_sample(dive3, sample);
    }
    dif_gasmix_t *gasmix3 = dif_gasmix_alloc();
    dif_dive_add_gasmix(dive3, gasmix3);

    return dc;
}

START_TEST (test_dif_dive_collection_alloc)
{
    dif_dive_collection_t *dc = NULL;
    dc = dif_dive_collection_alloc();
    fail_unless(dc != NULL,
                "new dif_dive_collection is still null");
    dif_dive_collection_free(dc);
}
END_TEST

START_TEST (test_dif_dive_alloc)
{
    dif_dive_t *dive = NULL;
    dive = dif_dive_alloc();
    fail_unless(dive != NULL,
                "new dif_dive is still null");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_sample_alloc)
{
    dif_sample_t *sample = NULL;
    sample = dif_sample_alloc();
    fail_unless(sample != NULL,
                "new dif_sample is still null");
    dif_sample_free(sample);
}
END_TEST

START_TEST (test_dif_gasmix_alloc)
{
    dif_gasmix_t *gasmix = NULL;
    gasmix = dif_gasmix_alloc();
    fail_unless(gasmix != NULL,
                "new dif_gasmix is still null");
    dif_gasmix_free(gasmix);
}
END_TEST

START_TEST (test_dif_subsample_alloc)
{
    dif_subsample_t *subsample = NULL;
    subsample = dif_subsample_alloc();
    fail_unless(subsample != NULL,
                "new dif_subsample is still null");
    dif_subsample_free(subsample);
}
END_TEST

START_TEST (test_dif_dive_collection_add_dive)
{
    dif_dive_collection_t *dc = NULL;
    dif_dive_t *dive = NULL;
    dc = dif_dive_collection_alloc();
    dive = dif_dive_alloc();
    dc = dif_dive_collection_add_dive(dc, dive);
    fail_unless(g_list_length(dc->dives) == 1,
                "dive not properly added");
    dif_dive_collection_free(dc);
}
END_TEST

START_TEST (test_dif_dive_add_sample)
{
    dif_dive_t *dive = NULL;
    dif_sample_t *sample = NULL;

    dive = dif_dive_alloc();
    sample = dif_sample_alloc();

    dive = dif_dive_add_sample(dive, sample);
    fail_unless(g_list_length(dive->samples) == 1,
                "sample not properly added");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_add_gasmix)
{
    dif_dive_t *dive = NULL;
    dif_gasmix_t *gasmix = NULL;

    dive = dif_dive_alloc();
    gasmix = dif_gasmix_alloc();

    dive = dif_dive_add_gasmix(dive, gasmix);
    fail_unless(g_list_length(dive->gasmixes) == 1,
                "gasmix not properly added");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_sample_add_subsample)
{
    dif_sample_t *sample = NULL;
    dif_subsample_t *subsample = NULL;

    sample = dif_sample_alloc();
    subsample = dif_subsample_alloc();

    sample = dif_sample_add_subsample(sample, subsample);
    fail_unless(g_list_length(sample->subsamples) == 1,
                "subsample not properly added");
    dif_sample_free(sample);
}
END_TEST

START_TEST (test_dif_dive_set_avgdepth)
{
    dif_dive_t *dive = dif_dive_alloc();
    fail_unless(!dive->hasAvgdepth,
                "hasAvgdepth should be FALSE after alloc");
    dive = dif_dive_set_avgdepth(dive, 12.3);
    fail_unless(dive->hasAvgdepth,
                "hasAvgdepth should be TRUE after set");
    fail_unless(fabs(dive->avgdepth - 12.3) < 0.001,
                "avgdepth not stored properly");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_set_tank_pressures)
{
    dif_dive_t *dive = dif_dive_alloc();
    fail_unless(!dive->hasTankPressures,
                "hasTankPressures should be FALSE after alloc");
    dive = dif_dive_set_tank_pressures(dive, 200.0, 50.0);
    fail_unless(dive->hasTankPressures,
                "hasTankPressures should be TRUE after set");
    fail_unless(fabs(dive->beginPressure - 200.0) < 0.001,
                "beginPressure not stored properly");
    fail_unless(fabs(dive->endPressure - 50.0) < 0.001,
                "endPressure not stored properly");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_set_min_temperature)
{
    dif_dive_t *dive = dif_dive_alloc();
    fail_unless(!dive->hasMinTemperature,
                "hasMinTemperature should be FALSE after alloc");
    dive = dif_dive_set_min_temperature(dive, 18.5);
    fail_unless(dive->hasMinTemperature,
                "hasMinTemperature should be TRUE after set");
    fail_unless(fabs(dive->minTemperature - 18.5) < 0.001,
                "minTemperature not stored properly");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_get_final_pressure)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_dive_t *dive = g_list_first(dc->dives)->data;
    fail_unless(fabs(dif_dive_get_final_pressure(dive, -1) - 177.5) < 0.001,
                "final pressure for any tank should be 177.5");
    fail_unless(fabs(dif_dive_get_final_pressure(dive, 1) - 177.5) < 0.001,
                "final pressure for tank 1 should be 177.5");
    fail_unless(fabs(dif_dive_get_final_pressure(dive, 2)) < 0.001,
                "final pressure for unknown tank should be 0.0");
    dif_dive_collection_free(dc);
}
END_TEST

START_TEST (test_dif_dive_get_final_pressure_no_pressure)
{
    dif_dive_t *dive = dif_dive_alloc();
    dif_sample_t *sample = dif_sample_alloc();
    sample->timestamp = 0;
    dif_subsample_t *ssdepth = dif_subsample_alloc();
    ssdepth->type = DIF_SAMPLE_DEPTH;
    ssdepth->value.depth = 5.0;
    sample = dif_sample_add_subsample(sample, ssdepth);
    dive = dif_dive_add_sample(dive, sample);
    fail_unless(fabs(dif_dive_get_final_pressure(dive, -1)) < 0.001,
                "final pressure without pressure samples should be 0.0");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_get_initial_pressure_tank)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_dive_t *dive = g_list_first(dc->dives)->data;
    fail_unless(dif_dive_get_initial_pressure_tank(dive) == 1,
                "initial pressure tank of dive1 should be 1");
    dif_dive_collection_free(dc);

    dif_dive_t *emptyDive = dif_dive_alloc();
    fail_unless(dif_dive_get_initial_pressure_tank(emptyDive) == -1,
                "initial pressure tank without samples should be -1");
    dif_dive_free(emptyDive);
}
END_TEST

/**
 * a dive with two transmitters: tank 1 reports early samples, tank 2
 * reports later samples. begin and end pressures must both come from
 * tank 1 (the tank of the first valid reading), never mix tanks.
 */
START_TEST (test_dif_dive_pressures_multi_tank)
{
    guint   timestamps[] = {0,   30,  60,  90};
    gdouble pressures[]  = {200.0, 150.0, 180.0, 100.0};
    guint   tanks[]      = {1,   1,   2,   2};
    guint num_samples = sizeof(pressures)/sizeof(pressures[0]);

    dif_dive_t *dive = dif_dive_alloc();
    guint ctr;
    for (ctr = 0; ctr < num_samples; ctr++) {
        dif_sample_t *sample = dif_sample_alloc();
        sample->timestamp = timestamps[ctr];
        dif_subsample_t *sspressure = dif_subsample_alloc();
        sspressure->type = DIF_SAMPLE_PRESSURE;
        sspressure->value.pressure.tank = tanks[ctr];
        sspressure->value.pressure.value = pressures[ctr];
        sample = dif_sample_add_subsample(sample, sspressure);
        dive = dif_dive_add_sample(dive, sample);
    }

    gint tank = dif_dive_get_initial_pressure_tank(dive);
    fail_unless(tank == 1, "first valid pressure should come from tank 1");
    fail_unless(fabs(dif_dive_get_initial_pressure(dive, tank) - 200.0) < 0.001,
                "initial pressure for tank 1 should be 200.0");
    fail_unless(fabs(dif_dive_get_final_pressure(dive, tank) - 150.0) < 0.001,
                "final pressure for tank 1 should be 150.0, not tank 2's 100.0");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_get_average_depth)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_dive_t *dive = g_list_first(dc->dives)->data;
    /* trapezoid over depths {0,1,2,2,1,0} at 30s spacing = 180m*s / 150s */
    fail_unless(fabs(dif_dive_get_average_depth(dive) - 1.2) < 0.001,
                "average depth of dive1 should be 1.2");
    dif_dive_collection_free(dc);
}
END_TEST

START_TEST (test_dif_dive_get_average_depth_edge)
{
    dif_dive_t *dive = dif_dive_alloc();
    fail_unless(fabs(dif_dive_get_average_depth(dive)) < 0.001,
                "average depth without samples should be 0.0");
    dif_sample_t *sample = dif_sample_alloc();
    sample->timestamp = 42;
    dif_subsample_t *ssdepth = dif_subsample_alloc();
    ssdepth->type = DIF_SAMPLE_DEPTH;
    ssdepth->value.depth = 7.5;
    sample = dif_sample_add_subsample(sample, ssdepth);
    dive = dif_dive_add_sample(dive, sample);
    fail_unless(fabs(dif_dive_get_average_depth(dive) - 7.5) < 0.001,
                "average depth with a single sample should be that depth");
    dif_dive_free(dive);
}
END_TEST

START_TEST (test_dif_dive_get_greatest_depth)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_dive_t *dive = g_list_first(dc->dives)->data;
    fail_unless(fabs(dif_dive_get_greatest_depth(dive) - 2.0) < 0.001,
                "greatest depth of dive1 should be 2.0");
    dif_dive_collection_free(dc);

    dif_dive_t *emptyDive = dif_dive_alloc();
    fail_unless(fabs(dif_dive_get_greatest_depth(emptyDive)) < 0.001,
                "greatest depth without samples should be 0.0");
    dif_dive_free(emptyDive);
}
END_TEST

START_TEST (test_dif_dive_get_lowest_temperature)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_dive_t *dive1 = g_list_first(dc->dives)->data;
    fail_unless(fabs(dif_dive_get_lowest_temperature(dive1) - 20.0) < 0.001,
                "lowest temperature of dive1 should be 20.0");
    /* dive2 has no temperature subsamples */
    dif_dive_t *dive2 = g_list_next(g_list_first(dc->dives))->data;
    fail_unless(fabs(dif_dive_get_lowest_temperature(dive2)) < 0.001,
                "lowest temperature without temperature samples should be 0.0");
    dif_dive_collection_free(dc);
}
END_TEST

START_TEST (test_dif_dive_get_dive_duration)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_dive_t *dive = g_list_first(dc->dives)->data;
    fail_unless(dif_dive_get_dive_duration(dive) == 150,
                "dive duration of dive1 should be 150");
    dif_dive_collection_free(dc);

    dif_dive_t *emptyDive = dif_dive_alloc();
    fail_unless(dif_dive_get_dive_duration(emptyDive) == 0,
                "dive duration without samples should be 0");
    dif_dive_free(emptyDive);
}
END_TEST

/**
 * helper for the read-back test: evaluate an xpath expression and return
 * the first result node's content as a double, or NAN if there is no match
 */
static gdouble _xpath_double(xmlXPathContextPtr ctx, const gchar *expr) {
    gdouble result = NAN;
    xmlXPathObjectPtr obj = xmlXPathEvalExpression(BAD_CAST expr, ctx);
    if (obj != NULL && obj->nodesetval != NULL && obj->nodesetval->nodeNr > 0) {
        xmlChar *content = xmlNodeGetContent(obj->nodesetval->nodeTab[0]);
        if (content != NULL) {
            result = g_ascii_strtod((const gchar *)content, NULL);
            xmlFree(content);
        }
    }
    if (obj != NULL) {
        xmlXPathFreeObject(obj);
    }
    return result;
}

START_TEST (test_dif_uddf_informationafterdive_values)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_save_dive_collection_uddf(dc, "test_iad.uddf");
    dif_dive_collection_free(dc);

    xmlDocPtr doc = xmlReadFile("test_iad.uddf", NULL, 0);
    fail_unless(doc != NULL, "could not parse test_iad.uddf");
    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    fail_unless(ctx != NULL, "could not create xpath context");

    /* dive1 is the first dive in the document; values are emitted with
     * one decimal digit, so allow for the rounding */
    gdouble value;
    value = _xpath_double(ctx, "(//*[local-name()='informationafterdive'])[1]/*[local-name()='lowesttemperature']");
    fail_unless(fabs(value - 293.15) < 0.051,
                "lowesttemperature should be ~293.15K (20.0C), got %f", value);
    value = _xpath_double(ctx, "(//*[local-name()='informationafterdive'])[1]/*[local-name()='greatestdepth']");
    fail_unless(fabs(value - 2.0) < 0.051,
                "greatestdepth should be 2.0, got %f", value);
    value = _xpath_double(ctx, "(//*[local-name()='informationafterdive'])[1]/*[local-name()='diveduration']");
    fail_unless(fabs(value - 150.0) < 0.051,
                "diveduration should be 150.0, got %f", value);
    value = _xpath_double(ctx, "(//*[local-name()='informationafterdive'])[1]/*[local-name()='averagedepth']");
    fail_unless(fabs(value - 1.2) < 0.051,
                "averagedepth should be 1.2, got %f", value);
    value = _xpath_double(ctx, "(//*[local-name()='informationafterdive'])[1]/*[local-name()='pressuredrop']");
    fail_unless(fabs(value - 250000.0) < 0.051,
                "pressuredrop should be 250000.0 (2.5 bar), got %f", value);

    /* dive2 has no temperature samples, so the optional element is absent */
    value = _xpath_double(ctx, "(//*[local-name()='informationafterdive'])[2]/*[local-name()='lowesttemperature']");
    fail_unless(isnan(value),
                "dive2 should not emit a lowesttemperature element");

    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);
}
END_TEST

START_TEST (test_dif_save_simple_dive_collection_uddf)
{
    dif_dive_collection_t *dc = dif_dive_collection_alloc();
    dif_dive_t *dive = dif_dive_alloc();
    dc = dif_dive_collection_add_dive(dc, dive);
    dif_save_dive_collection_uddf(dc, "test_simple.uddf");
    dif_dive_collection_free(dc);
    fail_unless(1==1, "shouldn't ever show this error");
}
END_TEST

START_TEST(test_dif_save_dive_collection_uddf)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dif_save_dive_collection_uddf(dc, "test.uddf");
    dif_dive_collection_free(dc);
    fail_unless(1==1, "shouldn't ever show this error");
}
END_TEST

START_TEST (test_dif_alg_dc_initial_pressure_fix)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dc = dif_alg_dc_initial_pressure_fix(dc);
    GList *dives = g_list_first(dc->dives);
    while (dives != NULL) {
        dif_dive_t *dive = dives->data;
        GList *samples = g_list_first(dive->samples);
        while (samples != NULL) {
            dif_sample_t *sample = samples->data;
            dif_subsample_t *pressure = dif_sample_get_subsample(sample, DIF_SAMPLE_PRESSURE);
            if (pressure != NULL) {
                fail_unless(pressure->value.pressure.value > 1.0,
                        "dif_alg_dc_initial_pressure_fix did not reset initial pressures above 1.0bar");
            }
            samples = g_list_next(samples);
        }
        dives = g_list_next(dives);
    };
}
END_TEST

START_TEST (test_dif_alg_dc_truncate_dives)
{
    dif_dive_collection_t *dc = _create_simple_dive_collection();
    dc = dif_alg_dc_truncate_dives(dc);
    GList *dives = g_list_first(dc->dives);
    dif_dive_t *dive = dives->data;
    fail_unless(g_list_length(dive->samples) == 6,
                "dif_alg_dc_truncate_dives truncated too many dives");

    dives = g_list_next(dives);
    dive = dives->data;
    fail_unless(g_list_length(dive->samples) == 8,
                "dif_alg_dc_trunacte_dives didn't properly truncate dives");
}
END_TEST

START_TEST (test_dif_gasmix_type)
{
    dif_gasmix_t *gasmix = dif_gasmix_alloc();

    fail_unless(dif_gasmix_type(gasmix) == DIF_GASMIX_AIR,
                "gasmix failed to test as AIR");

    gasmix->oxygen = 30.0;
    gasmix->nitrogen = 70.0;
    fail_unless(dif_gasmix_type(gasmix) == DIF_GASMIX_EANX30,
                "gasmix failed to test as EANX30");

    gasmix->oxygen = 100.0;
    gasmix->nitrogen = 0.0;
    fail_unless(dif_gasmix_type(gasmix) == DIF_GASMIX_OXYGEN100,
                "gasmix failed to test as OXYGEN100");

    /* right now the library doesn't understand trimix. I don't do
     * trimix and I don't know enough about how to name the different
     * gas mixtures to account for this right now
     */
    gasmix->oxygen  = 15.0;
    gasmix->nitrogen = 40.0;
    gasmix->helium = 45.0;
    fail_unless(dif_gasmix_type(gasmix) == DIF_GASMIX_UNKNOWN,
                "gasmix failed to test as UNKNOWN");
    dif_gasmix_free(gasmix);
}
END_TEST

/**
 * helper to append one Uwatec Smart framed record to a buffer:
 * [A5 A5 5A 5A][uint32 LE length incl. 8-byte header][payload]
 */
static gsize _dump_append_record(guint8 *buf, gsize offset,
                                 const guint8 *payload, guint32 payloadSize) {
    guint32 length = payloadSize + 8;
    buf[offset]   = 0xA5;
    buf[offset+1] = 0xA5;
    buf[offset+2] = 0x5A;
    buf[offset+3] = 0x5A;
    buf[offset+4] = length & 0xFF;
    buf[offset+5] = (length >> 8) & 0xFF;
    buf[offset+6] = (length >> 16) & 0xFF;
    buf[offset+7] = (length >> 24) & 0xFF;
    memcpy(buf + offset + 8, payload, payloadSize);
    return offset + length;
}

typedef struct {
    guint count;
    guint stopAfter;   /* 0 = never stop early */
    gsize sizes[8];
} dump_cb_data_t;

static gboolean _dump_count_cb(const guint8 *record, gsize size, guint index,
                               gpointer userdata) {
    dump_cb_data_t *data = userdata;
    data->sizes[index] = size;
    data->count++;
    /* every record must start with the magic */
    fail_unless(record[0] == 0xA5 && record[2] == 0x5A,
                "callback record does not start with the framing header");
    if (data->stopAfter > 0 && data->count >= data->stopAfter) {
        return FALSE;
    }
    return TRUE;
}

START_TEST (test_dumpfile_split_three_records)
{
    guint8 buf[256];
    guint8 p1[4] = {1, 2, 3, 4};
    guint8 p2[16] = {0};
    guint8 p3[2] = {9, 9};
    gsize len = 0;
    len = _dump_append_record(buf, len, p1, sizeof(p1));
    len = _dump_append_record(buf, len, p2, sizeof(p2));
    len = _dump_append_record(buf, len, p3, sizeof(p3));

    dump_cb_data_t data = {0, 0, {0}};
    GError *err = NULL;
    gint n = dumpfile_foreach_uwatec_smart(buf, len, _dump_count_cb, &data, &err);
    fail_unless(n == 3, "expected 3 records, got %d", n);
    fail_unless(err == NULL, "unexpected error: %s", err ? err->message : "");
    fail_unless(data.sizes[0] == 12 && data.sizes[1] == 24 && data.sizes[2] == 10,
                "record sizes should include the 8-byte header");
}
END_TEST

START_TEST (test_dumpfile_stop_early)
{
    guint8 buf[256];
    guint8 payload[4] = {0};
    gsize len = 0;
    len = _dump_append_record(buf, len, payload, sizeof(payload));
    len = _dump_append_record(buf, len, payload, sizeof(payload));
    len = _dump_append_record(buf, len, payload, sizeof(payload));

    dump_cb_data_t data = {0, 1, {0}};
    GError *err = NULL;
    gint n = dumpfile_foreach_uwatec_smart(buf, len, _dump_count_cb, &data, &err);
    fail_unless(n == 1, "expected iteration to stop after 1 record, got %d", n);
    fail_unless(err == NULL, "stopping early should not be an error");
    fail_unless(data.count == 1, "callback ran %u times after requesting stop",
                data.count);
}
END_TEST

START_TEST (test_dumpfile_bad_magic)
{
    guint8 buf[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    dump_cb_data_t data = {0, 0, {0}};
    GError *err = NULL;
    gint n = dumpfile_foreach_uwatec_smart(buf, sizeof(buf), _dump_count_cb,
                                           &data, &err);
    fail_unless(n == -1, "bad magic should fail");
    fail_unless(err != NULL && strstr(err->message, "memory image") != NULL,
                "error should hint that memory images are not replayable");
    g_clear_error(&err);
}
END_TEST

START_TEST (test_dumpfile_truncated)
{
    guint8 buf[256];
    guint8 payload[4] = {0};
    gsize len = _dump_append_record(buf, 0, payload, sizeof(payload));
    /* second record claims more bytes than remain in the buffer */
    buf[len]   = 0xA5;
    buf[len+1] = 0xA5;
    buf[len+2] = 0x5A;
    buf[len+3] = 0x5A;
    buf[len+4] = 0xFF;
    buf[len+5] = 0x00;
    buf[len+6] = 0x00;
    buf[len+7] = 0x00;
    len += 8;

    dump_cb_data_t data = {0, 0, {0}};
    GError *err = NULL;
    gint n = dumpfile_foreach_uwatec_smart(buf, len, _dump_count_cb, &data, &err);
    fail_unless(n == -1, "truncated record should fail");
    fail_unless(err != NULL && strstr(err->message, "offset") != NULL,
                "error should mention the byte offset");
    g_clear_error(&err);
}
END_TEST

START_TEST (test_dumpfile_empty)
{
    dump_cb_data_t data = {0, 0, {0}};
    GError *err = NULL;
    gint n = dumpfile_foreach_uwatec_smart(NULL, 0, _dump_count_cb, &data, &err);
    fail_unless(n == -1, "empty buffer should fail");
    fail_unless(err != NULL, "empty buffer should set an error");
    g_clear_error(&err);
}
END_TEST

Suite *
dif_suite (void)
{
    Suite *s = suite_create("dif");

    /* core test case */
    TCase *tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_dif_dive_collection_alloc);
    tcase_add_test(tc_core, test_dif_dive_alloc);
    tcase_add_test(tc_core, test_dif_sample_alloc);
    tcase_add_test(tc_core, test_dif_gasmix_alloc);
    tcase_add_test(tc_core, test_dif_subsample_alloc);
    tcase_add_test(tc_core, test_dif_dive_collection_add_dive);
    tcase_add_test(tc_core, test_dif_dive_add_sample);
    tcase_add_test(tc_core, test_dif_dive_add_gasmix);
    tcase_add_test(tc_core, test_dif_sample_add_subsample);
    suite_add_tcase(s, tc_core);

    TCase *tc_methods = tcase_create("Methods");
    tcase_add_test(tc_methods, test_dif_gasmix_type);
    tcase_add_test(tc_methods, test_dif_dive_set_avgdepth);
    tcase_add_test(tc_methods, test_dif_dive_set_tank_pressures);
    tcase_add_test(tc_methods, test_dif_dive_set_min_temperature);
    tcase_add_test(tc_methods, test_dif_dive_get_final_pressure);
    tcase_add_test(tc_methods, test_dif_dive_get_final_pressure_no_pressure);
    tcase_add_test(tc_methods, test_dif_dive_get_initial_pressure_tank);
    tcase_add_test(tc_methods, test_dif_dive_pressures_multi_tank);
    tcase_add_test(tc_methods, test_dif_dive_get_average_depth);
    tcase_add_test(tc_methods, test_dif_dive_get_average_depth_edge);
    tcase_add_test(tc_methods, test_dif_dive_get_greatest_depth);
    tcase_add_test(tc_methods, test_dif_dive_get_lowest_temperature);
    tcase_add_test(tc_methods, test_dif_dive_get_dive_duration);
    suite_add_tcase(s, tc_methods);

    TCase *tc_uddf = tcase_create("UDDF");
    tcase_add_test(tc_uddf, test_dif_save_simple_dive_collection_uddf);
    tcase_add_test(tc_uddf, test_dif_save_dive_collection_uddf);
    tcase_add_test(tc_uddf, test_dif_uddf_informationafterdive_values);
    suite_add_tcase(s, tc_uddf);

    TCase *tc_algos = tcase_create("Algorithms");
    tcase_add_test(tc_algos, test_dif_alg_dc_initial_pressure_fix);
    tcase_add_test(tc_algos, test_dif_alg_dc_truncate_dives);
    suite_add_tcase(s, tc_algos);

    TCase *tc_dumpfile = tcase_create("DumpFile");
    tcase_add_test(tc_dumpfile, test_dumpfile_split_three_records);
    tcase_add_test(tc_dumpfile, test_dumpfile_stop_early);
    tcase_add_test(tc_dumpfile, test_dumpfile_bad_magic);
    tcase_add_test(tc_dumpfile, test_dumpfile_truncated);
    tcase_add_test(tc_dumpfile, test_dumpfile_empty);
    suite_add_tcase(s, tc_dumpfile);
    return s;
}

int
main (int argc, char **argv) {
    int number_failed;
    Suite *s = dif_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
