#ifndef _MAP_PRIVATE
#define _MAP_PRIVATE
#include "inttypes.h"
typedef uintptr_t vtype;
struct Vector_ {
  vtype* ptr;
};

struct Map_ {
  vtype* ptr;
};

#define SZ_INT sizeof(uintptr_t)
#define MAP_CAP 8
#define OFFSET 2
#define VLEN(v)    (v)->ptr[0]
#define VCAP(v)    (v)->ptr[1]
#define VPTR(v, i)    (v)->ptr[OFFSET + (i)]
#define VKEY(v, i) (v)->ptr[OFFSET + (i) * 2]
#define VVAL(v, i) (v)->ptr[OFFSET + (i) * 2 + 1]
#endif
#ifndef __VECTOR
#define __VECTOR

typedef struct Vector_ * Vector;

static inline vtype vector_front(const Vector v) {
  return VPTR(v, 0);
}
static inline vtype vector_at(const Vector v, const vtype i) {
  return VPTR(v, i);
}
static inline vtype vector_size(Vector const v) {
  return VLEN(v);
}

static inline void vector_realloc(const Vector v) {
  if((OFFSET + (VLEN(v) << 1) + 1) > VCAP(v))
    v->ptr = realloc(v->ptr, (VCAP(v) <<= 1) * SZ_INT);
}

static inline void vector_init(const Vector v) {
  v->ptr = calloc(MAP_CAP, SZ_INT);
  VCAP(v) = MAP_CAP;
}

static inline void vector_release(const Vector v) {
  free(v->ptr);
}

static inline void vector_add(const Vector v, const vtype data) {
  vector_realloc(v);
  VPTR(v, VLEN(v)++) = (vtype)data;
}

typedef struct Map_    * Map;
void map_init(const Map);
vtype map_get(const Map, const char*);

static inline vtype map_at(const Map map, const vtype index) {
  return VVAL(map, index);
}
void map_set(const Map, const vtype, const vtype);
void map_release(const Map);

static inline vtype map_size(const Map map) {
  return VLEN(map);
}
void map_release_vector(const Map map);
#endif
