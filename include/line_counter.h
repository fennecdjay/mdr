struct LineCounter {
  char *str;
  long sz;
  long count;
};

static inline void linecounter_run(struct LineCounter* lc, const long n) {
  while(*lc->str && lc->count < n) {
    if(*lc->str == '\n')
      ++lc->count;
    ++lc->sz;
    ++lc->str;
  }
}
