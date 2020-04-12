#include <stdlib.h>
#include <string.h>
#include <container.h>
#include <mdr_string.h>

void map_init(const Map a) {
  a->ptr = calloc(MAP_CAP, SZ_INT);
  VCAP(a) = MAP_CAP;
}

vtype map_get(const Map map, const char *key) {
  for(vtype i = VLEN(map) + 1; --i;)
    if(!strcmp(key, (char*)VKEY(map, i - 1)))
      return VVAL(map, i - 1);
  return 0;
}

void map_set(const Map map, const vtype key, const vtype ptr) {
  vector_realloc((Vector)map);
  VKEY(map, VLEN(map)) = key;
  VVAL(map, VLEN(map)++) = ptr;
}

void map_release(const Map map) {
  free(map->ptr);
}

void map_release_vector(const Map map) {
  for(vtype i = 0; i < map_size(map); ++i) {
    vector_release((Vector)VVAL(map, i));
    free((Vector)VVAL(map, i));
  }
  map_release(map);
}

void map_release_string(Map map) {
  for(vtype i = 0; i < map_size(map); ++i) {
    free((char*)VKEY(map, i));
    free_string((struct MdrString*)VVAL(map, i));
  }
  map_release(map);
}
