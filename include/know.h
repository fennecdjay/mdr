struct Know {
  struct Map_ curr;
  Map         global;
  struct Map_ file_done;
};

struct MdrString* snippet_find(struct Know*, const struct MdrString*);
void snippet_set(struct Know *, const char*, const struct MdrString*);

void file_set(struct Know *, const struct Ast *ast, const struct MdrString*);

struct MdrString* know_get(struct Know*, const struct Ast*);
void know_release(struct Know*);
