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

struct MdrString* snippet_get(struct Know *know, const struct MdrString *str) {
  struct MdrString* ret = snippet_find(know, str);
  if(ret)
    return ret;
  (void)mdr_fail("can't find '%s' snippet\n", str->str);
  return NULL;
}

void snippet_set(struct Know *know, const char *name, const struct MdrString *str) {
  map_set(&know->curr, (vtype)strdup(name), (vtype)str);
}

struct MdrString* file_get(struct Know *know, const struct MdrString *str) {
  const char *s = str->str;
  struct MdrString *exists = (struct MdrString*)map_get(&know->file_done, s);
  if(exists)
    return exists;
  struct MdrString *ret = filename2str(s);
  if(ret)
    file_set(know, s, ret);
  return ret;
}

struct MdrString* know_get(struct Know *know, const struct Ast *ast) {
  return (!ast->info.dot ? snippet_get : file_get)(know, ast->info.str);
}

void file_set(struct Know *know, const char *name, const struct MdrString *str) {
  map_set(&know->file_done, (vtype)strdup(name), (vtype)str);
}

void know_release(struct Know *know) {
  map_release_string(&know->curr);
  map_release_string(&know->file_done);
}
