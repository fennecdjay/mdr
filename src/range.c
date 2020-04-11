#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "line_counter.h"
#include "mdr.h"

static long str_lines(char *str) {
  struct LineCounter lc = { .str=str };
  linecounter_run(&lc, LONG_MAX);
  return lc.count;
}

struct Range actual_range(char *str, const struct Range *src) {
  if(!(src->ini < 0 || src->end < 0))
    return *src;
  long lines = str_lines(str);
  struct Range range = { .ini=0, .end=0 };
  range.ini = src->ini >= 0 ? src->ini : lines + src->ini;
  if(range.ini < 0)
    range.ini = 0;
  range.end = src->end >= 0 ? src->end : lines + src->end;
  if(range.end < 0)
    range.end = 0;
  return range;
}

static void _string_append_range(struct MdrString *base, struct RangeIncluder *sr) {
  char *str = sr->str->str;
  sr->range = actual_range(base->str, &sr->range);
  struct LineCounter lc = { .str=str };
  linecounter_run(&lc, sr->range.ini - 1);
  size_t sz = lc.sz;
  lc.sz = 0;
  linecounter_run(&lc, sr->range.end + 1);
  struct MdrString new =  { .str=str + sz, .sz=lc.sz };
  string_append(base, &new);
}

void string_append_range(struct MdrString *base, struct RangeIncluder *sr) {
  if(!sr->range.ini)
    string_append(base, sr->str);
  else
    _string_append_range(base, sr);
}

struct MdrString* excluder_ini(struct RangeExcluder *ex, const struct Range *range) {
  char *str = ex->src->str;
  struct Range r = actual_range(str, range);
  struct LineCounter lc = { .str=ex->src->str };
  linecounter_run(&lc, r.ini - 1);
  struct MdrString *ini = new_string(ex->src->str, lc.sz);
  linecounter_run(&lc, r.end);
  const long sz = lc.sz;
  lc.count = lc.sz = 0;
  linecounter_run(&lc, LONG_MAX);
  if(r.end >= r.ini)
    ex->end = new_string(ex->src->str + sz, lc.sz);
  return ini;
}

void excluder_end(struct RangeExcluder *ex, struct MdrString *str) {
  free_string(ex->src);
  if(!ex->end)
    return;
  string_append(str, ex->end);
  free_string(ex->end);
}
