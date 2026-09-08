#include "tests.h"

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

  munit_assert_string_equal(pretty_expected, ctx.data);
  cnbt_free(&comp);

  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/formatter/write-pretty", test_write_pretty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

TestData formatter_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
