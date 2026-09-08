#include "tests.h"

#include <stdio.h>

static cnbt_IoFunc FILE_CBS = {
  .read = (PFN_cnbt_read_func)fread,
  .write = (PFN_cnbt_write_func)fwrite,
  .seek = (PFN_cnbt_seek_func)fseek,
};

static cnbt_Status read_compound(const char* path, cnbt_Compound* comp, cnbt_EndianMode mode) {
  FILE* f = fopen(path, "rb");
  munit_assert_not_null(f); // check your paths if this fails
  cnbt_Status ret = cnbt_read(comp, mode, f, &FILE_CBS);
  fclose(f);
  return ret;
}

static void check_test_compound(cnbt_Compound* comp) {
  cnbt_KeyTag* kt;
  munit_assert_int64(cnbt_comp_len(comp), ==, 14);

  kt = cnbt_comp_get(comp, "byteTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "byteTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_BYTE);
  munit_assert_int8(cnbt_get_byte(&kt->value), ==, 127);

  kt = cnbt_comp_get(comp, "shortTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "shortTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_SHORT);
  munit_assert_int16(cnbt_get_short(&kt->value), ==, 32767);

  kt = cnbt_comp_get(comp, "intTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "intTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_INT);
  munit_assert_int32(cnbt_get_int(&kt->value), ==, 2147483647);

  kt = cnbt_comp_get(comp, "longTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "longTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_LONG);
  munit_assert_int64(cnbt_get_long(&kt->value), ==, 9223372036854775807);

  kt = cnbt_comp_get(comp, "floatTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "floatTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_FLOAT);
  munit_assert_float(cnbt_get_float(&kt->value), ==, 0.4982315f);

  kt = cnbt_comp_get(comp, "doubleTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "doubleTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_DOUBLE);
  munit_assert_double(cnbt_get_double(&kt->value), ==, 0.493128713218231);

  static const char array_name[] = "byteArrayTest (the first 1000 values of (n*n*255+n*7)%100, "
                                   "starting with n=0 (0, 62, 34, 16, 8, ...))";
  int8_t array_data[1000];
  for (size_t n = 0; n < ARRSZ(array_data); ++n) {
    array_data[n] = (n * n * 255 + n * 7) % 100;
  }
  kt = cnbt_comp_get(comp, array_name);
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, array_name);
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_BYTE_ARRAY);
  munit_assert_int64(cnbt_byte_array_len(&kt->value), ==, sizeof(array_data));
  int8_t* data = cnbt_byte_array_data(&kt->value);
  munit_assert_not_null(data);
  munit_assert_memory_equal(sizeof(array_data), data, array_data);

  kt = cnbt_comp_get(comp, "stringTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "stringTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_STRING);
  munit_assert_not_null(cnbt_str_data(&kt->value));
  munit_assert_string_equal(cnbt_str_data(&kt->value),
                            "HELLO WORLD THIS IS A TEST STRING \u00C5\u00C4\u00D6!");

  kt = cnbt_comp_get(comp, "listTest (compound)");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "listTest (compound)");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_LIST);
  {
    cnbt_Compound* lcomp;
    cnbt_KeyTag* lkt;

    lcomp = cnbt_list_get(&kt->value, 0);
    munit_assert_not_null(lcomp);
    munit_assert_true(cnbt_get_type(lcomp) == CNBT_TYPE_COMPOUND);
    lkt = cnbt_comp_get(lcomp, "created-on");
    munit_assert_not_null(lkt);
    munit_assert_string_equal(lkt->key, "created-on");
    munit_assert_true(cnbt_get_type(&lkt->value) == CNBT_TYPE_LONG);
    munit_assert_int64(cnbt_get_long(&lkt->value), ==, 1264099775885);
    lkt = cnbt_comp_get(lcomp, "name");
    munit_assert_not_null(lkt);
    munit_assert_string_equal(lkt->key, "name");
    munit_assert_true(cnbt_get_type(&lkt->value) == CNBT_TYPE_STRING);
    munit_assert_int64(cnbt_str_len(&lkt->value), ==, 15);
    munit_assert_string_equal(cnbt_str_data(&lkt->value), "Compound tag #0");

    lcomp = cnbt_list_get(&kt->value, 1);
    munit_assert_not_null(lcomp);
    munit_assert_true(cnbt_get_type(lcomp) == CNBT_TYPE_COMPOUND);
    lkt = cnbt_comp_get(lcomp, "created-on");
    munit_assert_not_null(lkt);
    munit_assert_string_equal(lkt->key, "created-on");
    munit_assert_true(cnbt_get_type(&lkt->value) == CNBT_TYPE_LONG);
    munit_assert_int64(cnbt_get_long(&lkt->value), ==, 1264099775885);
    lkt = cnbt_comp_get(lcomp, "name");
    munit_assert_not_null(lkt);
    munit_assert_string_equal(lkt->key, "name");
    munit_assert_true(cnbt_get_type(&lkt->value) == CNBT_TYPE_STRING);
    munit_assert_int64(cnbt_str_len(&lkt->value), ==, 15);
    munit_assert_string_equal(cnbt_str_data(&lkt->value), "Compound tag #1");
  }

  kt = cnbt_comp_get(comp, "listTest (long)");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "listTest (long)");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_LIST);
  munit_assert_int64(cnbt_list_len(&kt->value), ==, 5);
  {
    const i64 val = 11;
    cnbt_Tag* ltag; // = {11, 12, 13, 14, 15}
    for (size_t i = 0; i < 5; ++i) {
      ltag = cnbt_list_get(&kt->value, i);
      munit_assert_not_null(ltag);
      munit_assert_true(cnbt_get_type(ltag) == CNBT_TYPE_LONG);
      munit_assert_int64(cnbt_get_long(ltag), ==, val + i);
    }
  }
}

static MunitResult test_read_uncompressed(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status ret;
  cnbt_Compound comp = {0};

  // bigtest
  ret = read_compound(TESTFILES "/bigtest_uncompr", &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);
  check_test_compound(&comp);
  cnbt_free(&comp);

  // littletest
  ret = read_compound(TESTFILES "/littletest_uncompr", &comp, CNBT_LITTLE_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);
  check_test_compound(&comp);
  cnbt_free(&comp);

  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/filestream/read-uncompressed", test_read_uncompressed, NULL, NULL,
   MUNIT_TEST_OPTION_NONE, NULL},
};

TestData filestream_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
