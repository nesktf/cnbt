#include "core.h"

CNBT_API const char* cnbt_tag_name(CNBT_Type type) {
  static const char* names[] = {"TAG_End",    "TAG_Byte",  "TAG_Short",   "TAG_Int",
                                "TAG_Long",   "TAG_Float", "TAG_Double",  "TAG_Byte_Array",
                                "TAG_String", "TAG_List",  "TAG_Compound"};
  return type > CNBT_ARRSZ(names) ? "UNKNOWN" : names[type];
}

CNBT_API CNBT_Type cnbt_get_type(const CNBT_Tag* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag : CNBT_TYPE_END;
}

CNBT_API void cnbt_free(CNBT_Tag* tag) {
  if (!tag) {
    return;
  }
  const uint8_t type = CNBT__GET_DATA(tag)->tag;
  switch (type) {
    case CNBT_TYPE_LIST:
      cnbt__free_list(CNBT__GET_DATA(tag)->as_list);
      break;
    case CNBT_TYPE_COMPOUND:
      cnbt__free_compound(CNBT__GET_DATA(tag)->as_compound);
      break;
    case CNBT_TYPE_STRING:
      CNBT__FREE(CNBT__GET_DATA(tag)->as_str);
      break;
    case CNBT_TYPE_BYTE_ARRAY:
      CNBT__FREE(CNBT__GET_DATA(tag)->as_blob);
      break;
    default:
      break;
  }
}

CNBT_API void cnbt_make_end(CNBT_Tag* tag) {
  if (!tag) {
    return;
  }
  memset(tag, 0x00, sizeof(*tag));
}

static void cnbt__init_int(CNBT_Tag* tag, int64_t value, CNBT_Type type) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    CNBT__GET_DATA(tag)->tag = type;
    CNBT__GET_DATA(tag)->as_int = value;
  }
}

CNBT_API void cnbt_make_byte(CNBT_Byte* tag, int8_t value) {
  cnbt__init_int(tag, value, CNBT_TYPE_BYTE);
}

CNBT_API int8_t cnbt_get_byte(const CNBT_Byte* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_BYTE ? (int8_t)CNBT__GET_DATA(tag)->as_int : 0
             : 0;
}

CNBT_API void cnbt_make_short(CNBT_Short* tag, int16_t value) {
  cnbt__init_int(tag, value, CNBT_TYPE_SHORT);
}

CNBT_API int16_t cnbt_get_short(const CNBT_Short* tag) {
  return tag
         ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_SHORT ? (int16_t)CNBT__GET_DATA(tag)->as_int : 0
         : 0;
}

CNBT_API void cnbt_make_int(CNBT_Int* tag, int32_t value) {
  cnbt__init_int(tag, value, CNBT_TYPE_INT);
}

CNBT_API int32_t cnbt_get_int(const CNBT_Int* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_INT ? (int32_t)CNBT__GET_DATA(tag)->as_int : 0
             : 0;
}

CNBT_API void cnbt_make_long(CNBT_Long* tag, int64_t value) {
  cnbt__init_int(tag, value, CNBT_TYPE_LONG);
}

CNBT_API int64_t cnbt_get_long(const CNBT_Long* tag) {
  return tag
         ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_LONG ? (int64_t)CNBT__GET_DATA(tag)->as_int : 0
         : 0;
}

CNBT_API void cnbt_make_float(CNBT_Float* tag, float value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    CNBT__GET_DATA(tag)->tag = CNBT_TYPE_FLOAT;
    CNBT__GET_DATA(tag)->as_float = value;
  }
}

CNBT_API float cnbt_get_float(const CNBT_Float* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_FLOAT ? CNBT__GET_DATA(tag)->as_float : 0.f
             : 0.f;
}

CNBT_API void cnbt_make_double(CNBT_Double* tag, double value) {
  if (tag) {
    memset(tag, 0x00, sizeof(*tag));
    CNBT__GET_DATA(tag)->tag = CNBT_TYPE_DOUBLE;
    CNBT__GET_DATA(tag)->as_double = value;
  }
}

CNBT_API double cnbt_get_double(const CNBT_Double* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_DOUBLE ? CNBT__GET_DATA(tag)->as_double : 0.
             : 0.;
}

static CNBT_Status cnbt__init_str(CNBT_String* tag, const char* data, size_t len) {
  if (len > CNBT_MAX_STRING_SIZE) {
    return CNBT_INVALID_DATA;
  }
  char* ptr = CNBT__MALLOC(len + 1);
  if (!ptr) {
    return CNBT_ALLOC_FAILED;
  }

  memset(tag, 0x00, sizeof(*tag));
  CNBT__GET_DATA(tag)->tag = CNBT_TYPE_STRING;
  CNBT__GET_DATA(tag)->as_str= ptr;
  CNBT__GET_DATA(tag)->size = (uint32_t)len;
  return CNBT_OK;
}

CNBT_API CNBT_Status cnbt_make_str(CNBT_String* tag, const char* str) {
  if (!str || !tag) {
    return CNBT_INVALID_DATA;
  }
  return cnbt__init_str(tag, str, strlen(str));
}

CNBT_API CNBT_Status cnbt_make_strn(CNBT_String* tag, const char* str, size_t n) {
  if (!str || !tag || !n) {
    return CNBT_INVALID_DATA;
  }
  return cnbt__init_str(tag, str, n);
}

CNBT_API char* cnbt_get_string(const CNBT_String* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_STRING ? CNBT__GET_DATA(tag)->as_str: NULL
             : NULL;
}

CNBT_API size_t cnbt_strlen(const CNBT_String* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_STRING ? CNBT__GET_DATA(tag)->size : 0 : 0;
}

CNBT_API CNBT_Status cnbt_make_byte_array(CNBT_ByteArray* arr, const int8_t* data, size_t n) {
  if (!arr || !data || !n || n > CNBT_MAX_LIST_SIZE) {
    return CNBT_INVALID_DATA;
  }

  int8_t* blob = CNBT__MALLOC(n);
  if (!blob) {
    return CNBT_ALLOC_FAILED;
  }

  CNBT__GET_DATA(arr)->tag = CNBT_TYPE_BYTE_ARRAY;
  CNBT__GET_DATA(arr)->as_blob = blob;
  CNBT__GET_DATA(arr)->size = (uint32_t)n;
  return CNBT_OK;
}

CNBT_API size_t cnbt_byte_array_len(const CNBT_ByteArray* arr) {
  return arr ? CNBT__GET_DATA(arr)->tag == CNBT_TYPE_BYTE_ARRAY ? CNBT__GET_DATA(arr)->size : 0 : 0;
}

CNBT_API int8_t* cnbt_get_byte_array(const CNBT_ByteArray* arr) {
  return arr ? CNBT__GET_DATA(arr)->tag == CNBT_TYPE_BYTE_ARRAY ? CNBT__GET_DATA(arr)->as_blob : NULL
             : NULL;
}
