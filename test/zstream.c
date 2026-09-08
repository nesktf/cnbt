#include "tests.h"

static MunitResult test_dummy(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);
  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/zlib/dummy", test_dummy, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

TestData zstream_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
