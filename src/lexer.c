#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"
#include "lexer.h"

static inline void lex_nl(struct Lexer *lex) {
  if(*lex->str == '\n')
    ++lex->line;
}

static inline unsigned int lex_end(struct Lexer *lex) {
  return *lex->str == '\0';
}

static void lex_adv(struct Lexer *lex) {
  lex_nl(lex);
  ++lex->str;
  ++lex->idx;
}

static int lex_eol(struct Lexer *lex) {
  int escape = 0;
  while(!lex_end(lex) && *lex->str != '\n') {
    escape = (*lex->str == '\\');
    lex_adv(lex);
  }
  lex_adv(lex);
  return escape;
}

static inline unsigned int is_path(const char c) {
  return c == '.' || c == '/';
}

static unsigned int lex_is_path(struct Lexer *lex) {
  if(!is_path(*lex->str))
    return 0;
  return lex->info.dot = 1;
}

static unsigned int path(struct Lexer *lex) {
  while(!lex_end(lex) && (isalnum(*lex->str) ||
      *lex->str == '_' || *lex->str == '-' || lex_is_path(lex)))
    lex_adv(lex);
  return 1;
}

static inline void eat_space(struct Lexer *lex) {
  while(*lex->str != '\n' && isspace(*lex->str))
    ++lex->str;
}

static struct MdrString* snippet_name(struct Lexer *lex) {
  eat_space(lex);
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

static inline unsigned int is_inc_end(const char *str) {
  return str[0] == ']' &&
         str[1] == ']';
}

static inline unsigned int is_cmd(const char *str) {
  return !strncmp(str, "exec", 4);
}

static inline unsigned int is_hide(const char *str) {
  return !strncmp(str, "hide", 4);
}

static enum mdr_status _comment(struct Lexer *lex) {
  lex->str += 2;
  lex->info.str = new_string("@", 1);
  return mdr_str;
}

static char* get_end(char *str) {
  while(*str) {
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
  lex->info.range.ini = lex->info.range.end = 0;
  lex->info.range.ini = strtol(str, &lex->str, 10);
  const char *end = get_end(str);
  if(end)
    lex->info.range.end = strtol(end + 1, &lex->str, 10);
}

static enum mdr_status _inc(struct Lexer *lex) {
  lex->str += 3;
  if(!(lex->info.str = snippet_name(lex))) {
    struct Loc loc = { .start=lex->line, .end=lex->line, .filename=lex->filename };
    return mdr_fail(&loc, "missing include name\n");
  }
  eat_space(lex);
  lex_range(lex);
  eat_space(lex);
  if(!is_inc_end(lex->str)) {
    free_string(lex->info.str);
    return mdr_err;
  }
  lex_adv(lex);
  lex_adv(lex);
  return mdr_inc;
}

static enum mdr_status _blk(struct Lexer *lex) {
  lex->alt = !lex->alt;
  lex->str += 4;
  if(lex->alt) {
    if(!(lex->info.str = snippet_name(lex))) {
      struct Loc loc = { .start=lex->line, .end = lex->line, .filename=lex->filename };
      return mdr_fail(&loc, "unstarted block\n");
    }
    eat_space(lex);
    lex_range(lex);
    lex_eol(lex);
  }
  return mdr_blk;
}

static void escape_nl(struct Lexer *lex, char * buf) {
  int escape = 1;
  do {
    const int base = lex->idx;
    escape = lex_eol(lex);
    struct MdrString str = { buf + base, lex->idx - base - escape};
    string_append(lex->info.str, &str);
  }
  while(escape);
}

static enum mdr_status _cmd(struct Lexer *lex) {
  lex->str += 5;
  eat_space(lex);
  char *const buf = lex->str;
  int escape = lex_eol(lex);
  if(lex->idx == 1) {
    struct Loc loc = { .start=lex->line, .end = lex->line, .filename=lex->filename };
    return mdr_fail(&loc, "missing exec command\n");
  }
  lex->info.str = new_string(buf, lex->idx - 1 - escape);
  if(escape)
    escape_nl(lex, buf);
  return mdr_cmd;
}

static enum mdr_status _hide(struct Lexer *lex) {
  lex->info.dot = 1;
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
  lex->info.str = new_string("@", 1);
  ++lex->str;
  return mdr_str;
}

enum mdr_status tokenize(struct Lexer *lex) {
  char *const buf = lex->str;
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
  lex->info.str = new_string(buf, lex->idx);
  return mdr_str;
}

struct AstInfo lex_info(struct Lexer *lex) {
  struct AstInfo info = lex->info;
  lex->info.str = NULL;
  lex->info.range.ini = lex->info.range.end = 0;
  lex->info.dot = 0;
  lex->idx = 0;
  return info;
}
