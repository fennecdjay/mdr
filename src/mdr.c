#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"
#include "viewopt.h"

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

void mdr_run(struct Mdr *mdr, struct Ast *ast) {
  if(snip(mdr) == mdr_err || file(mdr) == mdr_err)
    return;
  if(view_ast(mdr, ast) == mdr_ok) {
    for(vtype i = 0; i < map_size(&mdr->know.file_done); ++i) {
      char* name = (char*)VKEY(&mdr->know.file_done, i);
      FILE *f = mdr_open_write(name);
      if(!f)
        continue;
      struct MdrString *str = (struct MdrString*)VVAL(&mdr->know.file_done, i);
      fwrite(str->str, str->sz, 1, f);
      fclose(f);
    }
  }
}
