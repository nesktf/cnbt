#include "core.h"
#include "stb_ds.h"

#include <stdio.h>

CNBT_API CNBT_Status cnbt_write(const CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs);
CNBT_API CNBT_Status cnbt_read(CNBT_Tag* tag, void* src, const CNBT_IoCallbacks* cbs);

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
    cbs->write(endstr, sizeof(endstr[0]), sizeof(endstr)-1, src);
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
      c = snprintf(ctx->buff, sizeof(ctx->buff), "0x%02X",
                   (uint8_t)CNBT__GET_DATA(tag)->as_blob[i]);
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
