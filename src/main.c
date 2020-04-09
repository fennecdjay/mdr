#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"

struct Ast* prepare(struct Mdr *mdr, char *filename) {
  struct MdrString *str= filename2str(filename);
  if(!str)
    return NULL;
  struct Ast *ast = mdr_parse(mdr, str->str);
  free_string(str);
  return ast;
}

void run(struct Mdr *mdr, char* buf) {
  mdr_init(mdr);
  struct Ast *ast = mdr_parse(mdr, buf);
  if(ast) {
    mdr_run(mdr, ast);
    free_ast(ast);
  }
  mdr_release(mdr);
}

#ifndef __AFL_HAVE_MANUAL_CONTROL

static int usage(void) {
  fputs("usage: mdr <files>\n", stderr);
  return EXIT_FAILURE;
}

static void _fill_global(Map map, char *str) {
  struct Mdr mdr = { .know= { .global = map } };
  mdr.know.curr.ptr = map->ptr;
  struct Vector_ v;
  vector_init(&v);
  map_init(&mdr.snip);
  map_init(&mdr.file);
  while(str) {
    char *old = str;
    if((str = strchr(++str, ':'))) {
      str[0] = '\0';
      ++str;
    }
    struct Ast *ast = prepare(&mdr, old);
    if(ast)
      vector_add(&v, (vtype)ast);
  }
  snip(&mdr);
  for(vtype i = 0; i < map_size(map); ++i)
     VKEY(map, i) = (vtype)strdup((char*)VKEY(map, i));
  map_release_vector(&mdr.file);
  map_release_vector(&mdr.snip);
  for(vtype i = 0; i < vector_size(&v); ++i)
    free_ast((struct Ast*)vector_at(&v, i));
  vector_release(&v);
}

static inline void fill_global(Map map) {
  char *str = getenv("MDR_MAIN");
  if(str)
    _fill_global(map, str);
}

int main(int argc, char **argv) {
  if(argc < 2)
    return usage();
  struct Map_ global;
  map_init(&global);
  fill_global(&global);
  for(int i = 1; i < argc; ++i) {
    struct Mdr mdr = { .name=argv[i], .know = { .global=&global } };
    struct MdrString *str= filename2str(argv[i]);
    if(str) {
      run(&mdr, str->str);
      free_string(str);
    }
  }
  for(vtype i = 0; i < map_size(&global); ++i) {
    free((char*)VKEY(&global, i));
    free_string((struct MdrString*)VVAL(&global, i));
  }
  map_release(&global);
  return EXIT_SUCCESS;
}

#else

#define BUFSIZE 1024

int main(int argc, char **argv) {
  char buf[BUFSIZE];
  __AFL_INIT();
  struct Map_ global;
  map_init(&global);
  while(__AFL_LOOP(1000)) {
    memset(buf, 0, BUFSIZE);
    read(0, buf, BUFSIZE);
    struct Mdr mdr = { .name="afl", .know={ .global=&global } };
    run(&mdr, buf);
  }
  map_release(&global);
  return EXIT_SUCCESS;
}

#endif
