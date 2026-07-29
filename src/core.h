#ifndef CNBT_CORE_H_
#define CNBT_CORE_H_

#include "../include/cnbt/cnbt.h"
#include "../include/cnbt/stream.h"

#include <string.h>

#ifndef CNBT__ASSERT
#include <assert.h>
#define CNBT__ASSERT assert
#endif

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

#define CNBT_TYPE_NULL -1

typedef struct CNBT__Data {
  union {
    int64_t as_int;
    float as_float;
    double as_double;
    char* as_str;
    int8_t* as_blob;
    CNBT_Tag* as_list;
    CNBT_KeyTag* as_compound;
  };

  uint32_t tag;
  uint32_t size;
} CNBT__Data;

void cnbt__free_list(CNBT_Tag* list);
void cnbt__free_compound(CNBT_KeyTag* comp);

typedef struct CNBT__ReadCtx {
  const CNBT_IoCallbacks* cbs;
  void* src;
} CNBT__ReadCtx;

CNBT_Status cnbt__read_type(CNBT__ReadCtx* ctx, CNBT_Type* type);
CNBT_Status cnbt__read_byte(CNBT__ReadCtx* ctx, int8_t* num);
CNBT_Status cnbt__read_short(CNBT__ReadCtx* ctx, int16_t* num);
CNBT_Status cnbt__read_int(CNBT__ReadCtx* ctx, int32_t* num);
CNBT_Status cnbt__read_long(CNBT__ReadCtx* ctx, int64_t* num);
CNBT_Status cnbt__read_float(CNBT__ReadCtx* ctx, float* num);
CNBT_Status cnbt__read_double(CNBT__ReadCtx* ctx, double* num);
CNBT_Status cnbt__read_string(CNBT__ReadCtx* ctx, char** data, uint32_t* len);
CNBT_Status cnbt__read_blob(CNBT__ReadCtx* ctx, int8_t** data, uint32_t* len);
CNBT_Status cnbt__read_list(CNBT__ReadCtx* ctx, CNBT_Tag** data, uint32_t* len);
CNBT_Status cnbt__read_compound(CNBT__ReadCtx* ctx, CNBT_KeyTag** data, uint32_t* len);

#endif // CNBT_CORE_H_
