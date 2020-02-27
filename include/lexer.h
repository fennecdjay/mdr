struct Lexer {
  char   *str;
  char   *tok;
  size_t idx;
  int    dot;
};
enum mdr_status tokenize(struct Lexer*);
char* snippet_name(struct Lexer*);
void lex_adv(struct Lexer*);
void lex_eol(struct Lexer*);
