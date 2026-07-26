#include "tests.h"
#include <cnbt/cnbt.h>

#include <stdio.h>

static size_t read_func(void* buff, size_t sz, size_t nmemb, void* src) {
  return fread(buff, sz, nmemb, (FILE*)src);
}

static size_t write_func(const void* buff, size_t sz, size_t nmemb, void* src) {
  return fwrite(buff, sz, nmemb, (FILE*)src);
}

static int seek_func(void* src, long offset, int origin) {
  return fseek((FILE*)src, offset, origin);
}

static long tell_func(void* src) {
  return ftell((FILE*)src);
}

static CNBT_IoCallbacks cbs = {
  .read = read_func,
  .write = write_func,
  .seek = seek_func,
  .tell = tell_func,
};

MunitResult test_write_pretty(const MunitParameter* params, void* data) {
  (void)params;
  (void)data;

  CNBT_Tag comp;
  cnbt_make_compound(&comp);

  CNBT_Tag tag, list;
  cnbt_make_short(&tag, 124);
  cnbt_comp_put_tag(&comp, "asdad", tag);

  cnbt_make_double(&tag, 333);
  cnbt_comp_put_tag(&comp, "asdad3", tag);

  cnbt_make_list(&list);
  cnbt_make_short(&tag, 4511);
  cnbt_list_put_tag(&list, tag);
  cnbt_make_short(&tag, 5555);
  cnbt_list_put_tag(&list, tag);
  cnbt_comp_put_tag(&comp, "funny_list", list);

  fprintf(stdout, "\n");
  cnbt_write_pretty(&comp, stdout, &cbs);
  cnbt_free(&comp);

  return MUNIT_OK;
}
