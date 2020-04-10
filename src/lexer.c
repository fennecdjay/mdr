#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"
#include "lexer.h"

static inline unsigned int lex_end(struct Lexer *lex) {
  return *lex->str == '\0';
}

static void lex_adv(struct Lexer *lex) {
  ++lex->str;
  ++lex->idx;
}

static void lex_eol(struct Lexer *lex) {
  while(!lex_end(lex) && *lex->str != '\n')
    lex_adv(lex);
  lex_adv(lex);
}

static inline unsigned int is_path(const char c) {
  return c == '.' || c == '/';
}

static unsigned int lex_is_path(struct Lexer *lex) {
  if(!is_path(*lex->str))
    return 0;
  return lex->dot = 1;
}

static unsigned int path(struct Lexer *lex) {
  while(!lex_end(lex) && (isalnum(*lex->str) || *lex->str == '_' || lex_is_path(lex)))
    lex_adv(lex);
  return 1;
}

static inline void eat_space(struct Lexer *lex) {
  while(*lex->str != '\n' && isspace(*lex->str))
    ++lex->str;
}

static struct MdrString* snippet_name(struct Lexer *lex) {
  eat_space(lex);
  lex->idx = 0;
  char *const buf = lex->str;
  path(lex);
  if(!lex->idx)
    return NULL;
  struct MdrString *ret = new_string(buf, lex->idx);
  eat_space(lex);
  return ret;
}

static inline unsigned int is_comment(const char *str) {
  return str[0] == '@';
}

static inline unsigned int is_blk(const char *str) {
  return str[0] == '`' &&
         str[1] == '`' &&
         str[2] == '`';
}

static inline unsigned int is_inc(const char *str) {
  return str[0] == '[' &&
         str[1] == '[';
}

static inline unsigned int is_cmd(const char *str) {
  return !strncmp(str, "exec", 4);
}

static inline unsigned int is_hide(const char *str) {
  return !strncmp(str, "hide", 4);
}

static enum mdr_status _comment(struct Lexer *lex) {
  lex->str += 2;
  lex->tok = new_string("@", 1);
  return mdr_str;
}

static char* get_end(char *str) {
  while(str) {
    if(*str == ':')
      return str;
    if(*str == '\n')
      break;
    ++str;
  }
  return NULL;
}

static void lex_range(struct Lexer *lex) {
  char *str = lex->str;
  if(!str)
    return;
  lex->rng.ini = lex->rng.end = 0;
  lex->rng.ini = strtol(str, &lex->str, 10);
  const char *end = get_end(str);
  if(end)
    lex->rng.end = strtol(end + 1, &lex->str, 10);
}

static enum mdr_status _inc(struct Lexer *lex) {
  lex->str += 3;
  if(!(lex->tok = snippet_name(lex)))
    return mdr_fail("missing include name\n");
  eat_space(lex);
  lex_range(lex);
  eat_space(lex);
  return mdr_inc;
}

static enum mdr_status _blk(struct Lexer *lex) {
  lex->alt = !lex->alt;
  lex->str += 4;
  if(lex->alt) {
    if(!(lex->tok = snippet_name(lex)))
      return mdr_fail("unstarted block\n");
    eat_space(lex);
    lex_range(lex);
    lex_eol(lex);
  }
  return mdr_blk;
}

static enum mdr_status _cmd(struct Lexer *lex) {
  lex->str += 5;
  eat_space(lex);
  char *const buf = lex->str;
  lex_eol(lex);
  if(lex->idx == 1)
    return mdr_fail("missing exec command\n");
  lex->tok = new_string(buf, lex->idx - 1);
  return mdr_cmd;
}

static enum mdr_status _hide(struct Lexer *lex) {
  lex->dot = 1;
  _cmd(lex);
  return mdr_cmd;
}

static enum mdr_status mdr_command(struct Lexer *lex) {
  char *const str = lex->str + 1;
  if(is_comment(str))
    return _comment(lex);
  if(is_blk(str))
    return _blk(lex);
  if(is_inc(str))
    return _inc(lex);
  if(is_cmd(str))
    return _cmd(lex);
  if(is_hide(str))
    return _hide(lex);
  lex->tok = new_string("@", 1);
  ++lex->str;
  return mdr_str;
}

enum mdr_status tokenize(struct Lexer *lex) {
  char *const buf = lex->str;
  lex->idx = 0;
  char c;
  while((c = *lex->str)) {
    if(c == '@') {
      if(lex->idx)
        break;
      return mdr_command(lex);
    }
    lex_adv(lex);
  }
  if(!lex->idx)
    return mdr_end;
  lex->tok = new_string(buf, lex->idx);
  return mdr_str;
}
