#include "tests.h"

#include <stdio.h>

typedef struct BuffCtx {
  char data[1024];
  size_t pos;
  cnbt_EndianMode endian_mode;
} BuffCtx;

static size_t read_func(void* buff, size_t sz, size_t nmemb, BuffCtx* ctx) {
  memcpy(buff, ctx->data + ctx->pos, sz * nmemb);
  ctx->pos += sz * nmemb;
  return nmemb;
}

static size_t write_func(const void* buff, size_t sz, size_t nmemb, BuffCtx* ctx) {
  memcpy(ctx->data + ctx->pos, buff, sz * nmemb);
  ctx->pos += sz * nmemb;
  return nmemb;
}

static cnbt_IoFunc BUFFER_CBS = {
  .read = (PFN_cnbt_read_func)read_func,
  .write = (PFN_cnbt_write_func)write_func,
  .seek = NULL,
  .tell = NULL,
};

static void* test_read_endianness_big_setup(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  BuffCtx* ctx = malloc(sizeof(*ctx));
  memset(ctx, 0x00, sizeof(*ctx));

  static const uint8_t input_data[] = {
    0x01, // CNBT_TAG_TYPE_BYTE
    0xFF,

    0x02, // CNBT_TAG_TYPE_SHORT
    0xEE, 0xAA,

    0x03, // CNBT_TAG_TYPE_INT,
    0xCA, 0xFE, 0xBA, 0xBE,

    0x04, // CNBT_TAG_TYPE_LONG
    0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD, 0xF0, 0x0D,

    0x05,                   // CNBT_TAG_TYPE_FLOAT
    0xBA, 0xAA, 0xAA, 0xAA, // I'm a sheep

    0x06, // CNBT_TAG_TYPE_DOUBLE
    0xFE, 0xED, 0xFA, 0xCE, 0xCA, 0xFE, 0xBE, 0xEF,

    0x07, // CNBT_TAG_TYPE_BYTE_ARRAY, try to guess this hash
    0x00, 0x00, 0x00, 0x20, 0x6f, 0xe3, 0x16, 0x2f, 0xff, 0xcc, 0x57, 0x60,
    0xa2, 0xdc, 0xe0, 0x40, 0x8f, 0x50, 0x68, 0x69, 0xd1, 0xd8, 0x31, 0x41,
    0x44, 0x6e, 0x31, 0xe8, 0x74, 0x86, 0x5f, 0x96, 0xb3, 0x7c, 0xac, 0xe5,

    0x08, // CNBT_TAG_TYPE_STRING
    0x00, 0x09, 'f',  'o',  'o',  'b',  'a',  'r',  'b',  'a',  'z',

    0x00, // CNBT_TAG_TYPE_END
  };

  ctx->endian_mode = CNBT_BIG_ENDIAN;
  memcpy(ctx->data, input_data, sizeof(input_data));
  return ctx;
}

static void* test_read_endianness_little_setup(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  BuffCtx* ctx = malloc(sizeof(*ctx));
  memset(ctx, 0x00, sizeof(*ctx));

  static const uint8_t input_data[] = {
    0x01, // CNBT_TAG_TYPE_BYTE
    0xFF,

    0x02, // CNBT_TAG_TYPE_SHORT
    0xAA, 0xEE,

    0x03, // CNBT_TAG_TYPE_INT,
    0xBE, 0xBA, 0xFE, 0xCA,

    0x04, // CNBT_TAG_TYPE_LONG
    0x0D, 0xF0, 0xAD, 0xBA, 0xEF, 0xBE, 0xAD, 0xDE,

    0x05,                   // CNBT_TAG_TYPE_FLOAT
    0xAA, 0xAA, 0xAA, 0xBA, // I'm a sheep

    0x06, // CNBT_TAG_TYPE_DOUBLE
    0xEF, 0xBE, 0xFE, 0xCA, 0xCE, 0xFA, 0xED, 0xFE,

    0x07, // CNBT_TAG_TYPE_BYTE_ARRAY, try to guess this hash
    0x20, 0x00, 0x00, 0x00, 0x6f, 0xe3, 0x16, 0x2f, 0xff, 0xcc, 0x57, 0x60,
    0xa2, 0xdc, 0xe0, 0x40, 0x8f, 0x50, 0x68, 0x69, 0xd1, 0xd8, 0x31, 0x41,
    0x44, 0x6e, 0x31, 0xe8, 0x74, 0x86, 0x5f, 0x96, 0xb3, 0x7c, 0xac, 0xe5,

    0x08, // CNBT_TAG_TYPE_STRING
    0x09, 0x00, 'f',  'o',  'o',  'b',  'a',  'r',  'b',  'a',  'z',

    0x00, // CNBT_TAG_TYPE_END
  };
  ctx->endian_mode = CNBT_LITTLE_ENDIAN;
  memcpy(ctx->data, input_data, sizeof(input_data));
  return ctx;
}

static void test_read_endianness_tear_down(void* data) {
  free(data);
}

static MunitResult test_read_endianness(const MunitParameter* params, void* data) {
  UNUSED(params);

  BuffCtx* ctx = data;
  cnbt__ReadCtx read;
  read.cbs = BUFFER_CBS;
  read.src = ctx;
  read.endian_mode = ctx->endian_mode;

  cnbt_Status ret;
  cnbt_Type type;

  // Byte
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_BYTE);
  {
    i8 num;
    ret = cnbt__read_byte(&read, &num);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_int8(num, ==, (i8)0xFF);
  }

  // Short
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_SHORT);
  {
    i16 num;
    ret = cnbt__read_short(&read, &num);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_int16(num, ==, (i16)0xEEAA);
  }

  // Int
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_INT);
  {
    i32 num;
    ret = cnbt__read_int(&read, &num);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_int32(num, ==, 0xCAFEBABE);
  }

  // Long
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_LONG);
  {
    i64 num;
    ret = cnbt__read_long(&read, &num);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_int64(num, ==, 0xDEADBEEFBAADF00D);
  }

  // Float
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_FLOAT);
  {
    f32 num;
    ret = cnbt__read_float(&read, &num);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_float(num, ==, -0.0013020833f); // sheep in float language
  }

  // Double
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_DOUBLE);
  {
    f64 num;
    ret = cnbt__read_double(&read, &num);
    munit_assert_true(ret == CNBT_OK);

    union {
      f64 vf;
      i64 vi;
    } expected;

    expected.vi = 0xFEEDFACECAFEBEEF;
    munit_assert_double(num, ==, expected.vf);
  }

  // Byte array
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_BYTE_ARRAY);
  {
    // Tip: sha256 for some string
    static uint8_t expected[] = {
      0x6f, 0xe3, 0x16, 0x2f, 0xff, 0xcc, 0x57, 0x60, 0xa2, 0xdc, 0xe0,
      0x40, 0x8f, 0x50, 0x68, 0x69, 0xd1, 0xd8, 0x31, 0x41, 0x44, 0x6e,
      0x31, 0xe8, 0x74, 0x86, 0x5f, 0x96, 0xb3, 0x7c, 0xac, 0xe5,
    };
    cnbt__ByteArrayData arr = {0};
    ret = cnbt__read_blob(&read, &arr);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_uint32(arr.size, ==, 32);
    munit_assert_not_null(arr.data);
    munit_assert_memory_equal(32, arr.data, expected);
    cnbt__free_byte_array(&arr);
  }

  // String
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_STRING);
  {
    cnbt__StringData str = {0};
    ret = cnbt__read_string(&read, &str);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_uint32(str.size, ==, 9);
    munit_assert_not_null(str.data);
    munit_assert_string_equal(str.data, "foobarbaz");
    cnbt__free_string(&str);
  }

  // Null terminator
  ret = cnbt__read_type(&read, &type);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(type == CNBT_TYPE_END);

  return MUNIT_OK;
}

static cnbt_IoFunc FILE_CBS = {
  .read = (PFN_cnbt_read_func)fread,
  .write = (PFN_cnbt_write_func)fwrite,
  .seek = (PFN_cnbt_seek_func)fseek,
  .tell = (PFN_cnbt_tell_func)ftell,
};

static cnbt_Status read_compound(const char* path, cnbt_Compound* comp, cnbt_EndianMode mode) {
  FILE* f = fopen(path, "rb");
  munit_assert_not_null(f); // check your paths if this fails
  cnbt_Status ret = cnbt_read(comp, mode, f, &FILE_CBS);
  fclose(f);
  return ret;
}

static void check_file_compound(cnbt_Compound* comp) {
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

static MunitResult test_read_compound_uncompressed(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status ret;
  cnbt_Compound comp = {0};

  // bigtest
  ret = read_compound(TESTFILES "/bigtest_uncompr", &comp, CNBT_BIG_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);
  check_file_compound(&comp);
  cnbt_free(&comp);

  // littletest
  ret = read_compound(TESTFILES "/littletest_uncompr", &comp, CNBT_LITTLE_ENDIAN);
  munit_assert_true(ret == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);
  check_file_compound(&comp);
  cnbt_free(&comp);

  return MUNIT_OK;
}

static MunitResult test_write_pretty(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Tag comp;
  cnbt_make_compound(&comp);

  cnbt_Tag tag, list;
  cnbt_make_byte(&tag, 0);
  cnbt_comp_insert(&comp, "the_byte", tag);

  cnbt_make_short(&tag, 124);
  cnbt_comp_insert(&comp, "the_short", tag);

  cnbt_make_int(&tag, 144);
  cnbt_comp_insert(&comp, "the_int", tag);

  cnbt_make_long(&tag, 999);
  cnbt_comp_insert(&comp, "the_long", tag);

  cnbt_make_double(&tag, 333);
  cnbt_comp_insert(&comp, "some_double", tag);

  cnbt_make_list(&list);
  cnbt_make_short(&tag, 4511);
  cnbt_list_push(&list, tag);
  cnbt_make_short(&tag, 5555);
  cnbt_list_push(&list, tag);
  cnbt_comp_insert(&comp, "funny_list", list);

  cnbt_make_str(&tag, "hehahah");
  cnbt_comp_insert(&comp, "my_string", tag);

  int8_t bytes[] = {0, 1, 2, 3, 4, -1};
  cnbt_make_byte_array(&tag, bytes, sizeof(bytes));
  cnbt_comp_insert(&comp, "bytes", tag);

  BuffCtx ctx = {0};
  cnbt_write_pretty(&comp, &ctx, &BUFFER_CBS);

  static const char pretty_expected[] = R"({
  the_byte: 0b
  the_short: 124s
  the_int: 144i
  the_long: 999l
  some_double: 333.000000d
  funny_list: [4511s, 5555s]
  my_string: "hehahah"
  bytes: [0x00, 0x01, 0x02, 0x03, 0x04, 0xFF]
}
)";
  munit_assert_string_equal(pretty_expected, ctx.data);
  cnbt_free(&comp);

  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/read-big-endian", test_read_endianness, test_read_endianness_big_setup,
   test_read_endianness_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read-little-endian", test_read_endianness, test_read_endianness_little_setup,
   test_read_endianness_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
  {"/read-compound-uncompressed", test_read_compound_uncompressed, NULL, NULL,
   MUNIT_TEST_OPTION_NONE, NULL},
  {"/write-pretty", test_write_pretty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

TestData io_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
