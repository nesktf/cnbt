#include "tests.h"

#include <string.h>

static MunitResult test_primitive_init(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  {
    cnbt_Byte byte_tag;
    cnbt_make_byte(&byte_tag, 47);
    munit_assert_true(cnbt_get_type(&byte_tag) == CNBT_TYPE_BYTE);

    int8_t byte_value = cnbt_get_byte(&byte_tag);
    munit_assert_true(byte_value == 47);
    cnbt_free(&byte_tag);
  }
  {
    cnbt_Short short_tag;
    cnbt_make_short(&short_tag, 4124);
    munit_assert_true(cnbt_get_type(&short_tag) == CNBT_TYPE_SHORT);

    int16_t short_value = cnbt_get_short(&short_tag);
    munit_assert_true(short_value == 4124);
    cnbt_free(&short_tag);
  }
  {
    cnbt_Int int_tag;
    cnbt_make_int(&int_tag, 214214);
    munit_assert_true(cnbt_get_type(&int_tag) == CNBT_TYPE_INT);

    int32_t int_value = cnbt_get_int(&int_tag);
    munit_assert_true(int_value == 214214);
    cnbt_free(&int_tag);
  }
  {
    cnbt_Long long_tag;
    cnbt_make_long(&long_tag, 12125125512);
    munit_assert_true(cnbt_get_type(&long_tag) == CNBT_TYPE_LONG);

    int64_t long_value = cnbt_get_long(&long_tag);
    munit_assert_true(long_value == 12125125512);
    cnbt_free(&long_tag);
  }
  {
    cnbt_Float float_tag;
    cnbt_make_float(&float_tag, 12412.f);
    munit_assert_true(cnbt_get_type(&float_tag) == CNBT_TYPE_FLOAT);

    float float_value = cnbt_get_float(&float_tag);
    munit_assert_true(float_value == 12412.f);
    cnbt_free(&float_tag);
  }
  {
    cnbt_Double double_tag;
    cnbt_make_double(&double_tag, 24214.);
    munit_assert_true(cnbt_get_type(&double_tag) == CNBT_TYPE_DOUBLE);

    double double_value = cnbt_get_double(&double_tag);
    munit_assert_true(double_value == 24214.);
    cnbt_free(&double_tag);
  }
  return MUNIT_OK;
}

static MunitResult test_list_init(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status res;
  cnbt_Tag* tag;

  cnbt_List list;
  res = cnbt_make_list(&list);
  munit_assert_true(res == CNBT_OK);
  munit_assert_true(cnbt_get_type(&list) == CNBT_TYPE_LIST);

  {
    cnbt_Float float_tag;
    cnbt_make_float(&float_tag, 1.f);
    tag = cnbt_list_push(&list, float_tag);
    munit_assert_ptr_not_null(tag);
    munit_assert_true(cnbt_get_type(tag) == CNBT_TYPE_FLOAT);
    munit_assert_true(cnbt_get_float(tag) == 1.f);
  }

  {
    cnbt_String str_tag;
    res = cnbt_make_str(&str_tag, "my funny string");
    munit_assert_true(res == CNBT_OK);
    tag = cnbt_list_push(&list, str_tag);
    munit_assert_ptr_not_null(tag);
    munit_assert_true(cnbt_get_type(tag) == CNBT_TYPE_STRING);
    char* str = cnbt_str_data(tag);
    munit_assert_ptr_not_null(str);
    munit_assert_string_equal(str, "my funny string");
  }

  {
    cnbt_ByteArray arr_tag;
    int8_t data[] = {0, 1, 2, 3, 4, 5};
    res = cnbt_make_byte_array(&arr_tag, data, sizeof(data));
    munit_assert_true(res == CNBT_OK);

    tag = cnbt_list_push(&list, arr_tag);
    munit_assert_ptr_not_null(tag);
    munit_assert_true(cnbt_get_type(tag) == CNBT_TYPE_BYTE_ARRAY);

    int8_t* read = cnbt_byte_array_data(tag);
    munit_assert_ptr_not_null(read);
    munit_assert_memory_equal(sizeof(data), data, read);
  }

  size_t len = cnbt_list_len(&list);
  munit_assert_true(len == 3);
  tag = cnbt_list_get_unchecked(&list, 0);
  munit_assert_true(cnbt_get_type(tag) == CNBT_TYPE_FLOAT);
  tag = cnbt_list_get_unchecked(&list, 1);
  munit_assert_true(cnbt_get_type(tag) == CNBT_TYPE_STRING);
  tag = cnbt_list_get_unchecked(&list, 2);
  munit_assert_true(cnbt_get_type(tag) == CNBT_TYPE_BYTE_ARRAY);

  cnbt_free(&list);
  return MUNIT_OK;
}

static MunitResult test_compound_init(const MunitParameter* params, void* data) {
  UNUSED(params);
  UNUSED(data);

  cnbt_Status res;
  cnbt_KeyTag* tag;

  cnbt_Compound comp;
  res = cnbt_make_compound(&comp);
  munit_assert_true(res == CNBT_OK);
  munit_assert_true(cnbt_get_type(&comp) == CNBT_TYPE_COMPOUND);

  {
    cnbt_Float float_tag;
    cnbt_make_float(&float_tag, 1.f);
    tag = cnbt_comp_insert(&comp, "my float", float_tag);
    munit_assert_true(res == CNBT_OK);
    munit_assert_ptr_not_null(tag);
    munit_assert_string_equal(tag->key, "my float");
    munit_assert_true(cnbt_get_float(&tag->value) == 1.f);
  }

  tag = cnbt_comp_get(&comp, "asddsa");
  munit_assert_ptr_null(tag);

  tag = cnbt_comp_get(&comp, "my float");
  munit_assert_ptr_not_null(tag);
  munit_assert_string_equal(tag->key, "my float");
  munit_assert_true(cnbt_get_float(&tag->value) == 1.f);

  cnbt_free(&comp);
  return MUNIT_OK;
}

static MunitTest tests[] = {
  {"/tag/primitive-init", test_primitive_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {"/tag/list-init", test_list_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
  {"/tag/compound-init", test_compound_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};

TestData tag_tests = {
  .tests = tests,
  .len = ARRSZ(tests),
};
