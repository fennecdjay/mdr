struct Range {
  long ini;
  long end;
};

struct Range actual_range(char*, const struct Range*);


struct RangeIncluder {
  struct MdrString *str;
  struct Range range;
};

void string_append_range(struct MdrString *base, struct RangeIncluder *sr);


struct RangeExcluder {
  struct MdrString *end;
  struct MdrString *src;
};

struct MdrString* excluder_ini(struct RangeExcluder*, const struct Range*);
void excluder_end(struct RangeExcluder*, struct MdrString*);
