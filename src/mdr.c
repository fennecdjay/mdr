#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"

void mdr_init(struct Mdr *mdr) {
  map_init(&mdr->snip);
  map_init(&mdr->file);
  map_init(&mdr->done);
}

static void map_release_vector(const Map map) {
  for(vtype i = 0; i < map_size(map); ++i) {
    vector_release((Vector)VVAL(map, i));
    free((Vector)VVAL(map, i));
  }
  map_release(map);
}

void mdr_release(struct Mdr *mdr) {
  for(vtype i = 0; i < map_size(&mdr->done); ++i)
    free((char*)VVAL(&mdr->done, i));
  map_release(&mdr->done);
  map_release_vector(&mdr->snip);
  map_release_vector(&mdr->file);
}

void mdr_run(struct Mdr *mdr, struct Ast *ast) {
  if(snip(mdr) == mdr_err || file(mdr) == mdr_err)
    return;
  view_ast(mdr, ast);
}
