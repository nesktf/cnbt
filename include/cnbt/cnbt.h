#ifndef CNBT_H_
#define CNBT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CNBT_TAG_SIZE 24

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
  CNBT_TYPE_BLOB,
  CNBT_TYPE_STRING,
  CNBT_TYPE_LIST,
  CNBT_TYPE_COMPOUND,
} CNBT_Type;

typedef struct CNBT_Tag {
  uint8_t data[CNBT_TAG_SIZE];
} CNBT_Tag;

typedef struct CNBT_CompEntry {
  char* key;
  CNBT_Tag value;
} CNBT_CompEntry;

typedef CNBT_Tag CNBT_Byte;
typedef CNBT_Tag CNBT_Short;
typedef CNBT_Tag CNBT_Int;
typedef CNBT_Tag CNBT_Long;
typedef CNBT_Tag CNBT_Float;
typedef CNBT_Tag CNBT_Double;
typedef CNBT_Tag CNBT_String;
typedef CNBT_Tag CNBT_List;
typedef CNBT_Tag CNBT_Blob;
typedef CNBT_Tag CNBT_Compound;

CNBT_API const char* cnbt_tag_name(CNBT_Type type);

CNBT_API void cnbt_make_end(CNBT_Tag* tag);
CNBT_API void cnbt_destroy(CNBT_Tag* tag);
CNBT_API CNBT_Type cnbt_get_type(CNBT_Tag* tag);
CNBT_API char* cnbt_get_name(CNBT_Tag* tag);

CNBT_API CNBT_Status cnbt_make_byte(CNBT_Byte* tag, int8_t value, const char* name);
CNBT_API int8_t cnbt_get_byte(CNBT_Byte* tag);

CNBT_API CNBT_Status cnbt_make_short(CNBT_Short* tag, int16_t value, const char* name);
CNBT_API int16_t cnbt_get_short(CNBT_Short* tag);

CNBT_API CNBT_Status cnbt_make_int(CNBT_Int* tag, int32_t value, const char* name);
CNBT_API int32_t cnbt_get_int(CNBT_Int* tag);

CNBT_API CNBT_Status cnbt_make_long(CNBT_Long* tag, int64_t value, const char* name);
CNBT_API int64_t cnbt_get_long(CNBT_Long* tag);

CNBT_API CNBT_Status cnbt_make_float(CNBT_Float* tag, float value, const char* name);
CNBT_API float cnbt_get_float(CNBT_Float* tag);

CNBT_API CNBT_Status cnbt_make_double(CNBT_Double* tag, double value, const char* name);
CNBT_API double cnbt_get_double(CNBT_Double* tag);

CNBT_API CNBT_Status cnbt_make_str(CNBT_String* tag, const char* data, const char* name);
CNBT_API size_t cnbt_strlen(CNBT_String* tag);
CNBT_API char* cnbt_get_string(CNBT_String* tag);

CNBT_API CNBT_Status cnbt_make_blob(CNBT_Blob* blob, const void* data, size_t size,
                                    const char* name);
CNBT_API size_t cnbt_blob_size(CNBT_Blob* blob);
CNBT_API void* cnbt_get_blob(CNBT_Blob* blob);

CNBT_API CNBT_Status cnbt_make_list(CNBT_List* list, const char* name);
CNBT_API CNBT_Tag* cnbt_list_put_tag(CNBT_List* list, CNBT_Tag tag);
CNBT_API size_t cnbt_list_size(CNBT_List* list);
CNBT_API CNBT_Tag* cnbt_list_get(CNBT_List* list, size_t pos);
CNBT_API CNBT_Tag* cnbt_list_get_unchecked(CNBT_List* list, size_t pos);

CNBT_API CNBT_Status cnbt_make_compound(CNBT_Compound* comp, const char* name);
CNBT_API CNBT_CompEntry* cnbt_comp_put_tag(CNBT_Compound* comp, const char* key, CNBT_Tag tag);
CNBT_API CNBT_CompEntry* cnbt_comp_get(CNBT_Compound* comp, const char* key);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CNBT_H_
