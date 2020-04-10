#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "mdr.h"
#include "lexer.h"

struct Parser {
  struct Lexer   *lex;
  Map snip;
  Map file;
  struct Ast *sec; // ???
  struct Ast *ast;
  int blk;
};

static void known_set(const Map map, const char *key, struct Ast *ast) {
  const Vector exists = (Vector)map_get(map, key);
  if(exists) {
      vector_add(exists, (vtype)ast);
      return;
  }
  const Vector v = malloc(sizeof(*v));
  vector_init(v);
  vector_add(v, (vtype)ast);
  map_set(map, (vtype)key, (vtype)v);
}

static void parser_add(struct Parser *parser, struct Ast *ast) {
  if(parser->ast)
    parser->sec = (parser->sec->next = ast);
  else
    parser->sec = (parser->ast = ast);
}

static struct Ast* new_ast(struct Parser *parser, const enum mdr_status type) {
  struct Ast *ast = calloc(1, sizeof(struct Ast));
  ast->type = type;
  parser_add(parser, ast);
  return ast;
}

static struct Ast* parse(struct Parser*);

static enum mdr_status ast_str(struct Parser *parser) {
  struct Ast *ast = new_ast(parser, mdr_str);
  ast->str = parser->lex->tok;
  parser->lex->tok = NULL;
  return mdr_str;
}

static enum mdr_status ast_cmd(struct Parser *parser) {
  struct Ast *ast = new_ast(parser, mdr_cmd);
  ast->str = parser->lex->tok;
  ast->dot = parser->lex->dot;
  parser->lex->tok = NULL;
  parser->lex->dot = 0;
  return mdr_cmd;
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

static void get_lines(struct Lexer *lex, struct Range *r) {
  char *str = lex->str;
  if(!str)
    return;
  r->ini = strtol(str, NULL, 10);
  const char *end = get_end(str);
  if(end)
    r->end = strtol(end + 1, NULL, 10);
}

static enum mdr_status ast_blk(struct Parser *parser) {
  if(parser->blk)
    return mdr_end;
  struct MdrString *str = snippet_name(parser->lex);
  if(!str)
    return mdr_fail("unstarted block\n");
  struct Range e = {};
  get_lines(parser->lex, &e);
  lex_eol(parser->lex);
  const int dot = parser->lex->dot;
  struct Parser new_parser = { .lex=parser->lex, .blk=1, .snip=parser->snip, .file=parser->file };
  struct Ast *section = parse(&new_parser);
  if(!section) {
    free_string(str);
    return mdr_err;
  }
  known_set(dot ? parser->file : parser->snip, str->str, section);
  struct Ast *ast = new_ast(parser, mdr_blk);
  ast->str = str;
  ast->dot = dot;
  ast->ast = section;
  ast->ast->main = e;
  return mdr_blk;
}

static enum mdr_status ast_inc(struct Parser *parser) {
  struct Ast *section = new_ast(parser, mdr_inc);
  section->str = parser->lex->tok;
  parser->lex->tok = NULL;
  section->dot = parser->lex->dot;
  parser->lex->dot = 0; // what about idx ?
  get_lines(parser->lex, &section->self);
  while(*parser->lex->str != ']')
    ++parser->lex->str;
  ++parser->lex->str;
  if(*parser->lex->str != ']')
    return mdr_err; // msg
  ++parser->lex->str;
  return mdr_inc;
}

static enum mdr_status ast_end(struct Parser *parser __attribute__((unused))) {
  return mdr_end;
}

typedef enum mdr_status (*new_ast_fn)(struct Parser*);

static const new_ast_fn create_ast[] = {
  ast_str,
  ast_inc,
  ast_cmd,
  ast_blk,
  ast_end,
  ast_end,
  ast_end,
  ast_end
};

static enum mdr_status _parse(struct Parser *parser) {
  parser->lex->dot = 0;
  enum mdr_status type = tokenize(parser->lex);
  return create_ast[type](parser);
}

void free_ast(struct Ast *ast) {
  free_string(ast->str);
  if(ast->next)
    free_ast(ast->next);
  if(ast->ast)
    free_ast(ast->ast);
  free(ast);
}

static enum mdr_status parser_check(enum mdr_status type) {
  if(type == mdr_err || type == mdr_end)
    return type;
  return mdr_ok;
}

static struct Ast* parse(struct Parser *parser) {
  enum mdr_status type;
  enum mdr_status ret;
  do type = _parse(parser);
  while((ret = parser_check(type)) == mdr_ok);
  if(ret == mdr_err) {
    if(parser->ast)
      free_ast(parser->ast);
    return parser->ast = NULL;
  }
  return parser->ast;
}

struct Ast* mdr_parse(struct Mdr *mdr, char *const str) {
  struct Lexer lex = { .str = str };
  struct Parser parser = { .lex=&lex, .snip=&mdr->snip, .file=&mdr->file };
  return parse(&parser);
}
