#include "tests.h"

static MunitTest tests[] = {
  {"/int-init", test_int_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {"/list-init", test_list_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {"/compound-init", test_comp_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {"/write-pretty", test_write_pretty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

static const MunitSuite suite = {
  .prefix = "/cnbt-tests",
  .tests = tests,
  .suites = NULL,
  .iterations = 1,
  .options = MUNIT_SUITE_OPTION_NONE,
};

int main(int argc, char* argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}
