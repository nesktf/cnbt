#ifndef CNBT_H
#define CNBT_H

#include <stdint.h>
#include <stdlib.h>

#if defined CNBT_STATIC
#define CNBT_API
#elif defined _WIN32 || defined __CYGWIN__
#if defined CNBT__INTERNAL
#define CNBT_API __declspec(dllexport)
#else
#define CNBT_API __declspec(dllimport)
#endif
#elif defined CNBT__INTERNAL && defined __GNUC__
#define CNBT_API __attribute__((visibility("default")))
#else
#define CNBT_API
#endif

#define CNBT_MAX_STRING_SIZE 0xFFFF
#define CNBT_MAX_LIST_SIZE   0x7FFFFFFF

#define CNBT_SEEK_SET 0
#define CNBT_SEEK_CUR 1
#define CNBT_SEEK_END 2

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cnbt_Status {
  CNBT_OK = 0,
  CNBT_INVALID_DATA,
  CNBT_ALLOC_FAILED,
  CNBT_EOF,
  CNBT_ZLIB_ERROR,
} cnbt_Status;

typedef enum cnbt_Type {
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
} cnbt_Type;

typedef enum cnbt_EndianMode {
  CNBT_BIG_ENDIAN = 0,
  CNBT_LITTLE_ENDIAN,
} cnbt_EndianMode;

#ifdef CNBT__INTERNAL
typedef struct cnbt_Tag cnbt_Tag;
struct cnbt_Tag_pub {
#else
typedef struct cnbt_Tag {
#endif
  void* _private[3];
#ifdef CNBT__INTERNAL
};
#else
} cnbt_Tag;
#endif

typedef struct cnbt_KeyTag {
  char* key;
  cnbt_Tag value;
} cnbt_KeyTag;

typedef size_t (*PFN_cnbt_read_func)(void* buff, size_t sz, size_t nmemb, void* src);
typedef size_t (*PFN_cnbt_write_func)(const void* buff, size_t sz, size_t nmemb, void* src);
typedef int (*PFN_cnbt_seek_func)(void* src, long offset, int origin);
typedef long (*PFN_cnbt_tell_func)(void* src);

typedef struct cnbt_IoFunc {
  PFN_cnbt_read_func read;
  PFN_cnbt_write_func write;
  PFN_cnbt_seek_func seek;
  PFN_cnbt_tell_func tell;
} cnbt_IoFunc;

typedef cnbt_Tag cnbt_End;
typedef cnbt_Tag cnbt_Byte;
typedef cnbt_Tag cnbt_Short;
typedef cnbt_Tag cnbt_Int;
typedef cnbt_Tag cnbt_Long;
typedef cnbt_Tag cnbt_Float;
typedef cnbt_Tag cnbt_Double;
typedef cnbt_Tag cnbt_String;
typedef cnbt_Tag cnbt_ByteArray;
typedef cnbt_Tag cnbt_List;
typedef cnbt_Tag cnbt_Compound;

typedef struct cnbt_ZStream_T* cnbt_ZStream;

CNBT_API const char* cnbt_tag_name(cnbt_Type type);
CNBT_API void cnbt_free(cnbt_Tag* tag);
CNBT_API cnbt_Type cnbt_get_type(const cnbt_Tag* tag);

CNBT_API void cnbt_make_end(cnbt_End* tag);

CNBT_API void cnbt_make_byte(cnbt_Byte* tag, int8_t value);
CNBT_API int8_t cnbt_get_byte(const cnbt_Byte* tag);

CNBT_API void cnbt_make_short(cnbt_Short* tag, int16_t value);
CNBT_API int16_t cnbt_get_short(const cnbt_Short* tag);

CNBT_API void cnbt_make_int(cnbt_Int* tag, int32_t value);
CNBT_API int32_t cnbt_get_int(const cnbt_Int* tag);

CNBT_API void cnbt_make_long(cnbt_Long* tag, int64_t value);
CNBT_API int64_t cnbt_get_long(const cnbt_Long* tag);

CNBT_API void cnbt_make_float(cnbt_Float* tag, float value);
CNBT_API float cnbt_get_float(const cnbt_Float* tag);

CNBT_API void cnbt_make_double(cnbt_Double* tag, double value);
CNBT_API double cnbt_get_double(const cnbt_Double* tag);

CNBT_API cnbt_Status cnbt_make_str(cnbt_String* str, const char* value);
CNBT_API cnbt_Status cnbt_make_strn(cnbt_String* str, const char* value, size_t n);
CNBT_API size_t cnbt_str_len(const cnbt_String* str);
CNBT_API char* cnbt_str_data(const cnbt_String* str);

CNBT_API cnbt_Status cnbt_make_byte_array(cnbt_ByteArray* arr, const void* data, size_t n);
CNBT_API size_t cnbt_byte_array_len(const cnbt_ByteArray* arr);
CNBT_API int8_t* cnbt_byte_array_data(const cnbt_ByteArray* arr);

CNBT_API cnbt_Status cnbt_make_list(cnbt_List* list);
CNBT_API cnbt_Status cnbt_make_list_fill(cnbt_List* list, cnbt_Tag* data, size_t n);
CNBT_API cnbt_Tag* cnbt_list_push(cnbt_List* list, cnbt_Tag tag);
CNBT_API size_t cnbt_list_len(const cnbt_List* list);
CNBT_API cnbt_Tag* cnbt_list_get(const cnbt_List* list, size_t pos);
CNBT_API cnbt_Tag* cnbt_list_get_unchecked(const cnbt_List* list, size_t pos);

CNBT_API cnbt_Status cnbt_make_compound(cnbt_Compound* comp);
CNBT_API cnbt_KeyTag* cnbt_comp_insert(cnbt_Compound* comp, const char* key, cnbt_Tag tag);
CNBT_API size_t cnbt_comp_len(const cnbt_Compound* comp);
CNBT_API cnbt_KeyTag* cnbt_comp_get(const cnbt_Compound* comp, const char* key);

CNBT_API cnbt_Status cnbt_read(cnbt_Tag* tag, cnbt_EndianMode mode, void* src,
                               const cnbt_IoFunc* func);
CNBT_API cnbt_Status cnbt_write(const cnbt_Tag* tag, cnbt_EndianMode mode, void* src,
                                const cnbt_IoFunc* func);
CNBT_API cnbt_Status cnbt_write_pretty(const cnbt_Tag* tag, void* src, const cnbt_IoFunc* func);

CNBT_API cnbt_Status cnbt_make_zstream(cnbt_ZStream* zstr, size_t buffsz, void* src,
                                       const cnbt_IoFunc* func);
CNBT_API void cnbt_free_zstream(cnbt_ZStream zstr);
CNBT_API void cnbt_load_zstream_funcs(cnbt_ZStream zstr, cnbt_IoFunc* func);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CNBT_H
