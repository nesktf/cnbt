#include "cnbt_internal.h"
#include "stb_ds.h"

cnbt_Status cnbt__read_type(cnbt__ReadCtx* ctx, cnbt_Type* type) {
  assert(ctx && ctx->cbs.read);
  assert(type);
  int8_t tag;
  size_t read = ctx->cbs.read(&tag, sizeof(tag), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  if (tag < CNBT_TYPE_END || tag > CNBT_TYPE_COMPOUND) {
    return CNBT_INVALID_DATA;
  }
  *type = tag;
  return CNBT_OK;
}

cnbt_Status cnbt__read_byte(cnbt__ReadCtx* ctx, i8* num) {
  assert(ctx && ctx->cbs.read);
  assert(num);

  uint8_t buff[1];
  if (!ctx->cbs.read(buff, sizeof(buff), 1, ctx->src)) {
    return CNBT_EOF;
  }
  *num = (i8)buff[0];
  return CNBT_OK;
}

cnbt_Status cnbt__read_short(cnbt__ReadCtx* ctx, i16* num) {
  assert(ctx && ctx->cbs.read);
  assert(num);

  uint8_t buff[2];
  if (!ctx->cbs.read(buff, sizeof(buff), 1, ctx->src)) {
    return CNBT_EOF;
  }
  if (ctx->endian_mode == CNBT_BIG_ENDIAN) {
    *num = (i16)buff[1] | ((i16)buff[0] << 8);
  } else {
    *num = (u16)buff[0] | ((u16)buff[1] << 8);
  }
  return CNBT_OK;
}

cnbt_Status cnbt__read_int(cnbt__ReadCtx* ctx, i32* num) {
  assert(ctx && ctx->cbs.read);
  assert(num);

  uint8_t buff[4];
  if (!ctx->cbs.read(buff, sizeof(buff), 1, ctx->src)) {
    return CNBT_EOF;
  }
  if (ctx->endian_mode == CNBT_BIG_ENDIAN) {
    *num = (i32)buff[3] | ((i32)buff[2] << 8) | ((i32)buff[1] << 16) | ((i32)buff[0] << 24);
  } else {
    *num = (i32)buff[0] | ((i32)buff[1] << 8) | ((i32)buff[2] << 16) | ((i32)buff[3] << 24);
  }
  return CNBT_OK;
}

cnbt_Status cnbt__read_long(cnbt__ReadCtx* ctx, i64* num) {
  assert(ctx && ctx->cbs.read);
  assert(num);

  uint8_t buff[8];
  if (!ctx->cbs.read(buff, sizeof(buff), 1, ctx->src)) {
    return CNBT_EOF;
  }
  if (ctx->endian_mode == CNBT_BIG_ENDIAN) {
    *num = (i64)buff[7] | ((i64)buff[6] << 8) | ((i64)buff[5] << 16) | ((i64)buff[4] << 24) |
           ((i64)buff[3] << 32) | ((i64)buff[2] << 40) | ((i64)buff[1] << 48) |
           ((i64)buff[0] << 56);
  } else {
    *num = (i64)buff[0] | ((i64)buff[1] << 8) | ((i64)buff[2] << 16) | ((i64)buff[3] << 24) |
           ((i64)buff[4] << 32) | ((i64)buff[5] << 40) | ((i64)buff[6] << 48) |
           ((i64)buff[7] << 56);
  }
  return CNBT_OK;
}

cnbt_Status cnbt__read_float(cnbt__ReadCtx* ctx, f32* num) {
  union {
    i32 temp_int;
    f32 temp_float;
  } u;

  cnbt_Status ret = cnbt__read_int(ctx, &u.temp_int);
  if (ret) {
    return ret;
  }
  *num = u.temp_float;
  return ret;
}

cnbt_Status cnbt__read_double(cnbt__ReadCtx* ctx, f64* num) {
  union {
    i64 temp_int;
    f64 temp_double;
  } u;

  cnbt_Status ret = cnbt__read_long(ctx, &u.temp_int);
  if (ret) {
    return ret;
  }
  *num = u.temp_double;
  return ret;
}

cnbt_Status cnbt__read_string(cnbt__ReadCtx* ctx, cnbt__StringData* s) {
  assert(ctx && ctx->cbs.read);
  assert(s);

  cnbt_Status ret;

  union {
    i16 read_size;
    u16 size;
  } u;

  ret = cnbt__read_short(ctx, &u.read_size);
  if (ret) {
    return ret;
  }
  char* str = CNBT_MALLOC(u.size + 1);
  size_t read = ctx->cbs.read(str, sizeof(*str), u.size, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  str[u.size] = '\0';
  s->data = str;
  s->size = (u32)u.size;
  return CNBT_OK;
}

cnbt_Status cnbt__read_blob(cnbt__ReadCtx* ctx, cnbt__ByteArrayData* d) {
  assert(ctx);
  assert(d);

  cnbt_Status ret;

  i32 sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    return ret;
  }
  int8_t* blob = CNBT_MALLOC(sz);
  if (!blob) {
    ret = CNBT_ALLOC_FAILED;
    return ret;
  }
  size_t read = ctx->cbs.read(blob, sizeof(*blob), sz, ctx->src);
  if (!read) {
    CNBT_FREE(blob);
    return CNBT_EOF;
  }
  d->data = blob;
  d->size = (u32)sz;
  return ret;
}

static cnbt_Status read_data(cnbt__ReadCtx* ctx, cnbt_Tag* data, cnbt_Type type) {
  cnbt_Status ret;
  memset(data, 0x00, sizeof(*data));
  data->type = type;
  switch (type) {
    case CNBT_TYPE_END:
      ret = CNBT_OK;
    case CNBT_TYPE_BYTE: {
      ret = cnbt__read_byte(ctx, &data->as_i8);
    } break;
    case CNBT_TYPE_SHORT: {
      ret = cnbt__read_short(ctx, &data->as_i16);
    } break;
    case CNBT_TYPE_INT: {
      ret = cnbt__read_int(ctx, &data->as_i32);
    } break;
    case CNBT_TYPE_LONG: {
      ret = cnbt__read_long(ctx, &data->as_i64);
    } break;
    case CNBT_TYPE_FLOAT: {
      ret = cnbt__read_float(ctx, &data->as_f32);
    } break;
    case CNBT_TYPE_DOUBLE: {
      ret = cnbt__read_double(ctx, &data->as_f64);
    } break;
    case CNBT_TYPE_BYTE_ARRAY: {
      ret = cnbt__read_blob(ctx, &data->as_bytearr);
    } break;
    case CNBT_TYPE_STRING: {
      ret = cnbt__read_string(ctx, &data->as_string);
    } break;
    case CNBT_TYPE_LIST: {
      ret = cnbt__read_list(ctx, &data->as_list);
    } break;
    case CNBT_TYPE_COMPOUND: {
      ret = cnbt__read_compound(ctx, &data->as_compound);
    } break;
  }
  return ret;
}

cnbt_Status cnbt__read_list(cnbt__ReadCtx* ctx, cnbt__ListData* d) {
  assert(ctx);
  assert(d);

  cnbt_Status ret;
  cnbt_Type type;
  cnbt_Tag* list = NULL;
  ret = cnbt__read_type(ctx, &type);
  if (ret) {
    goto list_cleanup;
  }

  i32 sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    goto list_cleanup;
  }
  if (sz < 0) {
    ret = CNBT_INVALID_DATA;
    goto list_cleanup;
  }

  if (type != CNBT_TYPE_END) {
    for (i32 i = 0; i < sz; ++i) {
      cnbt_Tag value;
      ret = read_data(ctx, &value, type);
      if (ret) {
        goto list_cleanup;
      }
      stbds_arrput(list, value);
    }
  }
  d->data = list;
  d->size = (u32)sz;
  return ret;

list_cleanup:
  if (list) {
    cnbt__free_list(d);
  }
  return ret;
}

cnbt_Status cnbt__read_compound(cnbt__ReadCtx* ctx, cnbt__CompoundData* d) {
  assert(ctx);
  assert(d);

  cnbt_Type type;
  cnbt_Status ret = CNBT_OK;
  cnbt_KeyTag* comp = NULL;
  while (cnbt__read_type(ctx, &type) && type != CNBT_TYPE_END) {
    cnbt_Tag value;
    cnbt__StringData key;
    ret = cnbt__read_string(ctx, &key);
    if (ret) {
      goto compound_clean;
    }
    ret = read_data(ctx, &value, type);
    if (!ret) {
      stbds_shput(comp, key.data, value);
    }
    cnbt__free_string(&key);
    if (ret) {
      goto compound_clean;
    }
  }
  d->data = comp;
  d->size = stbds_shlenu(comp);
  return ret;

compound_clean:
  if (comp) {
    cnbt__free_compound(d);
  }
  return ret;
}

CNBT_API cnbt_Status cnbt_read(cnbt_Tag* tag, cnbt_EndianMode mode, void* src,
                               const cnbt_IoFunc* cbs) {
  if (!tag || !cbs) {
    return CNBT_INVALID_DATA;
  }
  if (!cbs->read) {
    return CNBT_INVALID_DATA;
  }

  cnbt__ReadCtx ctx;
  ctx.src = src;
  ctx.cbs = *cbs;
  ctx.endian_mode = mode;
  long start_pos = -1;
  if (cbs->tell) {
    start_pos = cbs->tell(src);
  }

  cnbt_Status ret;
  cnbt_Type type;
  ret = cnbt__read_type(&ctx, &type);
  if (ret) {
    goto read_err;
  }
  ret = read_data(&ctx, tag, type);
  if (ret) {
    goto read_err;
  }
  return ret;

read_err:
  if (cbs->seek && start_pos > 0) {
    cbs->seek(src, start_pos, CNBT_SEEK_SET); // reset the stream
  }
  return ret;
}

CNBT_API cnbt_Status cnbt_write(const cnbt_Tag* tag, cnbt_EndianMode mode, void* src,
                                const cnbt_IoFunc* cbs) {
  // TODO
  return CNBT_OK;
}
