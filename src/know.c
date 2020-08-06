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

struct MdrString* snippet_find(struct Know *know, const struct MdrString *str) {
  const char *s = str->str;
  return (struct MdrString*)(map_get(&know->curr, s) ?: map_get(know->global, s));
}

static struct MdrString* snippet_get(struct Know *know, const struct Ast *ast) {
  const char *s = ast->info.str->str;
  struct MdrString* ret = snippet_find(know, ast->info.str);
  if(ret)
    return ret;
  (void)mdr_fail(&ast->loc, "can't find '%s' snippet\n", s);
  return NULL;
}

void snippet_set(struct Know *know, const char *name, const struct MdrString *str) {
  map_set(&know->curr, (vtype)strdup(name), (vtype)str);
}

static struct MdrString* get_done(const Map map, const char *key) {
  for(vtype i = VLEN(map) + 1; --i;) {
    char *ptr = (char*)VKEY(map, i - 1);
    char* str = ptr + (sizeof (struct Loc));
    if(!strcmp(key, str))
      return (struct MdrString*)VVAL(map, i - 1);
  }
  return NULL;
}

static struct MdrString* file_get(struct Know *know, const struct Ast *ast) {
  const char *s = ast->info.str->str;
  struct MdrString *exists = get_done(&know->file_done, s);
  if(exists)
    return exists;
  struct MdrString *ret = filename2str(ast->info.str->str, &ast->loc);
  if(ret)
    file_set(know, ast, ret);
  return ret;
}

struct MdrString* know_get(struct Know *know, const struct Ast *ast) {
  return (!ast->info.dot ? snippet_get : file_get)(know, ast);
}

void file_set(struct Know *know, const struct Ast *key, const struct MdrString *text) {
  const Map map = &know->file_done;
  const char *str = key->info.str->str;
  char *ptr = malloc((sizeof (struct Loc)) + strlen(str) + 1);
  *(struct Loc*)ptr = key->loc;
  strcpy(ptr + (sizeof (struct Loc)), str);
  map_set(map, (vtype)ptr, (vtype)text);
}

void know_release(struct Know *know) {
  map_release_string(&know->curr);
  map_release_string(&know->file_done);
}
