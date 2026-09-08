#include "cnbt_internal.h"

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#define Z_CHUNKSIZE 16384

typedef struct cnbt_ZStream_T {
  Bytef buff[Z_CHUNKSIZE];
  z_stream str;
  cnbt_IoFunc cbs;
  void* src;
  long offset;
  cnbt_ZIoMode mode;
  int eof;
} cnbt_ZStream_T;

static void* zalloc(void* user, unsigned n, unsigned m) {
  UNUSED(user);
  return CNBT_MALLOC(n * m);
}

static void zfree(void* user, void* p) {
  UNUSED(user);
  CNBT_FREE(p);
}

CNBT_API cnbt_Status cnbt_zopen(cnbt_ZStream* zstr, cnbt_ZIoMode mode, int zlevel, void* src,
                                const cnbt_IoFunc* cbs) {
  if (!cbs) {
    return CNBT_INVALID_DATA;
  }
  if (mode == CNBT_ZREAD && (!cbs->read || !cbs->eof)) {
    return CNBT_INVALID_DATA;
  }
  if (mode == CNBT_ZWRITE && (!cbs->write)) {
    return CNBT_INVALID_DATA;
  }

  cnbt_ZStream ret = CNBT_MALLOC(sizeof(**zstr));
  if (!ret) {
    return CNBT_ALLOC_FAILED;
  }
  ret->cbs = *cbs;
  ret->src = src;
  ret->mode = mode;
  ret->offset = 0;
  ret->eof = 0;

  ret->str.zalloc = &zalloc;
  ret->str.zfree = &zfree;
  ret->str.opaque = Z_NULL;

  if (mode == CNBT_ZREAD) {
    inflateInit2(&ret->str, zlevel);
    ret->str.avail_in = 0;
    ret->str.next_in = Z_NULL;
  } else {
    deflateInit(&ret->str, zlevel);
    ret->str.next_out = ret->buff;
    ret->str.avail_out = Z_CHUNKSIZE;
  }
  *zstr = ret;
  return CNBT_OK;
}

CNBT_API cnbt_Status cnbt_zclose(cnbt_ZStream zstr) {
  if (!zstr) {
    return CNBT_INVALID_DATA;
  }

  cnbt_Status ret = CNBT_OK;
  if (zstr->mode == CNBT_ZWRITE) {
    ret = cnbt_zflush(zstr) == -1 ? CNBT_EOF : CNBT_OK;
    deflateEnd(&zstr->str);
  } else if (zstr->mode == 1) {
    inflateEnd(&zstr->str);
  }

  CNBT_FREE(zstr);
  return ret;
}

CNBT_API size_t cnbt_zread(void* buff, size_t sz, size_t nmemb, cnbt_ZStream zstr) {
  if (!zstr || zstr->mode != CNBT_ZREAD || sz == 0 || nmemb == 0) {
    return 0;
  }
  assert(zstr->cbs.read);
  assert(zstr->cbs.eof);

  const size_t total_out = sz * nmemb;
  zstr->str.next_out = (unsigned char*)buff;
  zstr->str.avail_out = total_out;
  while (zstr->str.avail_out > 0) {
    if (zstr->str.avail_in == 0 && !zstr->eof) {
      const size_t read_len = zstr->cbs.read(zstr->buff, 1, Z_CHUNKSIZE, zstr->src);
      if (read_len > 0) {
        zstr->str.avail_in = (uInt)read_len;
        zstr->str.next_in = zstr->buff;
      }
    }

    int ret = inflate(&zstr->str, Z_NO_FLUSH);
    if (ret == Z_STREAM_END) {
      zstr->eof = 1;
      break;
    } else if (ret == Z_BUF_ERROR) {
      // Need more data
      if (zstr->cbs.eof(zstr->src)) {
        break;
      }
    } else if (ret != Z_OK) {
      break;
    }
  }

  const size_t produced = total_out - zstr->str.avail_out;
  zstr->offset += produced;
  return produced / sz;
}

CNBT_API size_t cnbt_zwrite(const void* buff, size_t sz, size_t nmemb, cnbt_ZStream zstr) {
  if (!zstr || zstr->mode != CNBT_ZWRITE || sz == 0 || nmemb == 0) {
    return 0;
  }
  assert(zstr->cbs.write);

  const size_t total_in = sz * nmemb;
  zstr->str.next_in = (unsigned char*)buff;
  zstr->str.avail_in = (uInt)total_in;
  while (zstr->str.avail_in > 0) {
    if (zstr->str.avail_out == 0) {
      if (zstr->cbs.write(zstr->buff, 1, Z_CHUNKSIZE, zstr->src) != Z_CHUNKSIZE) {
        break;
      }
      zstr->str.next_out = zstr->buff;
      zstr->str.avail_out = Z_CHUNKSIZE;
    }

    int ret = deflate(&zstr->str, Z_NO_FLUSH);
    if (ret == Z_STREAM_ERROR) {
      break;
    }
  }

  const size_t consumed = total_in - zstr->str.avail_in;
  zstr->offset += consumed;
  return consumed / sz;
}

CNBT_API int cnbt_zseek(cnbt_ZStream zstr, long offset, int origin) {
  // Can't seek while writting, or setting SEEK_END without uncompressing everything
  if (!zstr || zstr->mode != CNBT_ZREAD || origin == CNBT_SEEK_END || !zstr->cbs.seek) {
    return -1;
  }

  if (origin == SEEK_CUR) {
    offset += zstr->offset;
  }

  if (offset < zstr->offset) {
    rewind(zstr->src);
    zstr->cbs.seek(zstr->src, 0, CNBT_SEEK_SET);
    inflateReset(&zstr->str);
    zstr->str.avail_in = 0;
    zstr->str.next_in = Z_NULL;
    zstr->offset = 0;
    zstr->eof = 0;
  }

  char dummy[4096];
  while (zstr->offset < offset) {
    const long diff = offset - zstr->offset;
    const size_t to_read = diff > (long)sizeof(dummy) ? sizeof(dummy) : (size_t)diff;
    if (!cnbt_zread(dummy, 1, to_read, zstr)) {
      return -1; // eof
    }
  }

  return 0;
}

CNBT_API long cnbt_ztell(cnbt_ZStream zstr) {
  return zstr ? zstr->offset : -1;
}

CNBT_API int cnbt_zflush(cnbt_ZStream zstr) {
  int zres;
  int ret = 0;

  zstr->str.next_in = Z_NULL;
  zstr->str.avail_in = 0;
  do {
    zres = deflate(&zstr->str, Z_FINISH);
    const size_t avail = Z_CHUNKSIZE - zstr->str.avail_out;
    if (avail > 0) {
      if (zstr->cbs.write(zstr->buff, 1, avail, zstr->src) != avail) {
        ret = -1;
      }
    }
    zstr->str.next_out = zstr->buff;
    zstr->str.avail_out = Z_CHUNKSIZE;
  } while (zres != Z_STREAM_END && ret == 0);

  return ret;
}
