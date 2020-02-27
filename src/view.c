#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"

struct View {
  Map done;
  FILE *file;
  enum mdr_status code;
};

static enum mdr_status actual_view_ast(struct View*, struct Ast*);

static enum mdr_status view_str(struct View *view, struct Ast *ast) {
  fprintf(view->file, "%s", ast->str);
  return mdr_ok;
}

static enum mdr_status view_blk(struct View *view, struct Ast *ast) {
  fprintf(view->file, "``` %s\n", ast->str);
  struct View new = { .done=view->done, .file=view->file, .code=1};
  const enum mdr_status ret = ast->ast ? actual_view_ast(&new, ast->ast) : mdr_ok;
  fprintf(view->file, "```");
  return ret;
}

static enum mdr_status view_inc(struct View *view, struct Ast *ast) {
  if(view->code) {
    fprintf(view->file, "@[[ %s ]]", ast->str);
    return mdr_ok;
  }
  char *str = (char*)snippet_get(view->done, ast->str);
  if(!str)
    return mdr_err;
  fprintf(view->file, "%s", str);
  return mdr_ok;
}

static enum mdr_status view_cmd(struct View *view, struct Ast *ast) {
  return cmd_file(view->file, ast->str);
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

static FILE* view_open(const char *name) {
  const size_t sz = strlen(name);
  char str[sz];
  strncpy(str, name, sz - 1);
  trimsz(str, sz);
  return mdr_open_write(str);
}

enum mdr_status view_ast(struct Mdr *mdr, struct Ast *ast) {
  struct View view = { .done=&mdr->done};
  if(!(view.file = view_open(mdr->name)))
    return mdr_err;
  const enum mdr_status ret = actual_view_ast(&view, ast);
  fclose(view.file);
  return ret;
}
