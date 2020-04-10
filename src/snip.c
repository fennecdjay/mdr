#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"

struct Snip {
  Map known;
  struct Know *know;
  Vector vec;
  struct MdrString *str;
};

static enum mdr_status actual_snip_ast(struct Snip * snip, struct Ast *ast);
static struct MdrString* expand(struct Snip* base, const char *name, const Vector v);

static int snip_exists(struct Snip *snip, char *name) {
  for(vtype i = 0; i < vector_size(snip->vec); ++i) {
    if(!strcmp(name, (char*)vector_at(snip->vec, i)))
      return 1;
  }
  return 0;
}

static enum mdr_status snip_str(struct Snip *snip, struct Ast *ast) {
  string_append(snip->str, ast->info.str);
  return mdr_ok;
}

static int snip_done(struct Snip * snip, struct Ast *ast) {
  struct MdrString *str = (struct MdrString*)map_get(&snip->know->curr, ast->info.str->str);
  if(!str)
    return 0;
  string_append(snip->str, str);
  return 1;
}

static struct MdrString* include_string(struct Snip *snip, struct Ast *ast) {
  if(snip_done(snip, ast))
    return (struct MdrString*)mdr_ok;
  const Vector v = (Vector)map_get(snip->known, ast->info.str->str);
  if(v)
    return expand(snip, ast->info.str->str, v);
  (void)mdr_fail("missing snippet '%s'\n", ast->info.str->str);
  return NULL;
}

static enum mdr_status snip_inc(struct Snip *snip, struct Ast *ast) {
  if(snip_exists(snip, ast->info.str->str))
    return mdr_fail("recursive snippet '%s'\n", ast->info.str->str);
  struct MdrString *str = !ast->info.dot ?
    include_string(snip, ast) : filename2str(ast->info.str->str);
  if(!str)
    return mdr_err;
  if(str == (struct MdrString*)mdr_ok)
    return mdr_ok;
  if(!ast->info.range.ini)
    string_append(snip->str, str);
  else {
    struct RangeIncluder range = { .str=str, .range=ast->info.range };
    string_append_range(snip->str, &range);
  }
  if(ast->info.dot)
    free_string(str);
  return mdr_ok;
}

static enum mdr_status snip_cmd(struct Snip * snip, struct Ast *ast) {
#ifdef __AFL_HAVE_MANUAL_CONTROL
return mdr_ok;
#endif
  struct MdrString *str = cmd(ast->info.str->str);
  if(!str)
    return mdr_err;
  string_append(snip->str, str);
  free_string(str);
  return mdr_ok;
}

typedef enum mdr_status (*snip_ast_fn)(struct Snip * snip, struct Ast*);

static const snip_ast_fn _snip_ast[] = {
  snip_str,
  snip_inc,
  snip_cmd,
};

static enum mdr_status actual_snip_ast(struct Snip * snip, struct Ast *ast) {
  do if(_snip_ast[ast->type](snip, ast) == mdr_err)
      return mdr_err;
  while((ast = ast->next));
  return mdr_ok;
}

static struct MdrString* snip_get(struct Snip* base, struct Ast *ast) {
  struct Snip snip = { .known=base->known, .know=base->know, .vec=base->vec };
  snip.str = new_string("", 0);
  if(actual_snip_ast(&snip, ast) != mdr_err)
    return snip.str;
  free_string(snip.str);
  return NULL;
}

static struct MdrString* expand(struct Snip* base, const char *name, const Vector v) {
  vector_add(base->vec, (vtype)name);
  struct MdrString *str = new_string("", 0);
  for(vtype i = 0; i < vector_size(v); ++i) {
    struct MdrString *curr = snip_get(base, ((struct Ast*)vector_at(v, i))->ast);
    if(!curr) {
      free_string(str);
      return NULL;
    }
    string_append(str, curr);

    if(str->sz && str->str[str->sz - 1] == '\n') {
      trim(str->str);
      --str->sz;
    }

    free_string(curr);
  }
  map_set(&base->know->curr, (vtype)name, (vtype)str);
  return str;
}

enum mdr_status snip(struct Mdr *mdr) {
  for(vtype i = 0; i < map_size(&mdr->snip); ++i) {
    const char*name = (char*)VKEY(&mdr->snip, i);
    if(map_get(&mdr->know.curr, name))
      continue;
    const Vector v = (Vector)VVAL(&mdr->snip, i);
    struct Vector_ vec;
    vector_init(&vec);
    struct Snip snip = { .known=&mdr->snip, .know=&mdr->know, .vec=&vec };
    const struct MdrString *ret = expand(&snip, name, v);
    vector_release(&vec);
    if(!ret)
      return mdr_err;
  }
  return mdr_ok;
}
