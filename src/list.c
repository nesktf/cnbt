#include "core.h"

#define STBDS_FREE(_ctx, _ptr)         CNBT__FREE(_ptr)
#define STBDS_REALLOC(_ctx, _ptr, _sz) CNBT__REALLOC(_ptr, _sz)
#define STBDS_ASSERT CNBT__ASSERT

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

CNBT_API CNBT_Status cnbt_make_list(CNBT_List* list) {
  if (!list) {
    return CNBT_INVALID_DATA;
  }
  memset(list, 0x00, sizeof(*list));
  CNBT__GET_DATA(list)->tag = CNBT_TYPE_LIST;
  CNBT__GET_DATA(list)->as_list = NULL;
  return CNBT_OK;
}

void cnbt__free_list(CNBT_Tag* list) {
  CNBT__ASSERT(list);
  for (size_t i = 0; i < stbds_arrlenu(list); ++i) {
    cnbt_free(list + i);
  }
  stbds_arrfree(list);
}

CNBT_API CNBT_Tag* cnbt_list_put_tag(CNBT_List* list, CNBT_Tag tag) {
  if (!list) {
    return NULL;
  }
  if (CNBT__GET_DATA(list)->tag != CNBT_TYPE_LIST) {
    return NULL;
  }
  stbds_arrput(CNBT__GET_DATA(list)->as_list, tag);
  return CNBT__GET_DATA(list)->as_list + stbds_arrlen(CNBT__GET_DATA(list)->as_list) - 1;
}

CNBT_API size_t cnbt_list_len(const CNBT_List* list) {
  return list ? CNBT__GET_DATA(list)->tag == CNBT_TYPE_LIST
                ? stbds_arrlen(CNBT__GET_DATA(list)->as_list)
                : 0
              : 0;
}

CNBT_API CNBT_Tag* cnbt_list_get_unchecked(const CNBT_List* list, size_t pos) {
  CNBT__ASSERT(list);
  CNBT__ASSERT(CNBT__GET_DATA(list)->tag == CNBT_TYPE_LIST);
  CNBT__ASSERT(pos < stbds_arrlen(CNBT__GET_DATA(list)->as_list));
  return CNBT__GET_DATA(list)->as_list + pos;
}

CNBT_API CNBT_Tag* cnbt_list_get(const CNBT_List* list, size_t pos) {
  if (!list) {
    return NULL;
  }
  if (CNBT__GET_DATA(list)->tag != CNBT_TYPE_LIST) {
    return NULL;
  }
  if (pos >= stbds_arrlen(CNBT__GET_DATA(list)->as_list)) {
    return NULL;
  }
  return CNBT__GET_DATA(list)->as_list + pos;
}

CNBT_API CNBT_Status cnbt_make_compound(CNBT_Compound* comp) {
  if (!comp) {
    return CNBT_INVALID_DATA;
  }
  memset(comp, 0x00, sizeof(*comp));
  CNBT__GET_DATA(comp)->tag = CNBT_TYPE_COMPOUND;
  CNBT__GET_DATA(comp)->as_compound = NULL;
  return CNBT_OK;
}

void cnbt__free_compound(CNBT_KeyTag* comp) {
  CNBT__ASSERT(comp);
  for (size_t i = 0; i < stbds_shlenu(comp); ++i) {
    cnbt_free(&comp[i].value);
  }
  stbds_shfree(comp);
}

CNBT_API CNBT_KeyTag* cnbt_comp_put_tag(CNBT_Compound* comp, const char* key, CNBT_Tag tag) {
  if (!comp || !key) {
    return NULL;
  }
  if (CNBT__GET_DATA(comp)->tag != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  stbds_shput(CNBT__GET_DATA(comp)->as_compound, key, tag);
  return shgetp(CNBT__GET_DATA(comp)->as_compound, key);
}

CNBT_API CNBT_KeyTag* cnbt_comp_get(const CNBT_Compound* comp, const char* key) {
  if (!comp || !key) {
    return NULL;
  }
  if (CNBT__GET_DATA(comp)->tag != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  return shgetp_null(CNBT__GET_DATA(comp)->as_compound, key);
}
