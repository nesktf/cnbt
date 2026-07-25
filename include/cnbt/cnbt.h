#ifndef CNBT_H_
#define CNBT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CNBT_TAG_SIZE        16
#define CNBT_MAX_STRING_SIZE 0xFFFF
#define CNBT_MAX_LIST_SIZE   0x7FFFFFFF

#ifndef CNBT_API
#define CNBT_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CNBT_Status {
  CNBT_OK = 0,
  CNBT_INVALID_DATA,
  CNBT_ALLOC_FAILED,
} CNBT_Status;

typedef enum CNBT_Type {
  CNBT_TYPE_END = 0,
  CNBT_TYPE_BYTE,
  CNBT_TYPE_SHORT,
  CNBT_TYPE_INT,
  CNBT_TYPE_LONG,
  CNBT_TYPE_FLOAT,
  CNBT_TYPE_DOUBLE,
  CNBT_TYPE_BYTE_ARRAY,
  CNBT_TYPE_STRING,
  CNBT_TYPE_LIST,
  CNBT_TYPE_COMPOUND,
} CNBT_Type;

typedef struct CNBT_Tag {
  uint8_t data[CNBT_TAG_SIZE];
} CNBT_Tag;

typedef struct CNBT_KeyTag {
  char* key;
  CNBT_Tag value;
} CNBT_KeyTag;

typedef CNBT_Tag CNBT_Byte;
typedef CNBT_Tag CNBT_Short;
typedef CNBT_Tag CNBT_Int;
typedef CNBT_Tag CNBT_Long;
typedef CNBT_Tag CNBT_Float;
typedef CNBT_Tag CNBT_Double;
typedef CNBT_Tag CNBT_String;
typedef CNBT_Tag CNBT_List;
typedef CNBT_Tag CNBT_ByteArray;
typedef CNBT_Tag CNBT_Compound;

CNBT_API const char* cnbt_tag_name(CNBT_Type type);

CNBT_API void cnbt_make_end(CNBT_Tag* tag);
CNBT_API void cnbt_free(CNBT_Tag* tag);
CNBT_API CNBT_Type cnbt_get_type(const CNBT_Tag* tag);

CNBT_API void cnbt_make_byte(CNBT_Byte* tag, int8_t value);
CNBT_API int8_t cnbt_get_byte(const CNBT_Byte* tag);

CNBT_API void cnbt_make_short(CNBT_Short* tag, int16_t value);
CNBT_API int16_t cnbt_get_short(const CNBT_Short* tag);

CNBT_API void cnbt_make_int(CNBT_Int* tag, int32_t value);
CNBT_API int32_t cnbt_get_int(const CNBT_Int* tag);

CNBT_API void cnbt_make_long(CNBT_Long* tag, int64_t value);
CNBT_API int64_t cnbt_get_long(const CNBT_Long* tag);

CNBT_API void cnbt_make_float(CNBT_Float* tag, float value);
CNBT_API float cnbt_get_float(const CNBT_Float* tag);

CNBT_API void cnbt_make_double(CNBT_Double* tag, double value);
CNBT_API double cnbt_get_double(const CNBT_Double* tag);

CNBT_API CNBT_Status cnbt_make_str(CNBT_String* str, const char* value);
CNBT_API CNBT_Status cnbt_make_strn(CNBT_String* str, const char* value, size_t n);
CNBT_API size_t cnbt_strlen(const CNBT_String* str);
CNBT_API char* cnbt_get_str(const CNBT_String* str);

CNBT_API CNBT_Status cnbt_make_byte_array(CNBT_ByteArray* arr, const int8_t* data, size_t n);
CNBT_API size_t cnbt_byte_array_len(const CNBT_ByteArray* arr);
CNBT_API int8_t* cnbt_get_byte_array(const CNBT_ByteArray* arr);

CNBT_API CNBT_Status cnbt_make_list(CNBT_List* list);
CNBT_API CNBT_Status cnbt_make_list_fill(CNBT_List* list, const CNBT_Tag* data, size_t n);
CNBT_API CNBT_Tag* cnbt_list_put_tag(CNBT_List* list, CNBT_Tag tag);
CNBT_API size_t cnbt_list_len(const CNBT_List* list);
CNBT_API CNBT_Tag* cnbt_list_get(const CNBT_List* list, size_t pos);
CNBT_API CNBT_Tag* cnbt_list_get_unchecked(const CNBT_List* list, size_t pos);

CNBT_API CNBT_Status cnbt_make_compound(CNBT_Compound* comp);
CNBT_API CNBT_KeyTag* cnbt_comp_put_tag(CNBT_Compound* comp, const char* key, CNBT_Tag tag);
CNBT_API CNBT_KeyTag* cnbt_comp_get(const CNBT_Compound* comp, const char* key);

CNBT_API CNBT_Status cnbt_write(FILE* f, const CNBT_Compound* comp);
CNBT_API CNBT_Status cnbt_write_pretty(FILE* f, const CNBT_Compound* comp);
CNBT_API CNBT_Status cnbt_read(FILE* f, CNBT_Compound* comp);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CNBT_H_
