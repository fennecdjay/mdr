#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"

struct File {
  Map done;
  FILE *curr;
};

static enum mdr_status file_str(struct File *file, struct Ast *ast) {
  fprintf(file->curr, "%s", ast->str);
  return mdr_ok;
}

static enum mdr_status file_inc(struct File *file, struct Ast *ast) {
  char *str = (char*)snippet_get(file->done, ast->str);
  if(!str)
    return mdr_err;
  fprintf(file->curr, "%s", str);
  return mdr_ok;
}

static enum mdr_status file_cmd(struct File *file, struct Ast *ast) {
  return cmd_file(file->curr, ast->str);
}

typedef enum mdr_status (*file_ast_fn)(struct File*, struct Ast*);

static const file_ast_fn _file_ast[] = {
  file_str,
  file_inc,
  file_cmd,
};

static enum mdr_status actual_file_ast(struct File *file, struct Ast *ast) {
  struct Ast *start = ast;
  char *const str = ast->str;
  enum mdr_status ret = mdr_ok;
  do ret = _file_ast[ast->type](file, ast);
  while(ret != mdr_err && (ast = ast->next));
  start->str = str;
  return ret;
}

enum mdr_status file(struct Mdr *mdr) {
  enum mdr_status ret = mdr_ok;
  for(vtype i = 0; i < map_size(&mdr->file); ++i) {
    struct File file = { .done=&mdr->done };
    if(!(file.curr = mdr_open_write((char*)VKEY(&mdr->file, i))))
      return mdr_err;
    const Vector v = (Vector)map_at(&mdr->file, i);
    for(vtype j = 0; ret == mdr_ok && j < vector_size(v); ++j) {
      struct Ast *ast = (struct Ast*)vector_at(v, j);
      ret = actual_file_ast(&file, ast);
    }
    fclose(file.curr);
  }
  return ret;
}
