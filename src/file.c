#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"
#include "io.h"

struct File {
  struct MdrString *curr;
  struct Know *know;
  struct Map_ *done;
};

static enum mdr_status file_str(struct File *file, struct Ast *ast) {
  string_append(file->curr, ast->info.str);
  return mdr_ok;
}

static enum mdr_status file_inc(struct File *file, struct Ast *ast) {
  struct MdrString *str = know_get(file->know, ast);
  if(!str)
    return mdr_err;
  struct RangeIncluder rs = { .str=str, .range=ast->info.range };
  string_append_range(file->curr, &rs);
  return mdr_ok;
}

static enum mdr_status file_cmd(struct File *file, struct Ast *ast) {
  struct MdrString *str = cmd(ast);
  if(!str)
    return mdr_err;
  string_append(file->curr, str);
  free_string(str);
  return mdr_ok;
}

typedef enum mdr_status (*file_ast_fn)(struct File*, struct Ast*);

static const file_ast_fn _file_ast[] = {
  file_str,
  file_inc,
  file_cmd,
};

static enum mdr_status actual_file_ast(struct File *file, struct Ast *src) {
  enum mdr_status ret = mdr_ok;
  struct Ast* ast = src->ast;
  struct RangeExcluder ex = { .src=file->curr };
  if(src->info.range.ini)
    file->curr = excluder_ini(&ex, &src->info.range);
  do ret = _file_ast[ast->type](file, ast);
  while(ret != mdr_err && (ast = ast->next));
  if(ex.end)
    excluder_end(&ex, file->curr);
  return ret;
}

enum mdr_status file(struct Mdr *mdr) {
  enum mdr_status ret = mdr_ok;
  for(vtype i = 0; ret == mdr_ok && i < map_size(&mdr->file); ++i) {
    struct File file = { .know=&mdr->know, .curr=new_string("", 0) };
    const Vector v = (Vector)map_at(&mdr->file, i);
    for(vtype j = 0; ret == mdr_ok && j < vector_size(v); ++j) {
      struct Ast *ast = (struct Ast*)vector_at(v, j);
      ret = actual_file_ast(&file, ast);
    }
    if(ret == mdr_ok)
      file_set(&mdr->know, (struct Ast*)vector_at((Vector)VVAL(&mdr->file, i), 0) , file.curr);
    else
      free_string(file.curr);
  }
  return ret;
}
