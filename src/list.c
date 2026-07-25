#include "core.h"

#define STBDS_FREE(_ctx, _ptr)         CNBT__FREE(_ptr)
#define STBDS_REALLOC(_ctx, _ptr, _sz) CNBT__REALLOC(_ptr, _sz)

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

CNBT_API CNBT_Status cnbt_make_list(CNBT_List* list, const char* name) {
  CNBT_Status ret = cnbt__init_tag(list, name);
  if (!ret) {
    CNBT__GET_DATA(list)->tag = CNBT_TYPE_LIST;
    CNBT__GET_DATA(list)->as_list = NULL;
  }
  return ret;
}

void cnbt__free_list(CNBT_Tag* list) {
  assert(list);
  assert(CNBT__GET_DATA(list)->tag == CNBT_TYPE_LIST);
  for (size_t i = 0; i < stbds_arrlenu(CNBT__GET_DATA(list)); ++i) {
    cnbt_destroy(CNBT__GET_DATA(list)->as_list + i);
  }
  memset(CNBT__GET_DATA(list)->as_list, 0x00,
         stbds_arrlenu(CNBT__GET_DATA(list)->as_list) * sizeof(CNBT__GET_DATA(list)->as_list[0]));
  stbds_arrfree(CNBT__GET_DATA(list)->as_list);
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

CNBT_API size_t cnbt_list_size(CNBT_List* list) {
  return list ? CNBT__GET_DATA(list)->tag == CNBT_TYPE_LIST
                ? stbds_arrlen(CNBT__GET_DATA(list)->as_list)
                : 0
              : 0;
}

CNBT_API CNBT_Tag* cnbt_list_get_unchecked(CNBT_List* list, size_t pos) {
  assert(list);
  assert(CNBT__GET_DATA(list)->tag == CNBT_TYPE_LIST);
  assert(pos < stbds_arrlen(CNBT__GET_DATA(list)->as_list));
  return CNBT__GET_DATA(list)->as_list + pos;
}

CNBT_API CNBT_Tag* cnbt_list_get(CNBT_List* list, size_t pos) {
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

CNBT_API CNBT_Status cnbt_make_compound(CNBT_Compound* comp, const char* name) {
  CNBT_Status ret = cnbt__init_tag(comp, name);
  if (!ret) {
    CNBT__GET_DATA(comp)->tag = CNBT_TYPE_COMPOUND;
    CNBT__GET_DATA(comp)->as_compound = NULL;
  }
  return ret;
}

void cnbt__free_compound(CNBT_Tag* comp) {
  assert(comp);
  assert(CNBT__GET_DATA(comp)->tag == CNBT_TYPE_COMPOUND);
  for (size_t i = 0; i < stbds_shlenu(CNBT__GET_DATA(comp)->as_compound); ++i) {
    cnbt_destroy(&CNBT__GET_DATA(comp)->as_compound[i].value);
  }
  stbds_shfree(CNBT__GET_DATA(comp)->as_compound);
}

CNBT_API CNBT_CompEntry* cnbt_comp_put_tag(CNBT_Compound* comp, const char* key, CNBT_Tag tag) {
  if (!comp || !key) {
    return NULL;
  }
  if (CNBT__GET_DATA(comp)->tag != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  stbds_shput(CNBT__GET_DATA(comp)->as_compound, key, tag);
  return shgetp(CNBT__GET_DATA(comp)->as_compound, key);
}

CNBT_API CNBT_CompEntry* cnbt_comp_get(CNBT_Compound* comp, const char* key) {
  if (!comp || !key) {
    return NULL;
  }
  if (CNBT__GET_DATA(comp)->tag != CNBT_TYPE_COMPOUND) {
    return NULL;
  }
  return shgetp(CNBT__GET_DATA(comp)->as_compound, key);
}
