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
  FILE *f = popen(str, "r");
  if(!f) {
    mdr_fail("can't exec '%s'\n", str);
    return NULL;
  }
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
  if(!ret)
    return mdr_err;
  fprintf(file, "%s", ret);
  free(ret);
  return mdr_ok;
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

vtype snippet_get(struct Know *know, const char *str) {
  const vtype ret = map_get(&know->curr, str) ?: map_get(know->main, str);
  if(ret)
    return ret;
  (void)mdr_fail("can't find '%s' snippet\n", str);
  return 0;
}

#ifdef __MINGW32__
char *strndup(const char *s, size_t n) {
  char* c = malloc(n+1);
  if(c) {
    strncpy(c, s, n);
    c[n] = '\0';
  }
  return c;
}
#endif
