#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "container.h"
#include "mdr_string.h"
#include "range.h"
#include "ast.h"
#include "know.h"
#include "mdr.h"
#include "lexer.h"

struct Parser {
  struct Lexer   *lex;
  Map snip;
  Map file;
  struct Ast *sec;
  struct Ast *ast;
  char *filename;
  int blk;
  short unsigned int start;
};

static inline void set_loc(struct Ast *ast, struct Parser *parser) {
  ast->loc.start = parser->start;
  ast->loc.end = parser->lex->line;
}

static inline void set_locnl(struct Ast *ast, struct Parser *parser) {
  ast->loc.start = parser->start;
  ast->loc.end = parser->lex->line - 1;
}

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
  ast->loc.filename = strdup(parser->filename);
  parser_add(parser, ast);
  return ast;
}

static struct Ast* parse(struct Parser*);

static enum mdr_status ast_str(struct Parser *parser) {
  struct Ast *ast = new_ast(parser, mdr_str);
  ast->info = lex_info(parser->lex);
  set_loc(ast, parser);
  return mdr_str;
}

static enum mdr_status ast_cmd(struct Parser *parser) {
  struct AstInfo info = lex_info(parser->lex);
  if(!info.str) // err_msg
    return mdr_err;
  struct Ast *ast = new_ast(parser, mdr_cmd);
  ast->info = info;
  set_loc(ast, parser);
  return mdr_cmd;
}

static enum mdr_status ast_blk(struct Parser *parser) {
  if(parser->blk)
    return mdr_end;
  struct AstInfo info = lex_info(parser->lex);
  if(!info.str) {
    struct Loc loc = { .start=parser->start, .end=parser->lex->line };
    return mdr_fail(&loc, "missing end block\n");
  }
  struct Parser new_parser = {
    .lex=parser->lex,
    .blk=1,
    .snip=parser->snip,
    .file=parser->file,
    .filename=parser->filename
  };
  struct Ast *section = parse(&new_parser);
  if(!section) {
    free_string(info.str);
    return mdr_err;
  }
  struct Ast *ast = new_ast(parser, mdr_blk);
  ast->info = info;
  ast->ast = section;
  set_loc(ast, parser);
  known_set(info.dot ? parser->file : parser->snip, info.str->str, ast);
  return mdr_blk;
}

static enum mdr_status ast_inc(struct Parser *parser) {
  struct Ast *ast = new_ast(parser, mdr_inc);
  ast->info = lex_info(parser->lex);
  set_loc(ast, parser);
  return mdr_inc; // err_msg
}

static enum mdr_status ast_end(struct Parser *parser __attribute__((unused))) {
  return mdr_end;
}

static enum mdr_status ast_err(struct Parser *parser __attribute__((unused))) {
  return mdr_err;
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
  ast_err
};

static enum mdr_status _parse(struct Parser *parser) {
  parser->start = parser->lex->line;
  enum mdr_status type = tokenize(parser->lex);
  return create_ast[type](parser);
}

void free_ast(struct Ast *ast) {
  free_string(ast->info.str);
  if(ast->next)
    free_ast(ast->next);
  if(ast->ast)
    free_ast(ast->ast);
  free(ast->loc.filename);
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
  struct Lexer lex = { .str = str, .line=1, .filename=mdr->name };
  struct Parser parser = {
    .lex=&lex,
    .snip=&mdr->snip,
    .file=&mdr->file,
    .filename=mdr->name
  };
  return parse(&parser);
}
