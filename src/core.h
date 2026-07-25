#ifndef CNBT_CORE_H_
#define CNBT_CORE_H_

#include "../include/cnbt/cnbt.h"

#include <assert.h>
#include <string.h>

#ifndef CNBT__MALLOC
#define CNBT__MALLOC malloc
#endif

#ifndef CNBT__FREE
#define CNBT__FREE free
#endif

#ifndef CNBT__REALLOC
#define CNBT__REALLOC realloc
#endif

#define CNBT_ARRSZ(_arr) sizeof(_arr) / sizeof(_arr[0])

#define CNBT__GET_DATA(_tag) ((CNBT__Data*)(_tag))

typedef struct CNBT__Data {
  union {
    int64_t as_int;
    float as_float;
    double as_double;
    char* as_blob;
    CNBT_Tag* as_list;
    CNBT_CompEntry* as_compound;
  };

  char* name;
  uint32_t tag;
  uint32_t size;
} CNBT__Data;

CNBT_Status cnbt__init_tag(CNBT_Tag* tag, const char* name);
void cnbt__free_list(CNBT_Tag* list);
void cnbt__free_compound(CNBT_Tag* comp);
void cnbt__free_blob(char* blob);

#endif // CNBT_CORE_H_
