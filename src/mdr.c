#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"

void mdr_init(struct Mdr *mdr) {
  map_init(&mdr->snip);
  map_init(&mdr->file);
  map_init(&mdr->file_done);
  map_init(&mdr->know.curr);
}

void map_release_vector(const Map map) {
  for(vtype i = 0; i < map_size(map); ++i) {
    vector_release((Vector)VVAL(map, i));
    free((Vector)VVAL(map, i));
  }
  map_release(map);
}

void mdr_release(struct Mdr *mdr) {
  for(vtype i = 0; i < map_size(&mdr->know.curr); ++i)
    free_string((struct MdrString*)VVAL(&mdr->know.curr, i));
  map_release(&mdr->know.curr);
  for(vtype i = 0; i < map_size(&mdr->file_done); ++i)
    free_string((struct MdrString*)VVAL(&mdr->file_done, i));
  map_release(&mdr->file_done);
  map_release_vector(&mdr->snip);
  map_release_vector(&mdr->file);
}

void mdr_run(struct Mdr *mdr, struct Ast *ast) {
  if(snip(mdr) == mdr_err || file(mdr) == mdr_err)
    return;
  if(view_ast(mdr, ast) == mdr_ok) {
    for(vtype i = 0; i < map_size(&mdr->file_done); ++i) {
      char* name = (char*)VKEY(&mdr->file_done, i);
      FILE *f = mdr_open_write(name);
      if(!f)
        continue;
      struct MdrString *str = (struct MdrString*)VVAL(&mdr->file_done, i);
      fwrite(str->str, str->sz, 1, f);
      fclose(f);
    }
  }
}
