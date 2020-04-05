#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#define GETLINE_MINSIZE 16
#define RET_ERR(a) { errno = (a); return -1; }

static inline int getline(char **lineptr, size_t *n, FILE *fp) {
  int ch, i = 0;
  char free_on_err = 0, *p;
  errno = 0;
  if(!lineptr || !n || !fp)
    RET_ERR(EINVAL);
  if(!*lineptr) {
    *n = GETLINE_MINSIZE;
    *lineptr = (char *)malloc( sizeof(char) * (*n));
    if(*lineptr == NULL)
      RET_ERR(ENOMEM);
    free_on_err = 1;
  }
  for (i=0; ; i++) {
    ch = fgetc(fp);
    while (i >= (int)((*n) - 2)) {
      *n *= 2;
      if(!(p = realloc(*lineptr, sizeof(char) * (*n)))) {
        if (free_on_err)
          free(*lineptr);
        RET_ERR(ENOMEM);
      }
      *lineptr = p;
    }
    if(ch == EOF) {
      if(!i) {
        if(free_on_err)
          free(*lineptr);
        return -1;
      }
      (*lineptr)[i] = '\0';
      *n = i;
      return i;
    }
    if(ch == '\n') {
      (*lineptr)[i] = '\n';
      (*lineptr)[i+1] = '\0';
      *n = i+1;
      return i+1;
    }
    (*lineptr)[i] = (char)ch;
  }
}
