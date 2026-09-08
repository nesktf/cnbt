#include "cnbt_internal.h"

#include <string.h>

#define STBDS_FREE(_ctx, _ptr)         CNBT_FREE(_ptr)
#define STBDS_REALLOC(_ctx, _ptr, _sz) CNBT_REALLOC(_ptr, _sz)
#define STBDS_ASSERT                   assert

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

CNBT_API const char* cnbt_tag_name(cnbt_Type type) {
  static const char* names[] = {"TAG_End",    "TAG_Byte",  "TAG_Short",   "TAG_Int",
                                "TAG_Long",   "TAG_Float", "TAG_Double",  "TAG_Byte_Array",
                                "TAG_String", "TAG_List",  "TAG_Compound"};
  return type > ARRSZ(names) ? "UNKNOWN" : names[type];
}

CNBT_API cnbt_Type cnbt_get_type(const cnbt_Tag* tag) {
  return tag ? (cnbt_Type)tag->type : CNBT_TYPE_END;
}

CNBT_API void cnbt_free(cnbt_Tag* tag) {
  if (!tag) {
    return;
  }
  const cnbt_Type type = tag->type;
  switch (type) {
    case CNBT_TYPE_LIST:
      cnbt__free_list(&tag->as_list);
      break;
    case CNBT_TYPE_COMPOUND:
      cnbt__free_compound(&tag->as_compound);
      break;
    case CNBT_TYPE_STRING:
      cnbt__free_string(&tag->as_string);
      break;
    case CNBT_TYPE_BYTE_ARRAY:
      cnbt__free_byte_array(&tag->as_bytearr);
      break;
    default:
      break;
  }
}

CNBT_API void cnbt_make_end(cnbt_Tag* tag) {
  if (!tag) {
    return;
  }
  memset(tag, 0x00, sizeof(*tag));
}

CNBT_API void cnbt_make_byte(cnbt_Byte* tag, int8_t value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    tag->type = CNBT_TYPE_BYTE;
    tag->as_i8 = value;
  }
}

CNBT_API int8_t cnbt_get_byte(const cnbt_Byte* tag) {
  return tag ? tag->type == CNBT_TYPE_BYTE ? tag->as_i8 : 0 : 0;
}

CNBT_API void cnbt_make_short(cnbt_Short* tag, int16_t value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    tag->type = CNBT_TYPE_SHORT;
    tag->as_i16 = value;
  }
}

CNBT_API int16_t cnbt_get_short(const cnbt_Short* tag) {
  return tag ? tag->type == CNBT_TYPE_SHORT ? tag->as_i16 : 0 : 0;
}

CNBT_API void cnbt_make_int(cnbt_Int* tag, int32_t value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    tag->type = CNBT_TYPE_INT;
    tag->as_i32 = value;
  }
}

CNBT_API int32_t cnbt_get_int(const cnbt_Int* tag) {
  return tag ? tag->type == CNBT_TYPE_INT ? tag->as_i32 : 0 : 0;
}

CNBT_API void cnbt_make_long(cnbt_Long* tag, int64_t value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    tag->type = CNBT_TYPE_LONG;
    tag->as_i64 = value;
  }
}

CNBT_API int64_t cnbt_get_long(const cnbt_Long* tag) {
  return tag ? tag->type == CNBT_TYPE_LONG ? tag->as_i64 : 0 : 0;
}

CNBT_API void cnbt_make_float(cnbt_Float* tag, float value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    tag->type = CNBT_TYPE_FLOAT;
    tag->as_f32 = value;
  }
}

CNBT_API float cnbt_get_float(const cnbt_Float* tag) {
  return tag ? tag->type == CNBT_TYPE_FLOAT ? tag->as_f32 : 0.f : 0.f;
}

CNBT_API void cnbt_make_double(cnbt_Double* tag, double value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    tag->type = CNBT_TYPE_DOUBLE;
    tag->as_f64 = value;
  }
}

CNBT_API double cnbt_get_double(const cnbt_Double* tag) {
  return tag ? tag->type == CNBT_TYPE_DOUBLE ? tag->as_f64 : 0. : 0.;
}

static cnbt_Status cnbt__init_str(cnbt_String* tag, const char* data, size_t len) {
  if (len > CNBT_MAX_STRING_SIZE) {
    return CNBT_INVALID_DATA;
  }
  char* ptr = CNBT_MALLOC(len + 1);
  if (!ptr) {
    return CNBT_ALLOC_FAILED;
  }

  memset(tag, 0x00, sizeof(*tag));
  memcpy(ptr, data, len);
  ptr[len] = '\0';
  tag->type = CNBT_TYPE_STRING;
  tag->as_string.data = ptr;
  tag->as_string.size = (u32)len;
  return CNBT_OK;
}

CNBT_API cnbt_Status cnbt_make_str(cnbt_String* tag, const char* str) {
  if (!str || !tag) {
    return CNBT_INVALID_DATA;
  }
  return cnbt__init_str(tag, str, strlen(str));
}

CNBT_API cnbt_Status cnbt_make_strn(cnbt_String* tag, const char* str, size_t n) {
  if (!str || !tag || !n) {
    return CNBT_INVALID_DATA;
  }
  return cnbt__init_str(tag, str, n);
}

void cnbt__free_string(cnbt__StringData* str) {
  if (!str) {
    return;
  }
  CNBT_FREE(str->data);
}

CNBT_API char* cnbt_str_data(const cnbt_String* tag) {
  return tag ? tag->type == CNBT_TYPE_STRING ? tag->as_string.data : NULL : NULL;
}

CNBT_API size_t cnbt_str_len(const cnbt_String* tag) {
  return tag ? tag->type == CNBT_TYPE_STRING ? (size_t)tag->as_string.size : 0 : 0;
}

CNBT_API cnbt_Status cnbt_make_byte_array(cnbt_ByteArray* arr, const void* data, size_t n) {
  if (!arr || !data || !n || n > CNBT_MAX_LIST_SIZE) {
    return CNBT_INVALID_DATA;
  }

  int8_t* blob = CNBT_MALLOC(n);
  if (!blob) {
    return CNBT_ALLOC_FAILED;
  }

  memcpy(blob, data, n);
  arr->type = CNBT_TYPE_BYTE_ARRAY;
  arr->as_bytearr.data = blob;
  arr->as_bytearr.size = (uint32_t)n;
  return CNBT_OK;
}

CNBT_API size_t cnbt_byte_array_len(const cnbt_ByteArray* arr) {
  return arr ? arr->type == CNBT_TYPE_BYTE_ARRAY ? arr->as_bytearr.size : 0 : 0;
}

CNBT_API int8_t* cnbt_byte_array_data(const cnbt_ByteArray* arr) {
  return arr ? arr->type == CNBT_TYPE_BYTE_ARRAY ? arr->as_bytearr.data : NULL : NULL;
}

CNBT_API cnbt_Status cnbt_make_list(cnbt_List* list) {
  if (!list) {
    return CNBT_INVALID_DATA;
  }
  memset(list, 0x00, sizeof(*list));
  list->type = CNBT_TYPE_LIST;
  return CNBT_OK;
}

void cnbt__free_list(cnbt__ListData* list) {
  if (!list) {
    return;
  }
  for (size_t i = 0; i < stbds_arrlenu(list->data); ++i) {
    cnbt_free(list->data + i);
  }
  stbds_arrfree(list->data);
}

void cnbt__free_byte_array(cnbt__ByteArrayData* arr) {
  if (!arr) {
    return;
  }
  CNBT_FREE(arr->data);
}

CNBT_API cnbt_Tag* cnbt_list_push(cnbt_List* list, cnbt_Tag tag) {
  if (!list) {
    return NULL;
  }
  if (list->type != CNBT_TYPE_LIST) {
    return NULL;
  }
  stbds_arrput(list->as_list.data, tag);
  return list->as_list.data + stbds_arrlen(list->as_list.data) - 1;
}

CNBT_API size_t cnbt_list_len(const cnbt_List* list) {
  return list ? list->type == CNBT_TYPE_LIST ? stbds_arrlen(list->as_list.data) : 0 : 0;
}

CNBT_API cnbt_Tag* cnbt_list_get_unchecked(const cnbt_List* list, size_t pos) {
  assert(list);
  assert(list->type == CNBT_TYPE_LIST);
  assert(pos < stbds_arrlen(list->as_list.data));
  return list->as_list.data + pos;
}

CNBT_API cnbt_Tag* cnbt_list_get(const cnbt_List* list, size_t pos) {
  if (!list) {
    return NULL;
  }
  if (list->type != CNBT_TYPE_LIST) {
    return NULL;
  }
  if (pos >= stbds_arrlen(list->as_list.data)) {
    return NULL;
  }
  return list->as_list.data + pos;
}

CNBT_API cnbt_Status cnbt_make_compound(cnbt_Compound* comp) {
  if (!comp) {
    return CNBT_INVALID_DATA;
  }
  memset(comp, 0x00, sizeof(*comp));
  comp->type = CNBT_TYPE_COMPOUND;
  return CNBT_OK;
}

void cnbt__free_compound(cnbt__CompoundData* comp) {
  if (!comp) {
    return;
  }
  for (size_t i = 0; i < stbds_shlenu(comp->data); ++i) {
    cnbt_free(&comp->data[i].value);
  }
  stbds_shfree(comp->data);
}

CNBT_API cnbt_KeyTag* cnbt_comp_insert(cnbt_Compound* comp, const char* key, cnbt_Tag tag) {
  if (!comp || !key) {
    return NULL;
  }
  if (comp->type != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  stbds_shput(comp->as_compound.data, key, tag);
  return shgetp_null(comp->as_compound.data, key);
}

CNBT_API size_t cnbt_comp_len(const cnbt_Compound* comp) {
  if (!comp) {
    return 0;
  }
  return stbds_shlenu(comp->as_compound.data);
}

CNBT_API cnbt_KeyTag* cnbt_comp_get(const cnbt_Compound* comp, const char* key) {
  if (!comp || !key) {
    return NULL;
  }
  if (comp->type != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  cnbt_KeyTag* tags = (cnbt_KeyTag*)comp->as_compound.data;
  return shgetp_null(tags, key);
}
