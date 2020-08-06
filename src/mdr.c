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

void write_file(const char *str, const struct Loc *loc, const struct MdrString *text) {
  FILE *f = mdr_open_write(str, loc);
  if(!f)
    return;
  fwrite(text->str, text->sz, 1, f);
  fclose(f);
}

static void do_file(struct Map_ *map) {
#ifndef __AFL_COMPILER
  for(vtype i = 0; i < map_size(map); ++i) {
    struct MdrString *text = (struct MdrString*)VVAL(map, i);
    if(text) {
      char *ptr = (char*)VKEY(map, i),
           *str = ptr + (sizeof (struct Loc));
      write_file(str, (struct Loc*)ptr, text);
    }
  }
#endif
}

void mdr_run(struct Mdr *mdr, struct Ast *ast) {
  if(snip(mdr) == mdr_err || file(mdr) == mdr_err)
    return;
  do_file(&mdr->know.file_done);
  view_ast(mdr, ast);
}

enum mdr_status mdr_fail(const struct Loc *loc, const char* fmt, ...) {
#ifdef __AFL_COMPILER
  return mdr_err;
#endif
  fprintf(stderr, "\033[31mMDR\033[0m:%s:%u-%u\n", loc->filename, loc->start, loc->end);
  va_list arg;
  va_start(arg, fmt);
  vfprintf(stderr, fmt, arg);
  va_end(arg);
  return mdr_err;
}
