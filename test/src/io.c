#include "tests.h"
#include <cnbt/cnbt.h>

#include <stdio.h>

static const char out[] = R"({
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

typedef struct ReadCtx {
  char data[1024];
  size_t pos;
} ReadCtx;

static size_t write_func(const void* buff, size_t sz, size_t nmemb, void* src) {
  ReadCtx* ctx = src;
  memcpy(ctx->data+ctx->pos, buff,sz*nmemb);
  ctx->pos += sz*nmemb;
  return sz*nmemb;
}

static CNBT_IoCallbacks cbs = {
  .read = NULL,
  .write = write_func,
  .seek = NULL,
  .tell = NULL,
};

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

  ReadCtx ctx = {0};
  cnbt_write_pretty(&comp, &ctx, &cbs);
  munit_assert_string_equal(out, ctx.data);
  cnbt_free(&comp);

  return MUNIT_OK;
}
