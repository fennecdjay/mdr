struct Lexer {
  char             *str;
  struct AstInfo   info;
  size_t            idx;
  unsigned int      alt;
};

enum mdr_status tokenize(struct Lexer*);

struct AstInfo lex_info(struct Lexer *lex);
