struct Lexer {
  char             *str;
  char             *filename;
  struct AstInfo   info;
  size_t            idx;
  unsigned int      alt;
  unsigned int      line;
};

enum mdr_status tokenize(struct Lexer*);

struct AstInfo lex_info(struct Lexer *lex);
