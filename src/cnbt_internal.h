#ifndef CNBT_INTERNAL_H
#define CNBT_INTERNAL_H

#include <assert.h>
#include <stdint.h>

#define CNBT_MALLOC  malloc
#define CNBT_FREE    free
#define CNBT_REALLOC realloc

#define ARRSZ(_arr) sizeof(_arr) / sizeof(_arr[0])
#define UNUSED(_o)  (void)_o

#define CNBT_TYPE_NULL -1

typedef int8_t i8;
typedef uint16_t u16;
typedef int16_t i16;
typedef uint32_t u32;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
static_assert(sizeof(f32) == 4, "float size mismatch");
typedef double f64;
static_assert(sizeof(f64) == 8, "double size mismatch");

struct cnbt_KeyTag;

typedef struct cnbt__StringData {
  char* data;
  u32 size;
} cnbt__StringData;

typedef struct cnbt__ByteArrayData {
  i8* data;
  u32 size;
} cnbt__ByteArrayData;

typedef struct cnbt__ListData {
  struct cnbt_Tag* data;
  u32 size;
} cnbt__ListData;

typedef struct cnbt__CompoundData {
  struct cnbt_KeyTag* data;
  u32 size;
} cnbt__CompoundData;

struct cnbt_Tag {
  u32 type;

  union {
    i8 as_i8;
    i16 as_i16;
    i32 as_i32;
    i64 as_i64;
    f32 as_f32;
    f64 as_f64;
    cnbt__StringData as_string;
    cnbt__ByteArrayData as_bytearr;
    cnbt__ListData as_list;
    cnbt__CompoundData as_compound;
  };
};

#define CNBT__INTERNAL
#include "../include/cnbt/cnbt.h"

static_assert(sizeof(struct cnbt_Tag) <= sizeof(struct cnbt_Tag_pub), "cnbt_Tag size mismatch");

typedef struct cnbt__ReadCtx {
  cnbt_IoFunc cbs;
  void* src;
  cnbt_EndianMode endian_mode;
} cnbt__ReadCtx;

void cnbt__free_byte_array(cnbt__ByteArrayData* arr);
void cnbt__free_string(cnbt__StringData* str);
void cnbt__free_list(cnbt__ListData* list);
void cnbt__free_compound(cnbt__CompoundData* comp);

cnbt_Status cnbt__read_type(cnbt__ReadCtx* ctx, cnbt_Type* type);
cnbt_Status cnbt__read_byte(cnbt__ReadCtx* ctx, i8* num);
cnbt_Status cnbt__read_short(cnbt__ReadCtx* ctx, i16* num);
cnbt_Status cnbt__read_int(cnbt__ReadCtx* ctx, i32* num);
cnbt_Status cnbt__read_long(cnbt__ReadCtx* ctx, i64* num);
cnbt_Status cnbt__read_float(cnbt__ReadCtx* ctx, f32* num);
cnbt_Status cnbt__read_double(cnbt__ReadCtx* ctx, f64* num);
cnbt_Status cnbt__read_string(cnbt__ReadCtx* ctx, cnbt__StringData* str);
cnbt_Status cnbt__read_byte_array(cnbt__ReadCtx* ctx, cnbt__ByteArrayData* arr);
cnbt_Status cnbt__read_list(cnbt__ReadCtx* ctx, cnbt__ListData* list);
cnbt_Status cnbt__read_compound(cnbt__ReadCtx* ctx, cnbt__CompoundData* comp);

#endif // CNBT_INTERNAL_H
