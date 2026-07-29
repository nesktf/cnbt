#ifndef CNBT_H_
#define CNBT_H_

#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef CNBT_INCLUDE_STDIO
#include <stdio.h>
#endif

#ifdef __cplusplus
#define CNBT_INLINE static inline
#else
#define CNBT_INLINE static
#endif

#define CNBT_TAG_SIZE        16
#define CNBT_MAX_STRING_SIZE 0xFFFF
#define CNBT_MAX_LIST_SIZE   0x7FFFFFFF

#define CNBT_SEEK_SET 0
#define CNBT_SEEK_CUR 1
#define CNBT_SEEK_END 2

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
  CNBT_EOF,
  CNBT_ZLIB_ERROR,
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
  alignas(void*) uint8_t data[CNBT_TAG_SIZE];
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

typedef size_t (*PFN_cnbt_read_func)(void* buff, size_t sz, size_t nmemb, void* src);
typedef size_t (*PFN_cnbt_write_func)(const void* buff, size_t sz, size_t nmemb, void* src);
typedef int (*PFN_cnbt_seek_func)(void* src, long offset, int origin);
typedef long (*PFN_cnbt_tell_func)(void* src);

typedef struct CNBT_IoCallbacks {
  PFN_cnbt_read_func read;
  PFN_cnbt_write_func write;
  PFN_cnbt_seek_func seek;
  PFN_cnbt_tell_func tell;
} CNBT_IoCallbacks;

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

CNBT_API CNBT_Status cnbt_read(CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs);

CNBT_API CNBT_Status cnbt_write(const CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs);
CNBT_API CNBT_Status cnbt_write_pretty(const CNBT_Tag* tag, void* src,
                                       const CNBT_IoCallbacks* cbs);

#ifdef CNBT_INCLUDE_STDIO
CNBT_INLINE CNBT_IoCallbacks CNBT_FILE_IO = {
  .read = (PFN_cnbt_read_func)fread,
  .write = (PFN_cnbt_write_func)fwrite,
  .seek = (PFN_cnbt_seek_func)fseek,
  .tell = (PFN_cnbt_tell_func)ftell,
};
#endif

#ifdef CNBT_HAS_ZLIB
typedef struct CNBT_ZStream_T* CNBT_ZStream;
CNBT_API CNBT_Status cnbt_make_zstream(CNBT_ZStream* zstr, size_t buffsz, void* src,
                                       const CNBT_IoCallbacks* cbs);
CNBT_API void cnbt_free_zstream(CNBT_ZStream zstr);

CNBT_API size_t cnbt_zread(void* buff, size_t sz, size_t nmemb, CNBT_ZStream zstr);
CNBT_API size_t cnbt_zwrite(const void* buff, size_t sz, size_t nmemb, CNBT_ZStream zstr);
CNBT_API int cnbt_zseek(CNBT_ZStream zstr, long offset, int origin);
CNBT_API long cnbt_ztell(CNBT_ZStream zstr);

CNBT_INLINE CNBT_IoCallbacks CNBT_ZSTREAM_IO = {
  .read = (PFN_cnbt_read_func)cnbt_zread,
  .write = (PFN_cnbt_write_func)cnbt_zwrite,
  .seek = (PFN_cnbt_seek_func)cnbt_zseek,
  .tell = (PFN_cnbt_tell_func)cnbt_ztell,
};
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CNBT_H_
