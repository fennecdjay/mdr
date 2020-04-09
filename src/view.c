#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"

struct View {
  struct Know *know;
  Map file;
  struct MdrString *curr;
  enum mdr_status code;
};

static enum mdr_status actual_view_ast(struct View*, struct Ast*);

static enum mdr_status view_str(struct View *view, struct Ast *ast) {
  string_append(view->curr, ast->str);
  return mdr_ok;
}

static void range_print(struct Range *range, struct MdrString *str) {
  if(!range->ini)
    return;
  char c[64];
  sprintf(c, " %li", range->ini);
  struct MdrString tmp = { .str=c, .sz=strlen(c) };
  string_append(str, &tmp);
  if(range->end) {
    sprintf(c, ":%li", range->ini);
    struct MdrString tmp = { .str=c, .sz=strlen(c) };
    string_append(str, &tmp);
  }
}

static enum mdr_status view_blk(struct View *view, struct Ast *ast) {
  {
    struct MdrString tmp = { .str="``` ", .sz=4 };
    string_append(view->curr, &tmp);
  }
  string_append(view->curr, ast->str);
  range_print(&ast->ast->main, view->curr);// inverse arg order
  {
    struct MdrString tmp = { .str="\n", .sz=1 };
    string_append(view->curr, &tmp);
  }
  struct View new = { .know=view->know, .curr=view->curr, .code=1};
  const enum mdr_status ret = actual_view_ast(&new, ast->ast);
  {
    struct MdrString tmp = { .str="```", .sz=3 };
    string_append(view->curr, &tmp);
  }
  return ret;
}

static struct MdrString* file_get(struct View *view, struct MdrString *str) {
  struct MdrString *exists = (struct MdrString*)map_get(view->file, str->str);
  if(exists)
    return exists;
  struct MdrString *ret = filename2str(str->str);
  map_set(view->file, (vtype)str->str, (vtype)ret);
  return ret;
}

static enum mdr_status view_inc(struct View *view, struct Ast *ast) {
  if(view->code) {
  {
    struct MdrString tmp = { .str="@[[ ", .sz=4 };
    string_append(view->curr, &tmp);
  }
  string_append(view->curr, ast->str);
  range_print(&ast->self, view->curr);
  {
    struct MdrString tmp = { .str=" ]]", .sz=3 };
    string_append(view->curr, &tmp);
  }
    return mdr_ok;
  }
  struct MdrString *str = !ast->dot ? (struct MdrString*)snippet_get(view->know, ast->str->str) :
    file_get(view, ast->str);
  if(!str)
    return mdr_err;
  struct RangeIncluder sr = { .str=str->str, .range=ast->self };
  string_append_range(view->curr, &sr);
  return mdr_ok;
}

static enum mdr_status view_cmd(struct View *view, struct Ast *ast) {
  struct MdrString *str = cmd(ast->str->str);
  if(!str)
    return mdr_err;
  string_append(view->curr, str);
  free_string(str);
  return mdr_ok;
}

typedef enum mdr_status (*view_ast_fn)(struct View*, struct Ast*);

static const view_ast_fn _view_ast[] = {
  view_str,
  view_inc,
  view_cmd,
  view_blk,
};

static enum mdr_status actual_view_ast(struct View *view, struct Ast *ast) {
  do _view_ast[ast->type](view, ast);
  while((ast = ast->next));
  return mdr_ok;
}

enum mdr_status view_ast(struct Mdr *mdr, struct Ast *ast) {
  struct View view = { .know=&mdr->know, .file=&mdr->file_done, .curr=new_string("", 0) };
  const enum mdr_status ret = actual_view_ast(&view, ast);
  if(ret == mdr_err)
    free_string(view.curr);
  else {
    trim(mdr->name);
    map_set(&mdr->file_done, (vtype)mdr->name, (vtype)view.curr);
  }
  return ret;
}
