struct MdrString {
  char *str;
  size_t sz;
};

struct MdrString* new_string(const char *, const size_t);
void free_string(struct MdrString *a);
void string_append(struct MdrString *old, struct MdrString *new);
