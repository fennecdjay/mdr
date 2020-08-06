struct MdrString* filename2str(const char*, const struct Loc*);
struct MdrString* cmd(const struct Ast*);
static inline void trimsz(char *str, size_t sz) {
  if(sz)
    str[sz - 1] = '\0';
}
static inline void trim(char *str) {
  trimsz(str, strlen(str));
}

FILE* mdr_open_write(const char*, const struct Loc*);
FILE* mdr_open_read(const char*, const struct Loc*);
void write_file(const char*, const struct Loc*, const struct MdrString *text);
