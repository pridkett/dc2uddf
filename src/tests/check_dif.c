#include <stdio.h>
#include <stdlib.h>
#include <check.h>
#include "dif/dif.h"

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
        sample = dif_sample_add_subsample(sample, sspressure);
        sample = dif_sample_add_subsample(sample, ssdepth);
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
    dive2 = dif_dive_set_datetime_utc(dive2, 2012, 02, 01, 12, 00, 00);
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
    suite_add_tcase(s, tc_methods);

    TCase *tc_uddf = tcase_create("UDDF");
    tcase_add_test(tc_uddf, test_dif_save_simple_dive_collection_uddf);
    tcase_add_test(tc_uddf, test_dif_save_dive_collection_uddf);
    suite_add_tcase(s, tc_uddf);

    TCase *tc_algos = tcase_create("Algorithms");
    tcase_add_test(tc_algos, test_dif_alg_dc_initial_pressure_fix);
    tcase_add_test(tc_algos, test_dif_alg_dc_truncate_dives);
    suite_add_tcase(s, tc_algos);
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
