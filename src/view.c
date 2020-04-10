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
  int blk;
};

static enum mdr_status actual_view_ast(struct View*, struct Ast*);

static enum mdr_status view_str(struct View *view, struct Ast *ast) {
  string_append(view->curr, ast->info.str);
  return mdr_ok;
}

static inline void _format_long(char c[64], long n, size_t *idx) {
  if(n / 10)
    _format_long(c, n / 10, idx);
  c[*idx] = n % 10 + '0';
  ++(*idx);
}

static size_t format_long(char c[64], long n) {
  size_t ret = 0;
  if(n < 0) {
    c[0] = '-';
    ++ret;
    n = -n;
  }
  _format_long(c, n, &ret);
  return ret;
}


static void string_append_long(struct MdrString *str, const char prefix, long n) {
  char c[64];
  memset(c, 0, 64);
  c[0] = prefix;
  size_t sz = format_long(c + 1, n);
  struct MdrString tmp = { .str=c, .sz=sz + 1 };
  string_append(str, &tmp);
}

static void range_print(struct Range *range, struct MdrString *str) {
  if(!range->ini)
    return;
  string_append_long(str, ' ', range->ini);
  if(range->end)
    string_append_long(str, ':', range->end);
}

static enum mdr_status view_blk(struct View *view, struct Ast *ast) {
  {
    struct MdrString tmp = { .str="``` ", .sz=4 };
    string_append(view->curr, &tmp);
  }
  string_append(view->curr, ast->info.str);
  range_print(&ast->info.range, view->curr);// inverse arg order
  {
    struct MdrString tmp = { .str="\n", .sz=1 };
    string_append(view->curr, &tmp);
  }
  struct View new = { .know=view->know, .curr=view->curr, .blk=1 };
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
  if(view->blk) {
  {
    struct MdrString tmp = { .str="@[[ ", .sz=4 };
    string_append(view->curr, &tmp);
  }
  string_append(view->curr, ast->info.str);
  range_print(&ast->info.range, view->curr);
  {
    struct MdrString tmp = { .str=" ]]", .sz=3 };
    string_append(view->curr, &tmp);
  }
    return mdr_ok;
  }
  struct MdrString *str = !ast->info.dot ? (struct MdrString*)snippet_get(view->know, ast->info.str->str) :
    file_get(view, ast->info.str);
  if(!str)
    return mdr_err;
  struct RangeIncluder sr = { .str=str, .range=ast->info.range };
  string_append_range(view->curr, &sr);
  return mdr_ok;
}

static enum mdr_status view_cmd(struct View *view, struct Ast *ast) {
  struct MdrString *str = cmd(ast->info.str->str);
  if(!str)
    return mdr_err;
  if(!ast->info.dot) {
    struct MdrString tmp0 = { .str="<span class=\"MdrCmdPre\">", .sz=24 };
    string_append(view->curr, &tmp0);
    string_append(view->curr, ast->info.str);
    struct MdrString tmp1 = { .str="</span>\n", .sz=8 };
    string_append(view->curr, &tmp1);
    struct MdrString tmp2 = { .str="<p class=\"MdrCmd\">\n", .sz=19 };
    string_append(view->curr, &tmp2);
  }
  string_append(view->curr, str);
  if(!ast->info.dot) {
    struct MdrString tmp = { .str="</p>\n", .sz=5 };
    string_append(view->curr, &tmp);
  }
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
  if(actual_view_ast(&view, ast) == mdr_ok) {
    trim(mdr->name);
    map_set(&mdr->file_done, (vtype)mdr->name, (vtype)view.curr);
    return mdr_ok;
  }
  free_string(view.curr);
  return mdr_err;
}
