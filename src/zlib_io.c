#include "cnbt_internal.h"

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#define ZWINDOWBITS 47

#define ZMINBUFFSZ 512

typedef struct cnbt_ZStream_T {
  z_stream stream;
  cnbt_IoFunc cbs;
  void* src;
  Bytef* buff;
  size_t buffsz;
} cnbt_ZStream_T;

static void* zalloc(void* user, unsigned n, unsigned m) {
  UNUSED(user);
  return CNBT_MALLOC(n * m);
}

static void zfree(void* user, void* p) {
  UNUSED(user);
  CNBT_FREE(p);
}

CNBT_API cnbt_Status cnbt_make_zstream(cnbt_ZStream* zstr_, size_t buffsz, void* src,
                                       const cnbt_IoFunc* cbs) {
  cnbt_ZStream zstr = NULL;
  cnbt_Status ret = CNBT_OK;
  if (!zstr_ || !cbs) {
    ret = CNBT_INVALID_DATA;
    goto make_zstr_clean;
  }
  if (!cbs->write || !cbs->read) {
    ret = CNBT_INVALID_DATA;
    goto make_zstr_clean;
  }

  zstr = CNBT_MALLOC(sizeof(*zstr));
  if (!zstr) {
    ret = CNBT_ALLOC_FAILED;
    goto make_zstr_clean;
  }

  memset(zstr, 0x00, sizeof(*zstr));
  zstr->cbs = *cbs;
  zstr->src = src;
  zstr->buffsz = buffsz < ZMINBUFFSZ ? ZMINBUFFSZ : buffsz;
  zstr->buff = CNBT_MALLOC(2 * zstr->buffsz); // half for input, half for output
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
      CNBT_FREE(zstr->buff);
    }
    CNBT_FREE(zstr);
  }
  return ret;
}

CNBT_API void cnbt_free_zstream(cnbt_ZStream zstr) {
  if (!zstr) {
    return;
  }
  inflateEnd(&zstr->stream);
  CNBT_FREE(zstr->buff);
}

static size_t zstream_read(void* buff, size_t sz, size_t nmemb, cnbt_ZStream zstr) {
  assert(zstr && "Invalid zstream");
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

static size_t zstream_write(const void* buff, size_t sz, size_t nmemb, cnbt_ZStream zstr) {
  // TODO
  return 0;
}

static int zstream_seek(cnbt_ZStream zstr, long offset, int origin) {
  // TODO
  return 0;
}

static long zstream_tell(cnbt_ZStream zstr) {
  // TODO
  return 0;
}

CNBT_API void cnbt_load_zstream_funcs(cnbt_ZStream zstr, cnbt_IoFunc* cbs) {
  if (!zstr || !cbs) {
    return;
  }
  cbs->write = (PFN_cnbt_write_func)&zstream_write;
  cbs->read = (PFN_cnbt_read_func)&zstream_read;
  cbs->seek = zstr->cbs.seek ? (PFN_cnbt_seek_func)&zstream_seek : NULL;
  cbs->tell = zstr->cbs.tell ? (PFN_cnbt_tell_func)&zstream_tell : NULL;
}
