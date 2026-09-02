#include "cnbt_internal.h"

#include <string.h>

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
