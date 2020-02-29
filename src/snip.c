#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr.h"

struct Snip {
  Map known;
  Map done;
  Vector vec;
  char *str;
};

static enum mdr_status actual_snip_ast(struct Snip * snip, struct Ast *ast);
static char* expand(struct Snip* base, const char *name, const Vector v);

static char * append_strings(char *old, const char *new) {
  size_t len = strlen(old) + strlen(new) + 1;
  char *out = malloc(len);
  sprintf(out, "%s%s", old, new);
  free(old);
  return out;
}

static int snip_exists(struct Snip *snip, char *name) {
  for(vtype i = 0; i < vector_size(snip->vec); ++i) {
    if(!strcmp(name, (char*)vector_at(snip->vec, i)))
      return 1;
  }
  return 0;
}

static enum mdr_status snip_str(struct Snip *snip, struct Ast *ast) {
  snip->str = append_strings(snip->str, ast->str);
  return mdr_ok;
}

static int snip_done(struct Snip * snip, struct Ast *ast) {
  char *str = (char*)map_get(snip->done, ast->str);
  if(!str)
    return 0;
  snip->str = append_strings(snip->str, str);
  trim(snip->str);
  return 1;
}

static enum mdr_status snip_inc(struct Snip * snip, struct Ast *ast) {
  if(snip_exists(snip, ast->str))
    return mdr_fail("recursive snippet '%s'\n", ast->str);
  if(snip_done(snip, ast))
    return mdr_ok;
  char *base = snip->str;
  const Vector v = (Vector)snippet_get(snip->known, ast->str);
  if(!v)
    return mdr_err;
  const char* str = expand(snip, ast->str, v);
  if(str) {
    snip->str = append_strings(base, str);
    trim(snip->str);
    return mdr_ok;
  }
  return mdr_err;
}

static enum mdr_status snip_cmd(struct Snip * snip, struct Ast *ast) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
return mdr_ok;
#endif
  char *str = cmd(ast->str);
  if(!str)
    return mdr_err;
  char *base = snip->str;
  snip->str = append_strings(base, str);
  free(str);
  return mdr_ok;
}

typedef enum mdr_status (*snip_ast_fn)(struct Snip * snip, struct Ast*);

static const snip_ast_fn _snip_ast[] = {
  snip_str,
  snip_inc,
  snip_cmd,
};

static enum mdr_status actual_snip_ast(struct Snip * snip, struct Ast *ast) {
  do {
    enum mdr_status ret = _snip_ast[ast->type](snip, ast);
    if(ret == mdr_err)
      return mdr_err;
  } while((ast = ast->next));
  return mdr_ok;
}

static char* snip_get(struct Snip* base, struct Ast *ast) {
  struct Snip snip = { .known=base->known, .done=base->done, .vec=base->vec };
  snip.str = empty_string();
  if(actual_snip_ast(&snip, ast) != mdr_err)
    return snip.str;
  free(snip.str);
  return NULL;
}

static char* expand(struct Snip* base, const char *name, const Vector v) {
  vector_add(base->vec, (vtype)name);
  char *str = empty_string();
  for(vtype i = 0; i < vector_size(v); ++i) {
    char *curr = snip_get(base, (struct Ast*)vector_at(v, i));
    if(!curr) {
      free(str);
      return NULL;
    }
    str = append_strings(str, curr);
    free(curr);
  }
  trim(str);
  map_set(base->done, (vtype)name, (vtype)str);
  return str;
}

enum mdr_status snip(struct Mdr *mdr) {
  for(vtype i = 0; i < map_size(&mdr->snip); ++i) {
    const char*name = (char*)VKEY(&mdr->snip, i);
    if(map_get(&mdr->done, name))
      continue;
    const Vector v = (Vector)VVAL(&mdr->snip, i);
    struct Vector_ vec;
    vector_init(&vec);
    struct Snip snip = { .known=&mdr->snip, .done=&mdr->done, .vec=&vec };
    const char *ret = expand(&snip, name, v);
    vector_release(&vec);
    if(!ret)
      return mdr_err;
  }
  return mdr_ok;
}
