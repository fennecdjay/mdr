enum mdr_status {
  mdr_str,
  mdr_inc,
  mdr_cmd,
  mdr_blk,
  mdr_end,
  mdr_eof,
  mdr_ok,
  mdr_err
};

struct Loc {
  char *filename;
  short unsigned int start;
  short unsigned int end;
};

struct AstInfo {
  struct MdrString *str;
  struct Range range;
  int dot;
};

struct Ast {
  struct AstInfo info;
  struct Ast *ast;
  struct Ast *next;
  struct Loc loc;
  enum mdr_status type;
};

void free_ast(struct Ast*);
