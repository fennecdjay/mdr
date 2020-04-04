struct Lexer {
  char        *str;
  char        *tok;
  size_t       idx;
  unsigned int dot;
};

enum mdr_status tokenize(struct Lexer*);
char* snippet_name(struct Lexer*);
void lex_adv(struct Lexer*);
void lex_eol(struct Lexer*);
static inline unsigned int is_path(const char c) {
  return c == '.' || c == '/';
}
