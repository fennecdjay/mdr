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
  char *str;
  struct Ast *ast;
  struct Ast *next;
  int dot;
  enum mdr_status type;
};

void free_ast(struct Ast*);

struct Mdr {
  struct Map_ snip;
  struct Map_ file;
  struct Map_ done;
  const char *name;
};

enum mdr_status snip(struct Mdr *mdr);
enum mdr_status file(struct Mdr *mdr);
enum mdr_status view_ast(struct Mdr *mdr, struct Ast *ast);
enum mdr_status mdr_fail(const char *fmt, ...);

struct Ast* mdr_parse(struct Mdr *, char *const);

// util.c
char* filename2str(const char*);
char* cmd(const char *str);
int   cmd_file(FILE *file, const char *str);
vtype snippet_get(Map, const char*);
static inline void trimsz(char *str, size_t sz) {
  if(sz)
    str[sz - 1] = '\0';
}
static inline void trim(char *str) {
  trimsz(str, strlen(str));
}
static inline char* empty_string(void) {
  char* str = malloc(1);
  *str = '\0';
  return str;
}
FILE* mdr_open_write(const char *str);
#ifdef __MINGW32__
char *strndup(const char *s, size_t n) {
  char* c = malloc(n+1);
  if(c) {
    strncpy(c, s, n);
    c[n] = '\0';
  }
  return c;
}
#endif

// mdr.c
void mdr_init(struct Mdr*);
void mdr_run(struct Mdr*, struct Ast*);
char* done(Map map, const char *name);
void mdr_release(struct Mdr*);
