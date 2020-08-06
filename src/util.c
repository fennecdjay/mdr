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
  while(fgets(line, sizeof(line), f)) {
    buf->sz += strlen(line);
    strcat(buf->str = realloc(buf->str, buf->sz + 1), line);
  }
  return buf;
}

struct MdrString* filename2str(const char *filename, const struct Loc *loc) {
  FILE * f = mdr_open_read(filename, loc);
  if(!f)
    return NULL;
  struct MdrString *str = file2str(f);
  fclose(f);
  return str;
}

struct MdrString* cmd(const struct Ast *ast) {
#ifdef __AFL_COMPILER
  return new_string("", 0);
#endif
  const char *str = ast->info.str->str;
  FILE *f = popen(str, "r");
  if(!f) {
    mdr_fail(&ast->loc, "can't exec '%s'\n", str);
    return NULL;
  }
  struct MdrString *ret = file2str(f);
  pclose(f);
  return ret;
}

FILE* mdr_open_read(const char *str, const struct Loc *loc) {
  FILE *const file = fopen(str, "rb");
  if(file)
    return file;
  mdr_fail(loc, "can't open '%s' for reading\n", str);
  return NULL;
}

FILE* mdr_open_write(const char *str, const struct Loc *loc) {
  FILE *const file = fopen(str, "w");
  if(file)
    return file;
  mdr_fail(loc, "can't open '%s' for writing\n", str);
  return NULL;
}
