#include "tests.h"

#include <zlib.h>
#include <stdio.h>

static const char test_data[] = "My funny test string with numbers, here look: 123456789";
static const uint8_t test_data_compr[]  = { // why is it bigger lol
  0x78, 0x9c, 0xf3, 0xad, 0x54, 0x48, 0x2b, 0xcd, 0xcb, 0xab, 0x54, 0x28, 0x49, 0x2d, 0x2e, 0x51, 
  0x28, 0x2e, 0x29, 0xca, 0xcc, 0x4b, 0x57, 0x28, 0xcf, 0x2c, 0xc9, 0x50, 0xc8, 0x2b, 0xcd, 0x4d, 
  0x4a, 0x2d, 0x2a, 0xd6, 0x51, 0xc8, 0x48, 0x2d, 0x4a, 0x55, 0xc8, 0xc9, 0xcf, 0xcf, 0xb6, 0x52, 
  0x30, 0x34, 0x32, 0x36, 0x31, 0x35, 0x33, 0xb7, 0xb0, 0x64, 0x00, 0x00, 0x46, 0x89, 0x12, 0xa2
};

static MunitResult test_zread(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status ret;
  size_t r;
  cnbt_ZStream zstr;
  char buff[1024];
  BuffCtx ctx = {0};
  memcpy(ctx.data, test_data_compr, sizeof(test_data_compr));

  ret = cnbt_zopen_read(&zstr, CNBT_ZDEFAULT_WINDOW_BITS, &ctx, &BUFFER_CBS);
  munit_assert_true(ret == CNBT_OK);
  
  memset(buff, 0x00, sizeof(buff));
  r = cnbt_zread(buff, 1, sizeof(test_data), zstr);
  munit_assert_int64(r, ==, sizeof(test_data));
  munit_assert_memory_equal(sizeof(test_data), buff, test_data);

  // Seek backwards
  munit_assert_int32(cnbt_zseek(zstr, 3, CNBT_SEEK_SET), ==, 0);
  munit_assert_int64(cnbt_ztell(zstr), ==, 3);
  memset(buff, 0x00, sizeof(buff));
  r = cnbt_zread(buff, 1, 5, zstr);
  munit_assert_int64(r, ==, 5);
  munit_assert_memory_equal(5, buff, "funny");

  // Seek forward
  munit_assert_int32(cnbt_zseek(zstr, 38, CNBT_SEEK_CUR), ==, 0);
  munit_assert_int64(cnbt_ztell(zstr), ==, 46);
  memset(buff, 0x00, sizeof(buff));
  r = cnbt_zread(buff, 1, 10, zstr);
  munit_assert_int64(r, ==, 10);
  munit_assert_memory_equal(10, buff, "123456789");

  // Seek end should error
  munit_assert_int32(cnbt_zseek(zstr, 9, CNBT_SEEK_END), ==, -1);

  cnbt_zclose(zstr);
  munit_assert_true(ret == CNBT_OK);

  return MUNIT_OK;
}

typedef struct WriteCtx {
  BuffCtx buff;
  int flush_thing;
} WriteCtx;

static void* test_zwrite_flush_setup(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);
  WriteCtx* ctx = malloc(sizeof(*ctx));
  memset(ctx, 0x00, sizeof(*ctx));
  ctx->flush_thing = 1;
  return ctx;
}

static void* test_zwrite_setup(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);
  WriteCtx* ctx = malloc(sizeof(*ctx));
  memset(ctx, 0x00, sizeof(*ctx));
  return ctx;
}

static void test_zwrite_teardown(void* data) {
  free(data);
}

static MunitResult test_zwrite(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  WriteCtx* ctx = data;
  cnbt_Status ret;
  size_t w;
  cnbt_ZStream zstr;

  ret = cnbt_zopen_write(&zstr, NULL, &ctx->buff, &BUFFER_CBS);
  munit_assert_true(ret == CNBT_OK);

  w = cnbt_zwrite(test_data, sizeof(*test_data), sizeof(test_data), zstr);
  munit_assert_int64(w, ==, sizeof(test_data));
  munit_assert_int64(cnbt_ztell(zstr), ==, sizeof(test_data));
  munit_assert_int32(cnbt_zseek(zstr, 0, CNBT_SEEK_SET), ==, -1); // should fail when writting

  if (ctx->flush_thing) {
    ret = cnbt_zflush(zstr);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_memory_equal(sizeof(test_data_compr), ctx->buff.data, test_data_compr);
  }

  ret = cnbt_zclose(zstr);
  munit_assert_true(ret == CNBT_OK);
  if (!ctx->flush_thing) {
    munit_assert_memory_equal(sizeof(test_data_compr), ctx->buff.data, test_data_compr);
  }
  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/zlib/zread", test_zread, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {"/zlib/zwrite", test_zwrite, test_zwrite_setup, test_zwrite_teardown,
  MUNIT_TEST_OPTION_NONE, NULL},
  {"/zlib/zwrite-close-flush", test_zwrite, test_zwrite_flush_setup, test_zwrite_teardown,
  MUNIT_TEST_OPTION_NONE, NULL},
};

TestData zstream_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
