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
  if (tag < CNBT_TYPE_END || tag > CNBT_TYPE_LONG_ARRAY) {
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
  if (!str) {
    return CNBT_ALLOC_FAILED;
  }
  if (u.size > 0) {
    size_t read = ctx->cbs.read(str, sizeof(*str), u.size, ctx->src);
    if (read != u.size) {
      CNBT_FREE(str);
      return CNBT_EOF;
    }
  }
  str[u.size] = '\0';
  s->data = str;
  s->size = (u32)u.size;
  return CNBT_OK;
}

cnbt_Status cnbt__read_byte_array(cnbt__ReadCtx* ctx, cnbt__ByteArrayData* d) {
  assert(ctx);
  assert(d);

  cnbt_Status ret;

  i32 sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    return ret;
  }
  if (sz < 0) {
    return CNBT_INVALID_DATA;
  }
  int8_t* blob = NULL;
  if (sz > 0) {
    blob = CNBT_MALLOC(sz);
    if (!blob) {
      return CNBT_ALLOC_FAILED;
    }
    size_t read = ctx->cbs.read(blob, sizeof(*blob), sz, ctx->src);
    if (read != (size_t)sz) {
      CNBT_FREE(blob);
      return CNBT_EOF;
    }
  }
  d->data = blob;
  d->size = (u32)sz;
  return CNBT_OK;
}

cnbt_Status cnbt__read_int_array(cnbt__ReadCtx* ctx, cnbt__IntArrayData* d) {
  assert(ctx);
  assert(d);

  cnbt_Status ret;
  i32 sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    return ret;
  }
  if (sz < 0) {
    return CNBT_INVALID_DATA;
  }
  i32* blob = NULL;
  if (sz > 0) {
    blob = CNBT_MALLOC(sz * sizeof(i32));
    if (!blob) {
      return CNBT_ALLOC_FAILED;
    }
    for (i32 i = 0; i < sz; ++i) {
      ret = cnbt__read_int(ctx, &blob[i]);
      if (ret) {
        CNBT_FREE(blob);
        return ret;
      }
    }
  }
  d->data = blob;
  d->size = (u32)sz;
  return CNBT_OK;
}

cnbt_Status cnbt__read_long_array(cnbt__ReadCtx* ctx, cnbt__LongArrayData* d) {
  assert(ctx);
  assert(d);

  cnbt_Status ret;
  i32 sz;
  ret = cnbt__read_int(ctx, &sz);
  if (ret) {
    return ret;
  }
  if (sz < 0) {
    return CNBT_INVALID_DATA;
  }
  i64* blob = NULL;
  if (sz > 0) {
    blob = CNBT_MALLOC(sz * sizeof(i64));
    if (!blob) {
      return CNBT_ALLOC_FAILED;
    }
    for (i32 i = 0; i < sz; ++i) {
      ret = cnbt__read_long(ctx, &blob[i]);
      if (ret) {
        CNBT_FREE(blob);
        return ret;
      }
    }
  }
  d->data = blob;
  d->size = (u32)sz;
  return CNBT_OK;
}

static cnbt_Status read_data(cnbt__ReadCtx* ctx, cnbt_Tag* data, cnbt_Type type) {
  cnbt_Status ret = CNBT_OK;
  memset(data, 0x00, sizeof(*data));
  data->type = type;
  switch (type) {
    case CNBT_TYPE_END: {
      ret = CNBT_OK;
    } break;
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
      ret = cnbt__read_byte_array(ctx, &data->as_bytearr);
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
    case CNBT_TYPE_INT_ARRAY: {
      ret = cnbt__read_int_array(ctx, &data->as_intarr);
    } break;
    case CNBT_TYPE_LONG_ARRAY: {
      ret = cnbt__read_long_array(ctx, &data->as_longarr);
    } break;
    default: {
      ret = CNBT_INVALID_DATA;
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
  stbds_sh_new_strdup(comp);
  while ((ret = cnbt__read_type(ctx, &type)) == CNBT_OK && type != CNBT_TYPE_END) {
    cnbt_Tag value;
    cnbt__StringData key;
    ret = cnbt__read_string(ctx, &key);
    if (ret) {
      goto compound_clean;
    }
    ret = read_data(ctx, &value, type);
    if (ret) {
      cnbt__free_string(&key);
      goto compound_clean;
    }
    stbds_shput(comp, key.data, value);
    cnbt__free_string(&key);
  }
  if (ret != CNBT_OK) {
    goto compound_clean;
  }
  d->data = comp;
  d->size = stbds_shlenu(comp);
  return CNBT_OK;

compound_clean:
  if (comp) {
    d->data = comp;
    d->size = stbds_shlenu(comp);
    cnbt__free_compound(d);
    d->data = NULL;
    d->size = 0;
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
  if (type == CNBT_TYPE_END) {
    cnbt_make_end(tag);
    return CNBT_OK;
  }
  cnbt__StringData root_name;
  ret = cnbt__read_string(&ctx, &root_name);
  if (ret) {
    goto read_err;
  }
  cnbt__free_string(&root_name);

  ret = read_data(&ctx, tag, type);
  if (ret) {
    goto read_err;
  }
  return ret;

read_err:
  if (cbs->seek && start_pos >= 0) {
    cbs->seek(src, start_pos, CNBT_SEEK_SET); // reset the stream
  }
  return ret;
}

static cnbt_Status cnbt__write_bytes(cnbt__WriteCtx* ctx, const void* buff, size_t sz) {
  assert(ctx && ctx->cbs.write);
  if (sz == 0) {
    return CNBT_OK;
  }
  size_t written = ctx->cbs.write(buff, 1, sz, ctx->src);
  return written == sz ? CNBT_OK : CNBT_EOF;
}

cnbt_Status cnbt__write_type(cnbt__WriteCtx* ctx, cnbt_Type type) {
  uint8_t b = (uint8_t)type;
  return cnbt__write_bytes(ctx, &b, 1);
}

cnbt_Status cnbt__write_byte(cnbt__WriteCtx* ctx, i8 val) {
  return cnbt__write_bytes(ctx, &val, 1);
}

cnbt_Status cnbt__write_short(cnbt__WriteCtx* ctx, i16 val) {
  uint8_t buff[2];
  if (ctx->endian_mode == CNBT_BIG_ENDIAN) {
    buff[0] = (uint8_t)((val >> 8) & 0xFF);
    buff[1] = (uint8_t)(val & 0xFF);
  } else {
    buff[0] = (uint8_t)(val & 0xFF);
    buff[1] = (uint8_t)((val >> 8) & 0xFF);
  }
  return cnbt__write_bytes(ctx, buff, 2);
}

cnbt_Status cnbt__write_int(cnbt__WriteCtx* ctx, i32 val) {
  uint8_t buff[4];
  if (ctx->endian_mode == CNBT_BIG_ENDIAN) {
    buff[0] = (uint8_t)((val >> 24) & 0xFF);
    buff[1] = (uint8_t)((val >> 16) & 0xFF);
    buff[2] = (uint8_t)((val >> 8) & 0xFF);
    buff[3] = (uint8_t)(val & 0xFF);
  } else {
    buff[0] = (uint8_t)(val & 0xFF);
    buff[1] = (uint8_t)((val >> 8) & 0xFF);
    buff[2] = (uint8_t)((val >> 16) & 0xFF);
    buff[3] = (uint8_t)((val >> 24) & 0xFF);
  }
  return cnbt__write_bytes(ctx, buff, 4);
}

cnbt_Status cnbt__write_long(cnbt__WriteCtx* ctx, i64 val) {
  uint8_t buff[8];
  if (ctx->endian_mode == CNBT_BIG_ENDIAN) {
    for (int i = 7; i >= 0; --i) {
      buff[7 - i] = (uint8_t)((val >> (i * 8)) & 0xFF);
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      buff[i] = (uint8_t)((val >> (i * 8)) & 0xFF);
    }
  }
  return cnbt__write_bytes(ctx, buff, 8);
}

cnbt_Status cnbt__write_float(cnbt__WriteCtx* ctx, f32 val) {
  union {
    i32 i;
    f32 f;
  } u;

  u.f = val;
  return cnbt__write_int(ctx, u.i);
}

cnbt_Status cnbt__write_double(cnbt__WriteCtx* ctx, f64 val) {
  union {
    i64 i;
    f64 d;
  } u;

  u.d = val;
  return cnbt__write_long(ctx, u.i);
}

cnbt_Status cnbt__write_string(cnbt__WriteCtx* ctx, const char* str, size_t len) {
  cnbt_Status ret = cnbt__write_short(ctx, (i16)len);
  if (ret != CNBT_OK) {
    return ret;
  }
  if (len > 0) {
    ret = cnbt__write_bytes(ctx, str, len);
  }
  return ret;
}

static cnbt_Status write_data(cnbt__WriteCtx* ctx, const cnbt_Tag* tag) {
  if (!tag) {
    return CNBT_INVALID_DATA;
  }
  switch (tag->type) {
    case CNBT_TYPE_END:
      return CNBT_OK;
    case CNBT_TYPE_BYTE:
      return cnbt__write_byte(ctx, tag->as_i8);
    case CNBT_TYPE_SHORT:
      return cnbt__write_short(ctx, tag->as_i16);
    case CNBT_TYPE_INT:
      return cnbt__write_int(ctx, tag->as_i32);
    case CNBT_TYPE_LONG:
      return cnbt__write_long(ctx, tag->as_i64);
    case CNBT_TYPE_FLOAT:
      return cnbt__write_float(ctx, tag->as_f32);
    case CNBT_TYPE_DOUBLE:
      return cnbt__write_double(ctx, tag->as_f64);
    case CNBT_TYPE_BYTE_ARRAY:
      return cnbt__write_byte_array(ctx, &tag->as_bytearr);
    case CNBT_TYPE_STRING:
      return cnbt__write_string(ctx, tag->as_string.data, tag->as_string.size);
    case CNBT_TYPE_LIST:
      return cnbt__write_list(ctx, &tag->as_list);
    case CNBT_TYPE_COMPOUND:
      return cnbt__write_compound(ctx, &tag->as_compound);
    case CNBT_TYPE_INT_ARRAY:
      return cnbt__write_int_array(ctx, &tag->as_intarr);
    case CNBT_TYPE_LONG_ARRAY:
      return cnbt__write_long_array(ctx, &tag->as_longarr);
    default:
      return CNBT_INVALID_DATA;
  }
}

cnbt_Status cnbt__write_byte_array(cnbt__WriteCtx* ctx, const cnbt__ByteArrayData* arr) {
  cnbt_Status ret = cnbt__write_int(ctx, (i32)arr->size);
  if (ret != CNBT_OK) {
    return ret;
  }
  if (arr->size > 0 && arr->data) {
    ret = cnbt__write_bytes(ctx, arr->data, arr->size);
  }
  return ret;
}

cnbt_Status cnbt__write_int_array(cnbt__WriteCtx* ctx, const cnbt__IntArrayData* arr) {
  cnbt_Status ret = cnbt__write_int(ctx, (i32)arr->size);
  if (ret != CNBT_OK) {
    return ret;
  }
  for (u32 i = 0; i < arr->size; ++i) {
    ret = cnbt__write_int(ctx, arr->data[i]);
    if (ret != CNBT_OK) {
      return ret;
    }
  }
  return CNBT_OK;
}

cnbt_Status cnbt__write_long_array(cnbt__WriteCtx* ctx, const cnbt__LongArrayData* arr) {
  cnbt_Status ret = cnbt__write_int(ctx, (i32)arr->size);
  if (ret != CNBT_OK) {
    return ret;
  }
  for (u32 i = 0; i < arr->size; ++i) {
    ret = cnbt__write_long(ctx, arr->data[i]);
    if (ret != CNBT_OK) {
      return ret;
    }
  }
  return CNBT_OK;
}

cnbt_Status cnbt__write_list(cnbt__WriteCtx* ctx, const cnbt__ListData* list) {
  size_t len = stbds_arrlenu(list->data);
  cnbt_Type elem_type = CNBT_TYPE_BYTE;
  if (len > 0) {
    elem_type = (cnbt_Type)list->data[0].type;
  } else {
    elem_type = CNBT_TYPE_END;
  }
  cnbt_Status ret = cnbt__write_type(ctx, elem_type);
  if (ret != CNBT_OK) {
    return ret;
  }
  ret = cnbt__write_int(ctx, (i32)len);
  if (ret != CNBT_OK) {
    return ret;
  }
  for (size_t i = 0; i < len; ++i) {
    ret = write_data(ctx, &list->data[i]);
    if (ret != CNBT_OK)
      return ret;
  }
  return CNBT_OK;
}

cnbt_Status cnbt__write_compound(cnbt__WriteCtx* ctx, const cnbt__CompoundData* comp) {
  size_t len = stbds_shlenu(comp->data);
  for (size_t i = 0; i < len; ++i) {
    const cnbt_KeyTag* kt = &comp->data[i];
    cnbt_Status ret = cnbt__write_type(ctx, (cnbt_Type)kt->value.type);
    if (ret != CNBT_OK) {
      return ret;
    }
    ret = cnbt__write_string(ctx, kt->key, strlen(kt->key));
    if (ret != CNBT_OK) {
      return ret;
    }
    ret = write_data(ctx, &kt->value);
    if (ret != CNBT_OK) {
      return ret;
    }
  }
  return cnbt__write_type(ctx, CNBT_TYPE_END);
}

CNBT_API cnbt_Status cnbt_write(const cnbt_Tag* tag, cnbt_EndianMode mode, void* src,
                                const cnbt_IoFunc* cbs) {
  if (!tag || !cbs || !cbs->write) {
    return CNBT_INVALID_DATA;
  }
  cnbt__WriteCtx ctx;
  ctx.src = src;
  ctx.cbs = *cbs;
  ctx.endian_mode = mode;

  cnbt_Status ret = cnbt__write_type(&ctx, (cnbt_Type)tag->type);
  if (ret != CNBT_OK) {
    return ret;
  }
  if (tag->type == CNBT_TYPE_END) {
    return CNBT_OK;
  }

  ret = cnbt__write_string(&ctx, "", 0); // empty root name
  if (ret != CNBT_OK) {
    return ret;
  }

  return write_data(&ctx, tag);
}
