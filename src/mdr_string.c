#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"

struct MdrString* new_string(const char *str, const size_t sz) {
  struct MdrString *a = malloc(sizeof(struct MdrString));
  a->str = malloc(sz + 1);
  strncpy(a->str, str, sz);
  a->str[sz] = '\0';
  a->sz = sz;
  return a;
}

void free_string(struct MdrString *a) {
  free(a->str);
  free(a);
}

void string_append(struct MdrString *old, struct MdrString *new) {
  if(!new->sz)
    return;
  const size_t len = old->sz + new->sz + 1;
  char *str = realloc(old->str, len);
  strcpy(str + old->sz, new->str);
  old->sz += new->sz;
  old->str = str;
}
