#include <stdlib.h>
#include <check.h>
#include "dif/dif.h"

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
