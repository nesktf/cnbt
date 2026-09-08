#ifndef CNBT_TEST_H_
#define CNBT_TEST_H_

#include "cnbt_internal.h"
#include <munit.h>

typedef struct BuffCtx {
  char data[1024];
  long pos;
  cnbt_EndianMode endian_mode;
} BuffCtx;

extern cnbt_IoFunc BUFFER_CBS;

typedef struct TestData {
  MunitTest* tests;
  size_t len;
} TestData;

extern TestData tag_tests;
extern TestData io_tests;
extern TestData zstream_tests;
extern TestData formatter_tests;
extern TestData filestream_tests;

#endif // CNBT_TEST_H_
