#include "tests.h"

#include <stdio.h>
#include <unistd.h>

static cnbt_IoFunc FILE_CBS = {
  .read = (PFN_cnbt_read_func)fread,
  .write = (PFN_cnbt_write_func)fwrite,
  .seek = (PFN_cnbt_seek_func)fseek,
  .tell = (PFN_cnbt_tell_func)ftell,
  .eof = (PFN_cnbt_eof_func)feof,
};

static cnbt_IoFunc ZSTREAM_CBS = {
  .read = (PFN_cnbt_read_func)cnbt_zread,
  .write = (PFN_cnbt_write_func)cnbt_zwrite,
  .seek = (PFN_cnbt_seek_func)cnbt_zseek,
  .tell = (PFN_cnbt_tell_func)cnbt_ztell,
  .eof = (PFN_cnbt_eof_func)NULL,
};

static cnbt_Status read_compound(const char* path, cnbt_Compound* comp, cnbt_EndianMode mode) {
  FILE* f = fopen(path, "rb");
  munit_assert_not_null(f); // check your paths if this fails
  cnbt_Status ret = cnbt_read(comp, mode, f, &FILE_CBS);
  fclose(f);
  return ret;
}

static cnbt_Status write_compound(const char* path, const cnbt_Compound* comp,
                                  cnbt_EndianMode mode) {
  FILE* f = fopen(path, "wb");
  munit_assert_not_null(f); // check your paths if this fails
  cnbt_Status ret = cnbt_write(comp, mode, f, &FILE_CBS);
  fclose(f);
  return ret;
}

static cnbt_Status read_compressed_compound(const char* path, cnbt_Compound* comp,
                                            cnbt_EndianMode mode) {
  FILE* f = fopen(path, "rb");
  munit_assert_not_null(f); // check your paths if this fails
  cnbt_ZStream zstr = NULL;
  cnbt_Status ret = cnbt_zopen_read(&zstr, CNBT_ZAUTO_WINDOW_BITS, f, &FILE_CBS);
  munit_assert_int32(ret, ==, CNBT_OK);
  ret = cnbt_read(comp, mode, zstr, &ZSTREAM_CBS);
  cnbt_zclose(zstr);
  fclose(f);
  return ret;
}

static cnbt_Status write_compressed_compound(const char* path, const cnbt_Compound* comp,
                                             cnbt_EndianMode mode) {
  FILE* f = fopen(path, "wb");
  munit_assert_not_null(f); // check your paths if this fails
  cnbt_ZStream zstr = NULL;
  cnbt_ZWriteArgs args = {
    .level = CNBT_ZDEFAULT_LEVEL,
    .window_bits = CNBT_ZGZIP_WINDOW_BITS,
    .mem_level = CNBT_ZDEFAULT_MEM_LEVEL,
    .strategy = CNBT_ZDEFAULT_STRATEGY,
  };
  cnbt_Status ret = cnbt_zopen_write(&zstr, &args, f, &FILE_CBS);
  munit_assert_int32(ret, ==, CNBT_OK);
  ret = cnbt_write(comp, mode, zstr, &ZSTREAM_CBS);
  cnbt_zclose(zstr);
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
  munit_assert_float(cnbt_get_float(&kt->value), ==, 0.49823147f);

  kt = cnbt_comp_get(comp, "doubleTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "doubleTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_DOUBLE);
  munit_assert_double(cnbt_get_double(&kt->value), ==, 0.4931287132182315);

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
    cnbt_Tag* ltag;
    for (size_t i = 0; i < 5; ++i) {
      ltag = cnbt_list_get(&kt->value, i);
      munit_assert_not_null(ltag);
      munit_assert_true(cnbt_get_type(ltag) == CNBT_TYPE_LONG);
      munit_assert_int64(cnbt_get_long(ltag), ==, val + i);
    }
  }

  kt = cnbt_comp_get(comp, "listTest (end)");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "listTest (end)");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_LIST);
  munit_assert_int64(cnbt_list_len(&kt->value), ==, 0);

  kt = cnbt_comp_get(comp, "nested compound test");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "nested compound test");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_COMPOUND);
  {
    cnbt_Compound* ncomp = &kt->value;
    cnbt_KeyTag* subkt = cnbt_comp_get(ncomp, "egg");
    munit_assert_not_null(subkt);
    munit_assert_true(cnbt_get_type(&subkt->value) == CNBT_TYPE_COMPOUND);
    cnbt_KeyTag* leaf = cnbt_comp_get(&subkt->value, "name");
    munit_assert_not_null(leaf);
    munit_assert_string_equal(cnbt_str_data(&leaf->value), "Eggbert");
    leaf = cnbt_comp_get(&subkt->value, "value");
    munit_assert_not_null(leaf);
    munit_assert_float(cnbt_get_float(&leaf->value), ==, 0.5f);

    subkt = cnbt_comp_get(ncomp, "ham");
    munit_assert_not_null(subkt);
    munit_assert_true(cnbt_get_type(&subkt->value) == CNBT_TYPE_COMPOUND);
    leaf = cnbt_comp_get(&subkt->value, "name");
    munit_assert_not_null(leaf);
    munit_assert_string_equal(cnbt_str_data(&leaf->value), "Hampus");
    leaf = cnbt_comp_get(&subkt->value, "value");
    munit_assert_not_null(leaf);
    munit_assert_float(cnbt_get_float(&leaf->value), ==, 0.75f);
  }

  kt = cnbt_comp_get(comp, "intArrayTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "intArrayTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_INT_ARRAY);
  munit_assert_int64(cnbt_int_array_len(&kt->value), ==, 4);
  {
    int32_t* idata = cnbt_int_array_data(&kt->value);
    munit_assert_not_null(idata);
    static const int32_t expected_ints[] = {0X00010203, 0X04050607, 0X08090A0B, 0X0C0D0E0F};
    munit_assert_memory_equal(sizeof(expected_ints), idata, expected_ints);
  }

  kt = cnbt_comp_get(comp, "longArrayTest");
  munit_assert_not_null(kt);
  munit_assert_string_equal(kt->key, "longArrayTest");
  munit_assert_true(cnbt_get_type(&kt->value) == CNBT_TYPE_LONG_ARRAY);
  munit_assert_int64(cnbt_long_array_len(&kt->value), ==, 2);
  {
    int64_t* ldata = cnbt_long_array_data(&kt->value);
    munit_assert_not_null(ldata);
    static const int64_t expected_longs[] = {(int64_t)0X0DECAFC0FFEEBABEULL,
                                             (int64_t)0XDEADBEEFBAADF00DULL};
    munit_assert_memory_equal(sizeof(expected_longs), ldata, expected_longs);
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

  // test roundtrip write uncompressed
  static const char tmp_uncompr[] = TESTFILES "/test_uncompr_roundtrip.nbt";
  ret = write_compound(tmp_uncompr, &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  cnbt_Compound comp_rt = {0};
  ret = read_compound(tmp_uncompr, &comp_rt, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  check_test_compound(&comp_rt);
  cnbt_free(&comp_rt);
  unlink(tmp_uncompr);

  cnbt_free(&comp);

  // littletest
  ret = read_compound(TESTFILES "/littletest_uncompr", &comp, CNBT_LITTLE_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);
  check_test_compound(&comp);
  cnbt_free(&comp);

  return MUNIT_OK;
}

static MunitResult test_read_bigtest_gzip(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status ret;
  cnbt_Compound comp = {0};

  // bigtest
  ret = read_compressed_compound(TESTFILES "/bigtest.nbt", &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);
  check_test_compound(&comp);

  // roundtrip write compressed
  static const char tmp_gzip[] = TESTFILES "/test_bigtest_roundtrip.nbt";
  ret = write_compressed_compound(tmp_gzip, &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);

  cnbt_Compound comp_rt = {0};
  ret = read_compressed_compound(tmp_gzip, &comp_rt, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  check_test_compound(&comp_rt);

  cnbt_free(&comp_rt);
  cnbt_free(&comp);
  unlink(tmp_gzip);

  return MUNIT_OK;
}

static MunitResult test_read_b173_level_gzip(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status ret;
  cnbt_Compound comp = {0};

  ret = read_compressed_compound(TESTFILES "/b173_level.dat", &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);

  cnbt_KeyTag* data_kt = cnbt_comp_get(&comp, "Data"); // root compound contains "Data"
  munit_assert_not_null(data_kt);
  munit_assert_true(cnbt_get_type(&data_kt->value) == CNBT_TYPE_COMPOUND);
  cnbt_Compound* data_comp = &data_kt->value;

  cnbt_KeyTag* seed_kt = cnbt_comp_get(data_comp, "RandomSeed");
  munit_assert_not_null(seed_kt);
  munit_assert_true(cnbt_get_type(&seed_kt->value) == CNBT_TYPE_LONG);
  munit_assert_int64(cnbt_get_long(&seed_kt->value), ==, (int64_t)0X1F47DB5057C52CE8ULL);

  cnbt_KeyTag* ver_kt = cnbt_comp_get(data_comp, "version");
  munit_assert_not_null(ver_kt);
  munit_assert_true(cnbt_get_type(&ver_kt->value) == CNBT_TYPE_INT);
  munit_assert_int32(cnbt_get_int(&ver_kt->value), ==, 19132);

  cnbt_KeyTag* lvl_kt = cnbt_comp_get(data_comp, "LevelName");
  munit_assert_not_null(lvl_kt);
  munit_assert_true(cnbt_get_type(&lvl_kt->value) == CNBT_TYPE_STRING);
  munit_assert_string_equal(cnbt_str_data(&lvl_kt->value), "New World");

  cnbt_KeyTag* player_kt = cnbt_comp_get(data_comp, "Player");
  munit_assert_not_null(player_kt);
  munit_assert_true(cnbt_get_type(&player_kt->value) == CNBT_TYPE_COMPOUND);

  // Roundtrip write compressed
  const char tmp_level[] = TESTFILES "/test_b173_roundtrip.dat";
  ret = write_compressed_compound(tmp_level, &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);

  cnbt_Compound comp_rt = {0};
  ret = read_compressed_compound(tmp_level, &comp_rt, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);

  data_kt = cnbt_comp_get(&comp_rt, "Data");
  munit_assert_not_null(data_kt);
  data_comp = &data_kt->value;
  seed_kt = cnbt_comp_get(data_comp, "RandomSeed");
  munit_assert_not_null(seed_kt);
  munit_assert_int64(cnbt_get_long(&seed_kt->value), ==, (int64_t)0x1f47db5057c52ce8ULL);
  lvl_kt = cnbt_comp_get(data_comp, "LevelName");
  munit_assert_not_null(lvl_kt);
  munit_assert_string_equal(cnbt_str_data(&lvl_kt->value), "New World");

  cnbt_free(&comp_rt);
  cnbt_free(&comp);
  unlink(tmp_level);

  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/filestream/read-uncompressed", test_read_uncompressed, NULL, NULL, MUNIT_TEST_OPTION_NONE,
   NULL},
  {"/filestream/read-bigtest-gzip", test_read_bigtest_gzip, NULL, NULL, MUNIT_TEST_OPTION_NONE,
   NULL},
  {"/filestream/read-b173-level-gzip", test_read_b173_level_gzip, NULL, NULL,
   MUNIT_TEST_OPTION_NONE, NULL},
};

TestData filestream_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
