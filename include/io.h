struct MdrString* filename2str(const char*);
struct MdrString* cmd(const char*);
int   cmd_file(FILE *file, const char *str);
static inline void trimsz(char *str, size_t sz) {
  if(sz)
    str[sz - 1] = '\0';
}
static inline void trim(char *str) {
  trimsz(str, strlen(str));
}

FILE* mdr_open_write(const char *str);
FILE* mdr_open_read(const char *str);
