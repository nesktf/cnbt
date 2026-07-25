#include "core.h"

CNBT_API const char* cnbt_tag_name(CNBT_Type type) {
  static const char* names[] = {"TAG_End",    "TAG_Byte",  "TAG_Short",   "TAG_Int",
                                "TAG_Long",   "TAG_Float", "TAG_Double",  "TAG_Byte_Array",
                                "TAG_String", "TAG_List",  "TAG_Compound"};
  return type > CNBT_ARRSZ(names) ? "UNKNOWN" : names[type];
}

CNBT_API CNBT_Type cnbt_get_type(CNBT_Tag* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag : CNBT_TYPE_END;
}

CNBT_API void cnbt_destroy(CNBT_Tag* tag) {
  if (!tag) {
    return;
  }
  const uint8_t type = CNBT__GET_DATA(tag)->tag;
  switch (type) {
    case CNBT_TYPE_LIST:
      cnbt__free_list(tag);
      break;
    case CNBT_TYPE_COMPOUND:
      cnbt__free_compound(tag);
      break;
    case CNBT_TYPE_STRING:
    case CNBT_TYPE_BLOB:
      memset(tag, 0x00, sizeof(*tag));
      CNBT__FREE(CNBT__GET_DATA(tag)->as_blob);
      break;
    default:
      memset(tag, 0x00, sizeof(*tag));
      break;
  }
  if (CNBT__GET_DATA(tag)->name) {
    CNBT__FREE(CNBT__GET_DATA(tag)->name);
  }
}

CNBT_API void cnbt_make_end(CNBT_Tag* tag) {
  if (!tag) {
    return;
  }
  memset(tag, 0x00, sizeof(*tag));
}

CNBT_API char* cnbt_get_name(CNBT_Tag* tag) {
  return tag ? CNBT__GET_DATA(tag)->name : NULL;
}

CNBT_Status cnbt__init_tag(CNBT_Tag* tag, const char* name) {
  if (!tag || !name) {
    return CNBT_INVALID_DATA;
  }
  const size_t len = strlen(name);
  if (!len) {
    return CNBT_INVALID_DATA;
  }

  char* str = CNBT__MALLOC(len + 1);
  if (!str) {
    return CNBT_ALLOC_FAILED;
  }
  memcpy(str, name, len);
  str[len] = '\0';
  memset(tag, 0x00, sizeof(*tag));
  return CNBT_OK;
}

static CNBT_Status cnbt__init_int(CNBT_Tag* tag, const char* name, int64_t value, CNBT_Type type) {
  CNBT_Status ret = cnbt__init_tag(tag, name);
  if (!ret) {
    CNBT__GET_DATA(tag)->tag = type;
    CNBT__GET_DATA(tag)->as_int = value;
  }
  return ret;
}

CNBT_API CNBT_Status cnbt_make_byte(CNBT_Byte* tag, int8_t value, const char* name) {
  return cnbt__init_int(tag, name, value, CNBT_TYPE_BYTE);
}

CNBT_API int8_t cnbt_get_byte(CNBT_Byte* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_BYTE ? (int8_t)CNBT__GET_DATA(tag)->as_int : 0
             : 0;
}

CNBT_API CNBT_Status cnbt_make_short(CNBT_Short* tag, int16_t value, const char* name) {
  return cnbt__init_int(tag, name, value, CNBT_TYPE_SHORT);
}

CNBT_API int16_t cnbt_get_short(CNBT_Short* tag) {
  return tag
         ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_SHORT ? (int16_t)CNBT__GET_DATA(tag)->as_int : 0
         : 0;
}

CNBT_API CNBT_Status cnbt_make_int(CNBT_Int* tag, int32_t value, const char* name) {
  return cnbt__init_int(tag, name, value, CNBT_TYPE_INT);
}

CNBT_API int32_t cnbt_get_int(CNBT_Int* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_INT ? (int32_t)CNBT__GET_DATA(tag)->as_int : 0
             : 0;
}

CNBT_API CNBT_Status cnbt_make_long(CNBT_Long* tag, int64_t value, const char* name) {
  return cnbt__init_int(tag, name, value, CNBT_TYPE_LONG);
}

CNBT_API int64_t cnbt_get_long(CNBT_Long* tag) {
  return tag
         ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_LONG ? (int64_t)CNBT__GET_DATA(tag)->as_int : 0
         : 0;
}

CNBT_API CNBT_Status cnbt_make_float(CNBT_Float* tag, float value, const char* name) {
  CNBT_Status ret = cnbt__init_tag(tag, name);
  if (!ret) {
    CNBT__GET_DATA(tag)->tag = CNBT_TYPE_FLOAT;
    CNBT__GET_DATA(tag)->as_float = value;
  }
  return ret;
}

CNBT_API float cnbt_get_float(CNBT_Float* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_FLOAT ? CNBT__GET_DATA(tag)->as_float : 0.f
             : 0.f;
}

CNBT_API CNBT_Status cnbt_make_double(CNBT_Double* tag, double value, const char* name) {
  CNBT_Status ret = cnbt__init_tag(tag, name);
  if (!ret) {
    CNBT__GET_DATA(tag)->tag = CNBT_TYPE_DOUBLE;
    CNBT__GET_DATA(tag)->as_double = value;
  }
  return ret;
}

CNBT_API double cnbt_get_double(CNBT_Double* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_DOUBLE ? CNBT__GET_DATA(tag)->as_double : 0.
             : 0.;
}

CNBT_API CNBT_Status cnbt_make_str(CNBT_String* tag, const char* data, const char* name) {
  CNBT_Status ret = CNBT_OK;
  char* str = NULL;
  if (!data) {
    ret = CNBT_INVALID_DATA;
    goto str_err;
  }

  const size_t len = strlen(data);
  str = CNBT__MALLOC(len + 1);
  if (!str) {
    ret = CNBT_ALLOC_FAILED;
    goto str_err;
  }

  ret = cnbt__init_tag(tag, name);
  if (!ret) {
    CNBT__GET_DATA(tag)->tag = CNBT_TYPE_STRING;
    CNBT__GET_DATA(tag)->as_blob = str;
    CNBT__GET_DATA(tag)->size = len;
    return ret;
  }

str_err:
  if (str) {
    CNBT__FREE(str);
  }
  return ret;
}

CNBT_API char* cnbt_get_string(CNBT_String* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_STRING ? CNBT__GET_DATA(tag)->as_blob : NULL
             : NULL;
}

CNBT_API size_t cnbt_strlen(CNBT_String* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_STRING ? CNBT__GET_DATA(tag)->size : 0 : 0;
}

CNBT_API CNBT_Status cnbt_make_blob(CNBT_Blob* tag, const void* data, size_t size,
                                    const char* name) {
  CNBT_Status ret = CNBT_OK;
  void* blob = NULL;
  if (!data || !size) {
    ret = CNBT_INVALID_DATA;
    goto blob_err;
  }

  blob = CNBT__MALLOC(size);
  if (!blob) {
    ret = CNBT_ALLOC_FAILED;
    goto blob_err;
  }

  ret = cnbt__init_tag(tag, name);
  if (!ret) {
    CNBT__GET_DATA(tag)->tag = CNBT_TYPE_BLOB;
    CNBT__GET_DATA(tag)->as_blob = blob;
    CNBT__GET_DATA(tag)->size = size;
    return ret;
  }

blob_err:
  if (blob) {
    CNBT__FREE(blob);
  }
  return ret;
}

CNBT_API size_t cnbt_blob_size(CNBT_Blob* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_BLOB ? CNBT__GET_DATA(tag)->size : 0 : 0;
}

CNBT_API void* cnbt_get_blob(CNBT_Blob* tag) {
  return tag ? CNBT__GET_DATA(tag)->tag == CNBT_TYPE_BLOB ? CNBT__GET_DATA(tag)->as_blob : NULL
             : NULL;
}
