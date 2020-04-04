#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"
#include "range_helper.h"

struct File {
  struct Know know;
  FILE *curr;
  const char *name;
};

static enum mdr_status file_str(struct File *file, struct Ast *ast) {
  fprintf(file->curr, "%s", ast->str);
  return mdr_ok;
}

static enum mdr_status file_inc(struct File *file, struct Ast *ast) {
  char *str = (char*)snippet_get(&file->know, ast->str);// know_done
  if(!str)
    return mdr_err;
  struct StrRange rs = { .str=str, .range=ast->self, .file=file->curr };
  range_include(&rs);
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
  enum mdr_status ret = mdr_ok;
  char tmp[10];
  strcpy(tmp, "mdrXXXXXX");
  do {
    struct FileRange fr = { };
    if(ast->main.ini) {
      fclose(file->curr);
      mkstemp(tmp);
      file->curr = mdr_open_write(tmp);
      if(filerange_init(&fr, file->name, &ast->main) == mdr_err)
        return mdr_err;
      while(filerange_before(&fr))
        filerange_write(&fr, file->curr);
    }
    ret = _file_ast[ast->type](file, ast);
    if(ast->main.ini) {
      while(filerange_while(&fr));
      while(filerange_read(&fr))
        filerange_write(&fr, file->curr);
      filerange_release(&fr);
      rename(tmp, file->name);
    }
  } while(ret != mdr_err && (ast = ast->next));
  return ret;
}

enum mdr_status file(struct Mdr *mdr) {
  enum mdr_status ret = mdr_ok;
  for(vtype i = 0; i < map_size(&mdr->file); ++i) {
    const char *name = (char*)VKEY(&mdr->file, i);
    struct File file = { .know=mdr->know, .name=name };
    if(!(file.curr = mdr_open_write(name)))
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
