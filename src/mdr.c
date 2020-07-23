#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"
#include "viewopt.h"
#include "io.h"

void mdr_init(struct Mdr *mdr) {
  map_init(&mdr->snip);
  map_init(&mdr->file);
  map_init(&mdr->know.file_done);
  map_init(&mdr->know.curr);
}

void mdr_release(struct Mdr *mdr) {
  know_release(&mdr->know);
  map_release_vector(&mdr->snip);
  map_release_vector(&mdr->file);
}

static void do_file(struct Map_ *map) {
#ifndef __AFL_COMPILER
  for(vtype i = 0; i < map_size(map); ++i) {
    char* name = (char*)VKEY(map, i);
    FILE *f = mdr_open_write(name);
    if(!f)
      continue;
    struct MdrString *str = (struct MdrString*)VVAL(map, i);
    if(str)
      fwrite(str->str, str->sz, 1, f);
    fclose(f);
  }
#endif
}

void mdr_run(struct Mdr *mdr, struct Ast *ast) {
  if(snip(mdr) == mdr_err || file(mdr) == mdr_err)
    return;
  do_file(&mdr->know.file_done);
  view_ast(mdr, ast);
}


enum mdr_status mdr_fail(const char* fmt, ...) {
#ifdef __AFL_COMPILER
  return mdr_err;
#endif
  fprintf(stderr, "\033[31mMDR\033[0m: ");
  va_list arg;
  va_start(arg, fmt);
  vfprintf(stderr, fmt, arg);
  va_end(arg);
  return mdr_err;
}
