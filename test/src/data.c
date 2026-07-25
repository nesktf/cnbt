#include "../../include/cnbt/cnbt.h"

#include <assert.h>
#include <string.h>

int main() {
  CNBT_Status res;

  {
    CNBT_Byte byte_tag;
    res = cnbt_make_byte(&byte_tag, 47, "my byte");
    assert(!res);
    assert(cnbt_get_type(&byte_tag) == CNBT_TYPE_BYTE);
    assert(!strcmp(cnbt_get_name(&byte_tag), "my byte"));

    int8_t byte_value = cnbt_get_byte(&byte_tag);
    assert(byte_value == 47);
    cnbt_destroy(&byte_tag);
  }
  {
    CNBT_Short short_tag;
    res = cnbt_make_short(&short_tag, 4124, "my short");
    assert(!res);
    assert(cnbt_get_type(&short_tag) == CNBT_TYPE_SHORT);
    assert(!strcmp(cnbt_get_name(&short_tag), "my short"));

    int16_t short_value = cnbt_get_short(&short_tag);
    assert(short_value == 4124);
    cnbt_destroy(&short_tag);
  }
  {
    CNBT_Int int_tag;
    res = cnbt_make_int(&int_tag, 214214, "my integer");
    assert(!res);
    assert(cnbt_get_type(&int_tag) == CNBT_TYPE_INT);
    assert(!strcmp(cnbt_get_name(&int_tag), "my integer"));

    int32_t int_value = cnbt_get_int(&int_tag);
    assert(int_value == 214214);
    cnbt_destroy(&int_tag);
  }
  {
    CNBT_Long long_tag;
    res = cnbt_make_long(&long_tag, 12125125512, "my long");
    assert(!res);
    assert(cnbt_get_type(&long_tag) == CNBT_TYPE_LONG);
    assert(!strcmp(cnbt_get_name(&long_tag), "my long"));

    int64_t long_value = cnbt_get_long(&long_tag);
    assert(long_value == 12125125512);
    cnbt_destroy(&long_tag);
  }
  {
    CNBT_Float float_tag;
    res = cnbt_make_float(&float_tag, 12412.f, "my float");
    assert(!res);
    assert(cnbt_get_type(&float_tag) == CNBT_TYPE_FLOAT);
    assert(!strcmp(cnbt_get_name(&float_tag), "my float"));

    float float_value = cnbt_get_float(&float_tag);
    assert(float_value == 12412.f);
    cnbt_destroy(&float_tag);
  }
  {
    CNBT_Double double_tag;
    res = cnbt_make_double(&double_tag, 24214., "my double");
    assert(!res);
    assert(cnbt_get_type(&double_tag) == CNBT_TYPE_DOUBLE);
    assert(!strcmp(cnbt_get_name(&double_tag), "my double"));

    double double_value = cnbt_get_double(&double_tag);
    assert(double_value == 24214.);
    cnbt_destroy(&double_tag);
  }
}
