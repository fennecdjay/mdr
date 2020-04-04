static inline ssize_t filerange_read(struct FileRange *fr) {
  return (fr->nread = getline(&fr->line, &fr->len, fr->stream)) != -1;
}

void range_include(struct StrRange *sr);

enum mdr_status filerange_init(struct FileRange *fr, const char *str, struct Range *range);

static inline void filerange_write(struct FileRange *fr, FILE *file) {
  fwrite(fr->line, fr->nread, 1, file);
}

static inline int filerange_before(struct FileRange *fr) {
  return filerange_read(fr) && ++fr->count < fr->range.ini;
}

static inline int filerange_while(struct FileRange *fr) {
  return filerange_read(fr) && ++fr->count <= fr->range.end;
}

static inline void filerange_release(struct FileRange *fr) {
  free(fr->line);
  fclose(fr->stream);
}
