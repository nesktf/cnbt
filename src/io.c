#include "core.h"
#include "stb_ds.h"

#include <stdio.h>

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

typedef struct PrettyCtx {
  char buff[256];
  int indent;
} PrettyCtx;

static const char int_post[] = {'b', 's', 'i', 'l'};
static const char indent[] = "  ";

static const char comp_chars[] = {'{', '\n', '}', '\n', ':', ' ', '{', '}', '\n'};
#define COMP_WRITECHAR(off, n)                                                  \
  cbs->write(comp_chars + (off * sizeof(comp_chars[0])), sizeof(comp_chars[0]), \
             n * sizeof(comp_chars[0]), src)

static const char list_chars[] = {'[', ']', '\n', ',', ' '};
#define LIST_WRITECHAR(off, n)                                                  \
  cbs->write(list_chars + (off * sizeof(list_chars[0])), sizeof(list_chars[0]), \
             n * sizeof(list_chars[0]), src)

static const char endstr[] = "END";

static void write_indent(void* src, const CNBT_IoCallbacks* cbs, int n) {
  for (int i = 0; i < n; ++i) {
    cbs->write(indent, sizeof(indent[0]), sizeof(indent) - 1, src);
  }
}

static void write_pretty_tag(PrettyCtx* ctx, const CNBT_Tag* tag, void* src,
                             const CNBT_IoCallbacks* cbs) {
  const uint8_t type = CNBT__GET_DATA(tag)->tag;
  if (type == CNBT_TYPE_END) {
    cbs->write(endstr, sizeof(endstr[0]), sizeof(endstr) - 1, src);
    return;
  }

  int c;
  if (type >= CNBT_TYPE_BYTE && type <= CNBT_TYPE_LONG) {
    c = snprintf(ctx->buff, sizeof(ctx->buff), "%li%c", CNBT__GET_DATA(tag)->as_int,
                 int_post[type - CNBT_TYPE_BYTE]);
    cbs->write(ctx->buff, sizeof(ctx->buff[0]), c, src);
    return;
  }

  if (type == CNBT_TYPE_FLOAT) {
    c = snprintf(ctx->buff, sizeof(ctx->buff), "%ff", CNBT__GET_DATA(tag)->as_float);
    cbs->write(ctx->buff, sizeof(ctx->buff[0]), c, src);
    return;
  }

  if (type == CNBT_TYPE_DOUBLE) {
    c = snprintf(ctx->buff, sizeof(ctx->buff), "%fd", CNBT__GET_DATA(tag)->as_double);
    cbs->write(ctx->buff, sizeof(ctx->buff[0]), c, src);
    return;
  }

  if (type == CNBT_TYPE_STRING) {
    const char ch = '"';
    cbs->write(&ch, sizeof(ch), 1, src);
    cbs->write(CNBT__GET_DATA(tag)->as_str, sizeof(ch), CNBT__GET_DATA(tag)->size, src);
    cbs->write(&ch, sizeof(ch), 1, src);
    return;
  }

  if (type == CNBT_TYPE_BYTE_ARRAY) {
    const size_t sz = CNBT__GET_DATA(tag)->size;
    if (sz == 0) {
      LIST_WRITECHAR(0, 3); // "[]\n"
      return;
    }
    LIST_WRITECHAR(0, 1); // "["
    for (size_t i = 0; i < sz; ++i) {
      c =
        snprintf(ctx->buff, sizeof(ctx->buff), "0x%02X", (uint8_t)CNBT__GET_DATA(tag)->as_blob[i]);
      cbs->write(ctx->buff, sizeof(ctx->buff[0]), c, src);
      if (i < sz - 1) {
        LIST_WRITECHAR(3, 2);
      }
    }
    LIST_WRITECHAR(1, 1); // "]"
  }

  if (type == CNBT_TYPE_LIST) {
    const size_t sz = stbds_arrlenu(CNBT__GET_DATA(tag)->as_list);
    if (sz == 0) {
      LIST_WRITECHAR(0, 3); // "[]\n"
      return;
    }
    LIST_WRITECHAR(0, 1); // "["
    for (size_t i = 0; i < sz; ++i) {
      write_pretty_tag(ctx, CNBT__GET_DATA(tag)->as_list + i, src, cbs);
      if (i < sz - 1) {
        LIST_WRITECHAR(3, 2);
      }
    }
    LIST_WRITECHAR(1, 1); // "]"
  }

  if (type == CNBT_TYPE_COMPOUND) {
    const size_t sz = stbds_shlenu(CNBT__GET_DATA(tag)->as_compound);
    if (sz == 0) {
      COMP_WRITECHAR(6, 3); // "{}\n"
      return;
    }
    COMP_WRITECHAR(0, 2); // "{\n"
    ctx->indent++;
    for (size_t i = 0; i < sz; ++i) {
      size_t len = strlen(CNBT__GET_DATA(tag)->as_compound[i].key);
      write_indent(src, cbs, ctx->indent);
      cbs->write(CNBT__GET_DATA(tag)->as_compound[i].key, 1, len, src);
      COMP_WRITECHAR(4, 2); // ": "
      write_pretty_tag(ctx, &CNBT__GET_DATA(tag)->as_compound[i].value, src, cbs);
      COMP_WRITECHAR(1, 1); // \n
    }
    ctx->indent--;
    write_indent(src, cbs, ctx->indent);
    COMP_WRITECHAR(2, 2); // "}\n"
    return;
  }
}

CNBT_API CNBT_Status cnbt_write_pretty(const CNBT_Tag* tag, void* src,
                                       const CNBT_IoCallbacks* cbs) {
  if (!tag || !cbs) {
    return CNBT_INVALID_DATA;
  }
  if (!cbs->write) {
    return CNBT_INVALID_DATA;
  }
  PrettyCtx ctx = {0};
  write_pretty_tag(&ctx, tag, src, cbs);
  return CNBT_OK;
}
