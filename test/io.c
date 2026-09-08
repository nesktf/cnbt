#include "tests.h"

static size_t mem_read_func(void* buff, size_t sz, size_t nmemb, BuffCtx* ctx) {
  size_t total = sz * nmemb;
  if (total > sizeof(ctx->data)) {
    total = sizeof(ctx->data);
  }
  memcpy(buff, ctx->data + ctx->pos, total);
  ctx->pos += total;
  return nmemb;
}

static size_t mem_write_func(const void* buff, size_t sz, size_t nmemb, BuffCtx* ctx) {
  size_t total = sz * nmemb;
  if (total > sizeof(ctx->data)) {
    total = sizeof(ctx->data);
  }
  memcpy(ctx->data + ctx->pos, buff, total);
  ctx->pos += total;
  return nmemb;
}

static int mem_seek_func(BuffCtx* ctx, long offset, int origin) {
  if (origin == CNBT_SEEK_SET) {
    ctx->pos = offset;
  } else if (origin == CNBT_SEEK_END) {
    ctx->pos += offset;
    if (ctx->pos > (long)sizeof(ctx->data)) {
      ctx->pos = (long)sizeof(ctx->data);
    }
  } else if (origin == CNBT_SEEK_CUR) {
    ctx->pos = (long)sizeof(ctx->data) - offset;
    if (ctx->pos < 0) {
      ctx->pos = 0;
    }
  }
  return 0;
}

static long mem_tell_func(BuffCtx* ctx) {
  return ctx->pos;
}

static int mem_eof_func(BuffCtx* ctx) {
  return ctx->pos == sizeof(ctx->data);
}

cnbt_IoFunc BUFFER_CBS = {
  .read = (PFN_cnbt_read_func)mem_read_func,
  .write = (PFN_cnbt_write_func)mem_write_func,
  .seek = (PFN_cnbt_seek_func)mem_seek_func,
  .tell = (PFN_cnbt_tell_func)mem_tell_func,
  .eof = (PFN_cnbt_eof_func)mem_eof_func,
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
    ret = cnbt__read_byte_array(&read, &arr);
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

static MunitTest tests[] = {
  {"/io/read-big-endian", test_read_endianness, test_read_endianness_big_setup,
   test_read_endianness_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
  {"/io/read-little-endian", test_read_endianness, test_read_endianness_little_setup,
   test_read_endianness_tear_down, MUNIT_TEST_OPTION_NONE, NULL},
};

TestData io_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
