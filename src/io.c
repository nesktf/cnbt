#include "core.h"
#include "stb_ds.h"

CNBT_Status cnbt__read_type(CNBT__ReadCtx* ctx, CNBT_Type* type) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(type);
  int8_t tag;
  size_t read = ctx->cbs->read(&tag, sizeof(tag), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  if (tag < CNBT_TYPE_END || tag > CNBT_TYPE_COMPOUND) {
    return CNBT_INVALID_DATA;
  }
  *type = tag;
  return CNBT_OK;
}

CNBT_Status cnbt__read_byte(CNBT__ReadCtx* ctx, int8_t* num) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(num);
  uint8_t buff[1];
  size_t read = ctx->cbs->read(buff, sizeof(buff), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  memcpy(num, buff, sizeof(buff)); // TODO: check endianess
  return CNBT_OK;
}

CNBT_Status cnbt__read_short(CNBT__ReadCtx* ctx, int16_t* num) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(num);
  uint8_t buff[2];
  size_t read = ctx->cbs->read(buff, sizeof(buff), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  memcpy(num, buff, sizeof(buff)); // TODO: check endianess
  return CNBT_OK;
}

CNBT_Status cnbt__read_int(CNBT__ReadCtx* ctx, int32_t* num) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(num);
  uint8_t buff[4];
  size_t read = ctx->cbs->read(buff, sizeof(buff), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  memcpy(num, buff, sizeof(buff)); // TODO: check endianess
  return CNBT_OK;
}

CNBT_Status cnbt__read_long(CNBT__ReadCtx* ctx, int64_t* num) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(num);
  uint8_t buff[8];
  size_t read = ctx->cbs->read(buff, sizeof(buff), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  memcpy(num, buff, sizeof(buff)); // TODO: check endianess
  return CNBT_OK;
}

CNBT_Status cnbt__read_float(CNBT__ReadCtx* ctx, float* num) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(num);
  uint8_t buff[4];
  size_t read = ctx->cbs->read(buff, sizeof(buff), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  memcpy(num, buff, sizeof(buff)); // TODO: check endianess
  return CNBT_OK;
}

CNBT_Status cnbt__read_double(CNBT__ReadCtx* ctx, double* num) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(num);
  uint8_t buff[8];
  size_t read = ctx->cbs->read(buff, sizeof(buff), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  memcpy(num, buff, sizeof(buff)); // TODO: check endianess
  return CNBT_OK;
}

CNBT_Status cnbt__read_string(CNBT__ReadCtx* ctx, char** data, uint32_t* len) {
  CNBT__ASSERT(ctx && ctx->cbs && ctx->cbs->read);
  CNBT__ASSERT(data);

  CNBT_Status ret;
  uint16_t sz;
  size_t read = ctx->cbs->read(&sz, sizeof(sz), 1, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  char* str = CNBT__MALLOC(sz + 1);
  read = ctx->cbs->read(str, sizeof(*str), sz, ctx->src);
  if (!read) {
    return CNBT_EOF;
  }
  str[sz] = '\0';
  *data = str;
  if (len) {
    *len = (uint32_t)sz;
  }
  return CNBT_OK;
}

CNBT_Status cnbt__read_blob(CNBT__ReadCtx* ctx, int8_t** data, uint32_t* len) {
  CNBT__ASSERT(ctx);
  CNBT__ASSERT(data);

  CNBT_Status ret;
  int32_t sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    return ret;
  }
  int8_t* blob = CNBT__MALLOC(sz);
  if (!blob) {
    ret = CNBT_ALLOC_FAILED;
    return ret;
  }
  *data = blob;
  *len = (uint32_t)sz;
  return ret;
}

static CNBT_Status read_data(CNBT__ReadCtx* ctx, CNBT_Tag* data, CNBT_Type type) {
  CNBT_Status ret;
  memset(data, 0x00, sizeof(*data));
  CNBT__GET_DATA(data)->tag = type;
  switch (type) {
    case CNBT_TYPE_END:
      ret = CNBT_OK;
    case CNBT_TYPE_BYTE: {
      int8_t num;
      ret = cnbt__read_byte(ctx, &num);
      if (!ret) {
        CNBT__GET_DATA(data)->as_int = num;
      }
    } break;
    case CNBT_TYPE_SHORT: {
      int16_t num;
      ret = cnbt__read_short(ctx, &num);
      if (!ret) {
        CNBT__GET_DATA(data)->as_int = num;
      }
    } break;
    case CNBT_TYPE_INT: {
      int32_t num;
      ret = cnbt__read_int(ctx, &num);
      if (!ret) {
        CNBT__GET_DATA(data)->as_int = num;
      }
    } break;
    case CNBT_TYPE_LONG: {
      int64_t num;
      ret = cnbt__read_long(ctx, &num);
      if (!ret) {
        CNBT__GET_DATA(data)->as_int = num;
      }
    } break;
    case CNBT_TYPE_FLOAT: {
      ret = cnbt__read_float(ctx, &CNBT__GET_DATA(data)->as_float);
    } break;
    case CNBT_TYPE_DOUBLE: {
      ret = cnbt__read_double(ctx, &CNBT__GET_DATA(data)->as_double);
    } break;
    case CNBT_TYPE_BYTE_ARRAY: {
      ret = cnbt__read_blob(ctx, &CNBT__GET_DATA(data)->as_blob, &CNBT__GET_DATA(data)->size);
    } break;
    case CNBT_TYPE_STRING: {
      ret = cnbt__read_string(ctx, &CNBT__GET_DATA(data)->as_str, &CNBT__GET_DATA(data)->size);
    } break;
    case CNBT_TYPE_LIST: {
      ret = cnbt__read_list(ctx, &CNBT__GET_DATA(data)->as_list, &CNBT__GET_DATA(data)->size);
    } break;
    case CNBT_TYPE_COMPOUND: {
      ret =
        cnbt__read_compound(ctx, &CNBT__GET_DATA(data)->as_compound, &CNBT__GET_DATA(data)->size);
    } break;
  }
  return ret;
}

CNBT_Status cnbt__read_list(CNBT__ReadCtx* ctx, CNBT_Tag** data, uint32_t* len) {
  CNBT__ASSERT(ctx);
  CNBT__ASSERT(data);

  CNBT_Status ret;
  CNBT_Type type;
  CNBT_Tag* list = NULL;
  ret = cnbt__read_type(ctx, &type);
  if (ret) {
    goto list_cleanup;
  }

  int32_t sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    goto list_cleanup;
  }
  if (len < 0) {
    ret = CNBT_INVALID_DATA;
    goto list_cleanup;
  }

  if (type != CNBT_TYPE_END) {
    for (int32_t i = 0; i < sz; ++i) {
      CNBT_Tag value;
      ret = read_data(ctx, &value, type);
      if (ret) {
        goto list_cleanup;
      }
      stbds_arrput(list, value);
    }
  }
  *data = list;
  *len = (uint32_t)sz;
  return ret;

list_cleanup:
  if (list) {
    cnbt__free_list(list);
  }
  return ret;
}

CNBT_Status cnbt__read_compound(CNBT__ReadCtx* ctx, CNBT_KeyTag** data, uint32_t* len) {
  CNBT__ASSERT(ctx);
  CNBT__ASSERT(data);

  CNBT_Type type;
  CNBT_Status ret = CNBT_OK;
  CNBT_KeyTag* comp = NULL;
  while (cnbt__read_type(ctx, &type) && type != CNBT_TYPE_END) {
    char* key;
    CNBT_Tag value;

    ret = cnbt__read_string(ctx, &key, NULL);
    if (ret) {
      goto compound_clean;
    }
    ret = read_data(ctx, &value, type);
    if (!ret) {
      stbds_shput(comp, key, value);
    }
    CNBT__FREE(key);
    if (ret) {
      goto compound_clean;
    }
  }
  *data = comp;
  *len = stbds_shlenu(comp);
  return ret;

compound_clean:
  if (comp) {
    cnbt__free_compound(CNBT__GET_DATA(data)->as_compound);
  }
  return ret;
}

CNBT_API CNBT_Status cnbt_read(CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs) {
  if (!tag || !cbs) {
    return CNBT_INVALID_DATA;
  }
  if (!cbs->read) {
    return CNBT_INVALID_DATA;
  }

  CNBT__ReadCtx ctx;
  ctx.src = src;
  ctx.cbs = cbs;
  long start_pos = -1;
  if (cbs->tell) {
    start_pos = cbs->tell(src);
  }

  CNBT_Status ret;
  CNBT_Type type;
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

CNBT_API CNBT_Status cnbt_write(const CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs) {
  // TODO
  return CNBT_OK;
}
