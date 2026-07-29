#include "core.h"
#include <stdio.h>
#include <zlib.h>

#define ZWINDOWBITS 47

#define ZMINBUFFSZ 512

typedef struct CNBT_ZStream_T {
  z_stream stream;
  CNBT_IoCallbacks cbs;
  void* src;
  Bytef* buff;
  size_t buffsz;
} CNBT_ZStream_T;

static void* zalloc(void* q, unsigned n, unsigned m) {
  (void)q;
  return CNBT__MALLOC(n * m);
}

static void zfree(void* q, void* p) {
  (void)q;
  CNBT__FREE(p);
}

CNBT_API CNBT_Status cnbt_make_zstream(CNBT_ZStream* zstr_, size_t buffsz, void* src,
                                       const CNBT_IoCallbacks* cbs) {
  CNBT_ZStream zstr = NULL;
  CNBT_Status ret = CNBT_OK;
  if (!zstr_ || !cbs) {
    ret = CNBT_INVALID_DATA;
    goto make_zstr_clean;
  }
  if (!cbs->write || !cbs->read) {
    ret = CNBT_INVALID_DATA;
    goto make_zstr_clean;
  }

  zstr = CNBT__MALLOC(sizeof(*zstr));
  if (!zstr) {
    ret = CNBT_ALLOC_FAILED;
    goto make_zstr_clean;
  }

  memset(zstr, 0x00, sizeof(*zstr));
  zstr->cbs = *cbs;
  zstr->src = src;
  zstr->buffsz = buffsz < ZMINBUFFSZ ? ZMINBUFFSZ : buffsz;
  zstr->buff = CNBT__MALLOC(2 * zstr->buffsz); // half for input, half for output
  if (!zstr->buff) {
    ret = CNBT_ALLOC_FAILED;
    goto make_zstr_clean;
  }

  zstr->stream.zalloc = &zalloc;
  zstr->stream.zfree = &zfree;
  zstr->stream.opaque = NULL;
  int zret = inflateInit2(&zstr->stream, ZWINDOWBITS);
  if (zret != Z_OK) {
    fprintf(stderr, "zlib error: %s", zError(zret));
    ret = CNBT_ZLIB_ERROR;
    goto make_zstr_clean;
  }

  *zstr_ = zstr;

make_zstr_clean:
  if (ret && zstr) {
    if (zstr->buff) {
      CNBT__FREE(zstr->buff);
    }
    CNBT__FREE(zstr);
  }
  return ret;
}

CNBT_API void cnbt_free_zstream(CNBT_ZStream zstr) {
  if (!zstr) {
    return;
  }
  inflateEnd(&zstr->stream);
  CNBT__FREE(zstr->buff);
}

CNBT_API size_t cnbt_zread(void* buff, size_t sz, size_t nmemb, CNBT_ZStream zstr) {
  CNBT__ASSERT(zstr && "Invalid zstream");
  if (zstr->stream.avail_in == 0) {
    size_t count = zstr->cbs.read(zstr->buff, sizeof(*zstr->buff), zstr->buffsz, zstr->src);
    zstr->stream.avail_in = count;
    zstr->stream.next_in = zstr->buff;
  }
  zstr->stream.next_out = zstr->buff + zstr->buffsz;
  zstr->stream.avail_out = zstr->buffsz;

  int ret = inflate(&zstr->stream, Z_NO_FLUSH);
  int read = zstr->buffsz - zstr->stream.avail_out;
  if (ret == Z_OK || ret == Z_STREAM_END) {
    return read;
  }
  fprintf(stderr, "zlib error: %s\n", zError(ret));
  return 0;
}

CNBT_API size_t cnbt_zwrite(const void* buff, size_t sz, size_t nmemb, CNBT_ZStream zstr) {
  // TODO
  return 0;
}

CNBT_API int cnbt_zseek(CNBT_ZStream zstr, long offset, int origin) {
  // TODO
  return 0;
}

CNBT_API long cnbt_ztell(CNBT_ZStream zstr) {
  // TODO
  return 0;
}
