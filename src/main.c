#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"

#ifndef __AFL_HAVE_MANUAL_CONTROL

static int usage(void) {
  fputs("usage: mdr <files>\n", stderr);
  return EXIT_FAILURE;
}

int main(int argc, char **argv) {
  if(argc < 2)
    return usage();
  --argv;
  ++argv;
  for(int i = 1; i < argc; ++i) {
    struct Mdr mdr = { .name=argv[i] };
    mdr_init(&mdr);
    char * str= filename2str(argv[i]);
    if(str) {
      struct Ast *ast = mdr_parse(&mdr, str);
      free(str);
      if(ast) {
        mdr_run(&mdr, ast);
        free_ast(ast);
      }
    }
    mdr_release(&mdr);
  }
  return EXIT_SUCCESS;
}

#else

#include <string.h>
#define BUFSIZE 1024
int main(int argc, char **argv) {
  char buf[BUFSIZE];
  __AFL_INIT();
  while(__AFL_LOOP(10)) {
    memset(buf, 0, BUFSIZE);
    read(0, buf, BUFSIZE);
    struct Mdr mdr = { .name="afl"};
    mdr_init(&mdr);
    struct Ast *ast = mdr_parse(&mdr, buf);
    if(ast) {
      mdr_run(&mdr, ast);
      free_ast(ast);
    }
    mdr_release(&mdr);
  }
  return EXIT_SUCCESS;
}
#endif
