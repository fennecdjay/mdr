struct Lexer {
  char             *str;
  struct MdrString *tok;
  struct Range      rng;
  size_t            idx;
  unsigned int      dot;
  unsigned int      alt;
};

enum mdr_status tokenize(struct Lexer*);
