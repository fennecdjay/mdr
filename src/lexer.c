#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "container.h"
#include "mdr.h"
#include "lexer.h"

static inline unsigned int lex_end(struct Lexer *lex) {
  return *lex->str == '\0';
}

void lex_adv(struct Lexer *lex) {
  ++lex->str;
  ++lex->idx;
}

void lex_eol(struct Lexer *lex) {
  while(!lex_end(lex) && *lex->str != '\n')
    lex_adv(lex);
  lex_adv(lex);
}

static inline void eat_space(struct Lexer *lex) {
  while(*lex->str != '\n' && isspace(*lex->str))
    ++lex->str;
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

enum mdr_status _comment(struct Lexer *lex) {
  lex->str += 2;
  lex->tok = strdup("@");
  return mdr_str;
}

enum mdr_status _inc(struct Lexer *lex) {
  lex->str += 3;
  if(!(lex->tok = snippet_name(lex)))
    return mdr_fail("missing include name\n");
  eat_space(lex);
  return mdr_inc;
}

enum mdr_status _blk(struct Lexer *lex) {
  lex->str += 4;
  return mdr_blk;
}

enum mdr_status _cmd(struct Lexer *lex) {
  lex->str += 5;
  eat_space(lex);
  char *const buf = lex->str;
  lex_eol(lex);
  if(lex->idx == 1)
    return mdr_fail("missing exec command\n");
  lex->tok = strndup(buf, lex->idx - 1);
  return mdr_cmd;
}

enum mdr_status mdr_command(struct Lexer *lex) {
  char *const str = lex->str + 1;
  if(is_comment(str))
    return _comment(lex);
  if(is_blk(str))
    return _blk(lex);
  if(is_inc(str))
    return _inc(lex);
  if(is_cmd(str))
    return _cmd(lex);
  lex->tok = strdup("@");
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
  lex->tok = strndup(buf, lex->idx);
  return mdr_str;
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

char* snippet_name(struct Lexer *lex) {
  eat_space(lex);
  lex->idx = 0;
  char *const buf = lex->str;
  path(lex);
  char *ret = lex->idx ? strndup(buf, lex->idx) : NULL;
  eat_space(lex);
  return ret;
}
