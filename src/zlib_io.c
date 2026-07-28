#include "core.h"
#include <stdio.h>
#include <zlib.h>

#define ZINBUF(_ctx) ((_ctx)->buff + 0)
#define ZOUTBUF(_ctx) ((_ctx)->buff + (_ctx)->buff_sz)

typedef struct CNBT_ZlibCtx_T {
  Bytef* buff;
  size_t buff_sz;
  size_t avail;
  z_stream stream;
  CNBT_IoCallbacks cbs;
  void* src;
} CNBT_ZlibCtx_T;

#define WINDOW_BITS 48

CNBT_Status cnbt_zlib_open(CNBT_ZlibCtx* ctx_, void* src, const CNBT_IoCallbacks* cbs) {
  if (!ctx_) {
    return CNBT_INVALID_DATA;
  }
  CNBT_ZlibCtx ctx = CNBT__MALLOC(sizeof(**ctx_));
  if (!ctx) {
    return CNBT_ALLOC_FAILED;
  }
  memset(ctx, 0x00, sizeof(*ctx));
  ctx->cbs = *cbs;
  ctx->src = src;
  ctx->buff = malloc(32768*2);
  ctx->buff_sz = 32768;

  int res = inflateInit2(&ctx->stream, WINDOW_BITS);
  if (res != Z_OK) {
    CNBT__FREE(ctx);
    return CNBT_ZLIB_ERROR;
  }
  *ctx_ = ctx;

  return CNBT_OK;
}

void cnbt_zlib_close(CNBT_ZlibCtx ctx) {
  if (!ctx) {
    return;
  }
  inflateEnd(&ctx->stream);
}

static int do_zlib_thing(CNBT_ZlibCtx ctx) {
  CNBT__ASSERT(ctx);
  size_t avail;
  do {
    if (ctx->stream.avail_in <= 0) {
      size_t read = ctx->cbs.read(ZINBUF(ctx), ctx->buff_sz, sizeof(*ZINBUF(ctx)), ctx->src);
      if (read == 0) {
        fprintf(stderr, "zlib end of stream\n");
        return CNBT_EOF;
      }
      ctx->stream.next_in = ZINBUF(ctx);
      ctx->stream.avail_in = read;
    }
    ctx->stream.next_out = ZOUTBUF(ctx);
    ctx->stream.avail_out = ctx->buff_sz;

    int res = inflate(&ctx->stream, Z_NO_FLUSH);
    avail = ctx->buff_sz - ctx->stream.avail_out;
    switch (res) {
      case Z_STREAM_END:
        ctx->cbs.seek(ctx->src, -ctx->stream.avail_in, SEEK_CUR);
        break;
      case Z_MEM_ERROR:
        fprintf(stderr, "zlib out of memory\n");
        return CNBT_ALLOC_FAILED;
      case Z_NEED_DICT:
      case Z_DATA_ERROR:
        fprintf(stderr, "zlib error: %s\n", zError(res));
        return CNBT_ZLIB_ERROR;
    }
  } while (avail == 0);
  ctx->avail = avail;
  return 0;
}

size_t cnbt_zlib_read(void* buff, size_t sz, size_t nmemb, CNBT_ZlibCtx ctx) {
  size_t wants = sz*nmemb;
  if (wants > ctx->avail) {
    memcpy(buff, ZOUTBUF(ctx), ctx->avail*sizeof(*ZOUTBUF(ctx)));
  }
}

size_t cnbt_zlib_write(const void* buff, size_t sz, size_t nmemb, CNBT_ZlibCtx ctx) {

}
