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
#include "io.h"

struct View {
  struct MdrString *curr;
  struct Know *know;
  struct ViewOpt *vopt;
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
  string_append(view->curr, view->vopt->blk_pre);
  string_append(view->curr, ast->info.str);
  range_print(&ast->info.range, view->curr);// inverse arg order
  string_append(view->curr, view->vopt->blk_ini);
  string_append(view->curr, ast->info.str);
  range_print(&ast->info.range, view->curr);// inverse arg order
  string_append(view->curr, view->vopt->blk_mid);
  struct View new = { .know=view->know, .curr=view->curr, .blk=1, .vopt=view->vopt };
  const enum mdr_status ret = actual_view_ast(&new, ast->ast);
  string_append(view->curr, view->vopt->blk_end);
  return ret;
}

static struct MdrString inc_ini = { .str="@[[ ", .sz = 4 };
static struct MdrString inc_end = { .str=" ]]", .sz = 3 };

static enum mdr_status view_inc(struct View *view, struct Ast *ast) {
  if(view->blk) {
    string_append(view->curr, &inc_ini);
    string_append(view->curr, ast->info.str);
    range_print(&ast->info.range, view->curr);
    string_append(view->curr, &inc_end);
    return mdr_ok;
  }
  struct MdrString *str = know_get(view->know, ast);
  if(!str) // err_msg
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
    string_append(view->curr, view->vopt->cmd_ini);
    string_append(view->curr, ast->info.str);
    string_append(view->curr, view->vopt->cmd_mid);
  }
  string_append(view->curr, str);
  if(!ast->info.dot)
    string_append(view->curr, view->vopt->cmd_end);
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
  struct View view = { .know=&mdr->know, .curr=new_string("", 0), .vopt=mdr->vopt };
  if(actual_view_ast(&view, ast) == mdr_err) {
    free_string(view.curr);
    return mdr_err;
  }
  const size_t sz = strlen(mdr->name);
  char c[sz];
  memcpy(c, mdr->name, sz - 1);
  c[sz - 1] = '\0';
  write_file(c, view.curr);
  return mdr_ok;
}
