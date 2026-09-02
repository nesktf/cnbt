#ifndef CNBT_TEST_H_
#define CNBT_TEST_H_

#include "cnbt_internal.h"
#include <munit.h>

typedef struct TestData {
  MunitTest* tests;
  size_t len;
} TestData;

extern TestData data_tests;
extern TestData io_tests;

#endif // CNBT_TEST_H_
