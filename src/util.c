#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "container.h"
#include "mdr.h"

static char* file2str(FILE *f) {
  char line[128];
  char *buf = empty_string();
  unsigned int size = 1;
  while(fgets(line, sizeof(line), f)) {
    size += strlen(line);
    strcat(buf=realloc(buf,size + 1), line);
  }
  return buf;
}

char* filename2str(const char* name) {
  FILE * f = fopen(name, "rb");
  if(!f) {
    mdr_fail("can't open file '%s' for reading\n", name);
    return NULL;
  }
  char *str = file2str(f);
  fclose (f);
  return str;
}

char* cmd(const char *str) {
  char cmd[strlen(str) + 16];
  sprintf(cmd, "%s", str);
  FILE *f = popen(cmd, "r");
  assert(f);
  char *ret = file2str(f);
  pclose(f);
  return ret;
}

int cmd_file(FILE *file, const char *str) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
  (void)file;
  return 1;
#endif
  char *ret = cmd(str);
  if(ret) {
    fprintf(file, "%s", ret);
    free(ret);
  }
  return !ret;
}

enum mdr_status mdr_fail(const char* fmt, ...) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
  return mdr_err;
#endif
  fprintf(stderr, "\033[31mMDR\033[0m: ");
  va_list arg;
  va_start(arg, fmt);
  vfprintf(stderr, fmt, arg);
  va_end(arg);
  return mdr_err;
}

FILE* mdr_open_write(const char *str) {
  FILE *const file = fopen(str, "w");
  if(file)
    return file;
  mdr_fail("can't open '%s' for writing\n", str);
  return NULL;
}

vtype snippet_get(Map map, const char *str) {
  const vtype ret = map_get(map, str);
  if(ret)
    return ret;
  (void)mdr_fail("can't find '%s' snippet\n", str);
  return 0;
}
