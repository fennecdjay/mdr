#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"
#include "line_counter.h"

struct File {
  struct Know *know;
  struct Map_ *done;
  struct MdrString *curr;
  const char *name;
};

static enum mdr_status file_str(struct File *file, struct Ast *ast) {
  string_append(file->curr, ast->str);
  return mdr_ok;
}

static enum mdr_status file_inc(struct File *file, struct Ast *ast) {
  struct MdrString *str = (struct MdrString*)snippet_get(file->know, ast->str->str);// know_done
  if(!str)
    return mdr_err;
  struct RangeIncluder rs = { .str=str->str, .range=ast->self };
  string_append_range(file->curr, &rs);
  return mdr_ok;
}

static enum mdr_status file_cmd(struct File *file, struct Ast *ast) {
  struct MdrString *str = cmd(ast->str->str);
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
/*
struct RangeExcluder {
  struct MdrString *end;
  struct MdrString *src;
};

static struct MdrString* excluder_ini(struct RangeExcluder *ex, const struct Range *range) {
  char *str = ex->src->str;
  struct Range r = actual_range(str, range);
  struct LineCounter lc = { .str=ex->src->str };
  linecounter_run(&lc, r.ini - 1);
  struct MdrString *ini = new_string(ex->src->str, lc.sz);
  linecounter_run(&lc, r.end - 1);
  const long sz = lc.sz;
  lc.count = lc.sz = 0;
  linecounter_run(&lc, LONG_MAX);
  if(r.end >= r.ini)
    ex->end = new_string(ex->src->str + sz, lc.sz);
  return ini;
}

static void excluder_end(struct RangeExcluder *ex, struct MdrString *str) {
  free_string(ex->src);
  if(!ex->end)
    return;
  string_append(str, ex->end);
  free_string(ex->end);
}
*/
static enum mdr_status actual_file_ast(struct File *file, struct Ast *ast) {
  enum mdr_status ret = mdr_ok;
  do {
    struct RangeExcluder ex = { .src=file->curr };
    if(ast->main.ini)
      file->curr = excluder_ini(&ex, &ast->main);
    ret = _file_ast[ast->type](file, ast);
    if(ex.end)
      excluder_end(&ex, file->curr);
  } while(ret != mdr_err && (ast = ast->next));
  return ret;
}

enum mdr_status file(struct Mdr *mdr) {
  enum mdr_status ret = mdr_ok;
  for(vtype i = 0; ret == mdr_ok && i < map_size(&mdr->file); ++i) {
    const char *name = (char*)VKEY(&mdr->file, i);
    struct File file = { .know=&mdr->know, .name=name, .curr=new_string("", 0) };
    const Vector v = (Vector)map_at(&mdr->file, i);
    for(vtype j = 0; ret == mdr_ok && j < vector_size(v); ++j) {
      struct Ast *ast = (struct Ast*)vector_at(v, j);
      ret = actual_file_ast(&file, ast);
    }
    if(ret == mdr_ok)
      map_set(&mdr->file_done, (vtype)name, (vtype)file.curr);
    else
      free_string(file.curr);
  }
  return ret;
}
