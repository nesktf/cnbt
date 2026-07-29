#include "tests.h"
#include <cnbt/cnbt.h>

#include <stdio.h>

typedef struct BuffCtx {
  char data[1024];
  size_t pos;
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

static CNBT_IoCallbacks BUFFER_CBS = {
  .read = (PFN_cnbt_read_func)read_func,
  .write = (PFN_cnbt_write_func)write_func,
  .seek = NULL,
  .tell = NULL,
};

// clang-format off
static const uint8_t read_basic_data[] = {
  0x03, // CNBT_TAG_TYPE_INT,
  0x00, 0x00, 0x00, 0x00, // num = 0

  0x08, // CNBT_TAG_TYPE_STRING
  0x09, 0x00, // len = 9, little
  'f', 'o', 'o', 'b', 'a', 'r', 'b', 'a', 'z',

  0x00, // CNBT_TAG_TYPE_END
};
// clang-format on

MunitResult test_read_basic(const MunitParameter* params, void* data) {
  (void)params;
  (void)data;

  CNBT_Status ret;
  CNBT_Type type;
  BuffCtx ctx = {0};
  memcpy(ctx.data, read_basic_data, sizeof(read_basic_data));

  CNBT__ReadCtx read;
  read.cbs = &BUFFER_CBS;
  read.src = &ctx;

  {
    ret = cnbt__read_type(&read, &type);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_true(type == CNBT_TYPE_INT);
    int32_t num_int;

    ret = cnbt__read_int(&read, &num_int);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_int32(num_int, ==, 0);
  }
  {
    ret = cnbt__read_type(&read, &type);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_true(type == CNBT_TYPE_STRING);

    char* str = NULL;
    uint32_t str_len = 0;
    ret = cnbt__read_string(&read, &str, &str_len);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_uint32(str_len, ==, 9);
    munit_assert_not_null(str);
    munit_assert_string_equal(str, "foobarbaz");
    CNBT__FREE(str);
  }
  {
    ret = cnbt__read_type(&read, &type);
    munit_assert_true(ret == CNBT_OK);
    munit_assert_true(type == CNBT_TYPE_END);
  }

  return MUNIT_OK;
}

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

MunitResult test_write_pretty(const MunitParameter* params, void* data) {
  (void)params;
  (void)data;

  CNBT_Tag comp;
  cnbt_make_compound(&comp);

  CNBT_Tag tag, list;
  cnbt_make_byte(&tag, 0);
  cnbt_comp_put_tag(&comp, "the_byte", tag);

  cnbt_make_short(&tag, 124);
  cnbt_comp_put_tag(&comp, "the_short", tag);

  cnbt_make_int(&tag, 144);
  cnbt_comp_put_tag(&comp, "the_int", tag);

  cnbt_make_long(&tag, 999);
  cnbt_comp_put_tag(&comp, "the_long", tag);

  cnbt_make_double(&tag, 333);
  cnbt_comp_put_tag(&comp, "some_double", tag);

  cnbt_make_list(&list);
  cnbt_make_short(&tag, 4511);
  cnbt_list_put_tag(&list, tag);
  cnbt_make_short(&tag, 5555);
  cnbt_list_put_tag(&list, tag);
  cnbt_comp_put_tag(&comp, "funny_list", list);

  cnbt_make_str(&tag, "hehahah");
  cnbt_comp_put_tag(&comp, "my_string", tag);

  int8_t bytes[] = {0, 1, 2, 3, 4, -1};
  cnbt_make_byte_array(&tag, bytes, sizeof(bytes));
  cnbt_comp_put_tag(&comp, "bytes", tag);

  BuffCtx ctx = {0};
  cnbt_write_pretty(&comp, &ctx, &BUFFER_CBS);
  munit_assert_string_equal(pretty_expected, ctx.data);
  cnbt_free(&comp);

  return MUNIT_OK;
}
