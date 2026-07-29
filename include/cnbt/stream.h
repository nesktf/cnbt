#ifndef CNBT_STREAM_H_
#define CNBT_STREAM_H_

#include <cnbt/cnbt.h>

#define CNBT_SEEK_SET 0
#define CNBT_SEEK_CUR 1
#define CNBT_SEEK_END 2

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

#ifdef CNBT_HAS_ZLIB
typedef struct CNBT_ZStream_T* CNBT_ZStream;
CNBT_API CNBT_Status cnbt_make_zstream(CNBT_ZStream* zstr, size_t buffsz, void* src,
                                       const CNBT_IoCallbacks* cbs);
CNBT_API void cnbt_free_zstream(CNBT_ZStream zstr);

CNBT_API size_t cnbt_zread(void* buff, size_t sz, size_t nmemb, CNBT_ZStream zstr);
CNBT_API size_t cnbt_zwrite(const void* buff, size_t sz, size_t nmemb, CNBT_ZStream zstr);
CNBT_API int cnbt_zseek(CNBT_ZStream zstr, long offset, int origin);
CNBT_API long cnbt_ztell(CNBT_ZStream zstr);
#endif

CNBT_API CNBT_Status cnbt_read(CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs);

CNBT_API CNBT_Status cnbt_write(const CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs);
CNBT_API CNBT_Status cnbt_write_pretty(const CNBT_Tag* tag, void* src,
                                       const CNBT_IoCallbacks* cbs);

#endif // CNBT_STREAM_H_
