#include "cnbt_internal.h"

#define STBDS_FREE(_ctx, _ptr)         CNBT_FREE(_ptr)
#define STBDS_REALLOC(_ctx, _ptr, _sz) CNBT_REALLOC(_ptr, _sz)
#define STBDS_ASSERT                   assert

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

CNBT_API cnbt_Status cnbt_make_list(cnbt_List* list) {
  if (!list) {
    return CNBT_INVALID_DATA;
  }
  memset(list, 0x00, sizeof(*list));
  list->type = CNBT_TYPE_LIST;
  return CNBT_OK;
}

void cnbt__free_list(cnbt__ListData* list) {
  if (!list) {
    return;
  }
  for (size_t i = 0; i < stbds_arrlenu(list->data); ++i) {
    cnbt_free(list->data + i);
  }
  stbds_arrfree(list->data);
}

void cnbt__free_byte_array(cnbt__ByteArrayData* arr) {
  if (!arr) {
    return;
  }
  CNBT_FREE(arr->data);
}

CNBT_API cnbt_Tag* cnbt_list_push(cnbt_List* list, cnbt_Tag tag) {
  if (!list) {
    return NULL;
  }
  if (list->type != CNBT_TYPE_LIST) {
    return NULL;
  }
  stbds_arrput(list->as_list.data, tag);
  return list->as_list.data + stbds_arrlen(list->as_list.data) - 1;
}

CNBT_API size_t cnbt_list_len(const cnbt_List* list) {
  return list ? list->type == CNBT_TYPE_LIST ? stbds_arrlen(list->as_list.data) : 0 : 0;
}

CNBT_API cnbt_Tag* cnbt_list_get_unchecked(const cnbt_List* list, size_t pos) {
  assert(list);
  assert(list->type == CNBT_TYPE_LIST);
  assert(pos < stbds_arrlen(list->as_list.data));
  return list->as_list.data + pos;
}

CNBT_API cnbt_Tag* cnbt_list_get(const cnbt_List* list, size_t pos) {
  if (!list) {
    return NULL;
  }
  if (list->type != CNBT_TYPE_LIST) {
    return NULL;
  }
  if (pos >= stbds_arrlen(list->as_list.data)) {
    return NULL;
  }
  return list->as_list.data + pos;
}

CNBT_API cnbt_Status cnbt_make_compound(cnbt_Compound* comp) {
  if (!comp) {
    return CNBT_INVALID_DATA;
  }
  memset(comp, 0x00, sizeof(*comp));
  comp->type = CNBT_TYPE_COMPOUND;
  return CNBT_OK;
}

void cnbt__free_compound(cnbt__CompoundData* comp) {
  if (!comp) {
    return;
  }
  for (size_t i = 0; i < stbds_shlenu(comp->data); ++i) {
    cnbt_free(&comp->data[i].value);
  }
  stbds_shfree(comp->data);
}

CNBT_API cnbt_KeyTag* cnbt_comp_insert(cnbt_Compound* comp, const char* key, cnbt_Tag tag) {
  if (!comp || !key) {
    return NULL;
  }
  if (comp->type != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  stbds_shput(comp->as_compound.data, key, tag);
  return shgetp_null(comp->as_compound.data, key);
}

CNBT_API cnbt_KeyTag* cnbt_comp_get(const cnbt_Compound* comp, const char* key) {
  if (!comp || !key) {
    return NULL;
  }
  if (comp->type != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  cnbt_KeyTag* tags = (cnbt_KeyTag*)comp->as_compound.data;
  return shgetp_null(tags, key);
}
