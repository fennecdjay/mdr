#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"
#include "range_helper.h"

static unsigned int str_lines(void *data) {
  char *str = (char*)data;
  unsigned int count = 0;
  while(str != (char*)1 && ++count && (str = strchr(str, '\n') + 1));
  return count;
}

unsigned int file_lines(void *data) {
  FILE *f = (FILE*)data;
  unsigned int count = 0;
  char *line = NULL;
  size_t len = 0;
  while(getline(&line, &len, f) != -1)
    ++count;
  free(line);
  fseek(f, 0, SEEK_SET);
  return count + 1;
}

struct Range actual_range(void *data, const struct Range *src, unsigned int (*f)(void*)) {
  if(!(src->ini < 0 || src->end < 0))
    return *src;
  unsigned int lines = f(data);
  struct Range range = { .ini=0, .end=0 };
  range.ini = src->ini >= 0 ? src->ini : lines + src->ini;
  range.end = src->end >= 0 ? src->end : lines + src->end;
  return range;
}

enum mdr_status filerange_init(struct FileRange *fr, const char *str, struct Range *src) {
  if(!(fr->stream = mdr_open_read(str)))
    return mdr_err;
  fr->range = actual_range(fr->stream, src, file_lines);
  return mdr_ok;
}

static void range_inc(struct StrRange *sr) {
  char *str = sr->str;
  sr->range = actual_range(str, &sr->range, str_lines);
  unsigned int count = 0;
  while(str != (char*)1 && ++count < sr->range.ini && (str = strchr(str, '\n') + 1));
  if(str == (char*)1)
    return;
  while(*str && count <= sr->range.end) {
    fputc(*str, sr->file);
    ++str;
    if(*str == '\n')
      count++;
  }
}

void range_include(struct StrRange *sr) {
  if(!sr->range.ini)
    fprintf(sr->file, "%s", sr->str);
  else
    range_inc(sr);
}
