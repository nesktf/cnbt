#ifndef CNBT_TEST_H_
#define CNBT_TEST_H_

#include "../../src/core.h"
#include <munit.h>

MunitResult test_int_init(const MunitParameter* params, void* data);
MunitResult test_list_init(const MunitParameter* params, void* data);
MunitResult test_comp_init(const MunitParameter* params, void* data);

MunitResult test_write_pretty(const MunitParameter* params, void* data);
MunitResult test_read_basic(const MunitParameter* params, void* data);

#endif // CNBT_TEST_H_
