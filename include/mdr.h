#ifdef __AFL_HAVE_MANUAL_CONTROL
#define fprintf(a,b, ...)
#define fopen(a,b) (FILE*)1
#define fclose(a)
#endif

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

struct Ast {
  struct MdrString *str;
  struct Ast *ast;
  struct Ast *next;
  struct Range range;
  int dot;
  enum mdr_status type;
};

void free_ast(struct Ast*);

struct Know {
  struct Map_ curr;
  Map         global;
};

struct Mdr {
  struct Map_ snip;
  struct Map_ file;
  struct Map_ file_done;
  struct Know know;
  const char *name;
};

enum mdr_status snip(struct Mdr *mdr);
enum mdr_status file(struct Mdr *mdr);
enum mdr_status view_ast(struct Mdr *mdr, struct Ast *ast);
enum mdr_status mdr_fail(const char *fmt, ...);

struct Ast* mdr_parse(struct Mdr *, char *const);

// util.c
struct MdrString* filename2str(const char*);
struct MdrString* cmd(const char*);
int   cmd_file(FILE *file, const char *str);
vtype snippet_get(struct Know*, const char*);
static inline void trimsz(char *str, size_t sz) {
  if(sz)
    str[sz - 1] = '\0';
}
static inline void trim(char *str) {
  trimsz(str, strlen(str));
}

FILE* mdr_open_write(const char *str);
FILE* mdr_open_read(const char *str);
enum mdr_status mdr_cpy(FILE *tgt, const char* name);

// mdr.c
void mdr_init(struct Mdr*);
void mdr_run(struct Mdr*, struct Ast*);
char* done(struct Know *know, const char *name);
void mdr_release(struct Mdr*);
