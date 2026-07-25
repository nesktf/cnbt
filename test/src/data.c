#include "../../include/cnbt/cnbt.h"

#include <assert.h>
#include <string.h>

static void test_integer() {
  {
    CNBT_Byte byte_tag;
    cnbt_make_byte(&byte_tag, 47);
    assert(cnbt_get_type(&byte_tag) == CNBT_TYPE_BYTE);

    int8_t byte_value = cnbt_get_byte(&byte_tag);
    assert(byte_value == 47);
    cnbt_free(&byte_tag);
  }
  {
    CNBT_Short short_tag;
    cnbt_make_short(&short_tag, 4124);
    assert(cnbt_get_type(&short_tag) == CNBT_TYPE_SHORT);

    int16_t short_value = cnbt_get_short(&short_tag);
    assert(short_value == 4124);
    cnbt_free(&short_tag);
  }
  {
    CNBT_Int int_tag;
    cnbt_make_int(&int_tag, 214214);
    assert(cnbt_get_type(&int_tag) == CNBT_TYPE_INT);

    int32_t int_value = cnbt_get_int(&int_tag);
    assert(int_value == 214214);
    cnbt_free(&int_tag);
  }
  {
    CNBT_Long long_tag;
    cnbt_make_long(&long_tag, 12125125512);
    assert(cnbt_get_type(&long_tag) == CNBT_TYPE_LONG);

    int64_t long_value = cnbt_get_long(&long_tag);
    assert(long_value == 12125125512);
    cnbt_free(&long_tag);
  }
  {
    CNBT_Float float_tag;
    cnbt_make_float(&float_tag, 12412.f);
    assert(cnbt_get_type(&float_tag) == CNBT_TYPE_FLOAT);

    float float_value = cnbt_get_float(&float_tag);
    assert(float_value == 12412.f);
    cnbt_free(&float_tag);
  }
  {
    CNBT_Double double_tag;
    cnbt_make_double(&double_tag, 24214.);
    assert(cnbt_get_type(&double_tag) == CNBT_TYPE_DOUBLE);

    double double_value = cnbt_get_double(&double_tag);
    assert(double_value == 24214.);
    cnbt_free(&double_tag);
  }
}

static void test_list() {
  CNBT_Status res;
  CNBT_Tag* tag;

  CNBT_List list;
  res = cnbt_make_list(&list);
  assert(!res);
  assert(cnbt_get_type(&list) == CNBT_TYPE_LIST);

  {
    CNBT_Float float_tag;
    cnbt_make_float(&float_tag, 1.f);
    tag = cnbt_list_put_tag(&list, float_tag);
    assert(tag);
    assert(cnbt_get_type(tag) == CNBT_TYPE_FLOAT);
    assert(cnbt_get_float(tag) == 1.f);
  }

  {
    CNBT_String str_tag;
    res = cnbt_make_str(&str_tag, "my funny string");
    assert(res);
    tag = cnbt_list_put_tag(&list, str_tag);
    assert(tag);
    assert(cnbt_get_type(tag) == CNBT_TYPE_STRING);
    char* str = cnbt_get_str(tag);
    assert(str);
    assert(!strcmp(str, "my funny string"));
  }

  {
    CNBT_ByteArray arr_tag;
    int8_t data[] = {0, 1, 2, 3, 4, 5};
    res = cnbt_make_byte_array(&arr_tag, data, sizeof(data));
    assert(res);

    tag = cnbt_list_put_tag(&list, arr_tag);
    assert(tag);
    assert(cnbt_get_type(tag) == CNBT_TYPE_BYTE_ARRAY);

    int8_t* read = cnbt_get_byte_array(tag);
    assert(read);
    assert(!memcmp(read, data, sizeof(data)));
  }

  size_t len = cnbt_list_len(&list);
  assert(len == 3);
  tag = cnbt_list_get_unchecked(&list, 0);
  assert(cnbt_get_type(tag) == CNBT_TYPE_FLOAT);
  tag = cnbt_list_get_unchecked(&list, 1);
  assert(cnbt_get_type(tag) == CNBT_TYPE_STRING);
  tag = cnbt_list_get_unchecked(&list, 2);
  assert(cnbt_get_type(tag) == CNBT_TYPE_BYTE_ARRAY);

  cnbt_free(&list);
}

int main() {
  test_integer();
  test_list();
}
