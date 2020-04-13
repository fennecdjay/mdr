#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"
#include "io.h"

static struct MdrString* file2str(FILE *f) {
  char line[128];
  struct MdrString *buf = new_string("", 0);
  buf->sz = 0;
  while(fgets(line, sizeof(line), f)) {
    buf->sz += strlen(line);
    strcat(buf->str = realloc(buf->str, buf->sz + 1), line);
  }
  return buf;
}

struct MdrString* filename2str(const char* name) {
  FILE * f = mdr_open_read(name);
  if(!f)
    return NULL;
  struct MdrString *str = file2str(f);
  fclose(f);
  return str;
}

struct MdrString* cmd(const char *str) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
  return new_string("", 0);
#endif
  FILE *f = popen(str, "r");
  if(!f) {
    mdr_fail("can't exec '%s'\n", str);
    return NULL;
  }
  struct MdrString *ret = file2str(f);
  pclose(f);
  return ret;
}

FILE* mdr_open_read(const char *str) {
  FILE *const file = fopen(str, "rb");
  if(file)
    return file;
  mdr_fail("can't open '%s' for reading\n", str);
  return NULL;
}

FILE* mdr_open_write(const char *str) {
  FILE *const file = fopen(str, "w");
  if(file)
    return file;
  mdr_fail("can't open '%s' for writing\n", str);
  return NULL;
}
