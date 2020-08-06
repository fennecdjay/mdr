#ifdef __AFL_HAVE_MANUAL_CONTROL
#define fprintf(a,b, ...)
#define fopen(a,b) (FILE*)1
#define fclose(a)
#endif

struct Mdr {
  struct Map_ snip;
  struct Map_ file;
  struct Know know;
  struct ViewOpt *vopt;
  char *name;
};

enum mdr_status snip(struct Mdr *mdr);
enum mdr_status file(struct Mdr *mdr);
enum mdr_status view_ast(struct Mdr *mdr, struct Ast *ast);
struct Ast* mdr_parse(struct Mdr *, char *const);
enum mdr_status mdr_fail(const struct Loc *loc, const char *fmt, ...);

// mdr.c
void mdr_init(struct Mdr*);
void mdr_run(struct Mdr*, struct Ast*);
void mdr_release(struct Mdr*);
