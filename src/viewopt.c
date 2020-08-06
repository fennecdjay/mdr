#include <stdlib.h>
#include <string.h>
#include "mdr_string.h"
#include "viewopt.h"

static const char* view_opt[][2] = {
  { "MDR_BLK_PRE", "  > " },
  { "MDR_BLK_INI", "\n``` " },
  { "MDR_BLK_MID", "  \n" },
  { "MDR_BLK_END", "```  " },
  { "MDR_CMD_INI", "  > " },
  { "MDR_CMD_MID", "  \n```\n" },
  { "MDR_CMD_INI", "```  \n" },
};

static inline struct MdrString* viewopt_get(const size_t idx) {
  const char *str = getenv(view_opt[idx][0]) ?: view_opt[idx][1];
  return new_string(str, strlen(str));
}

void viewopt_fill(struct ViewOpt *vo) {
  vo->blk_pre = viewopt_get(0);
  vo->blk_ini = viewopt_get(1);
  vo->blk_mid = viewopt_get(2);
  vo->blk_end = viewopt_get(3);
  vo->cmd_ini = viewopt_get(4);
  vo->cmd_mid = viewopt_get(5);
  vo->cmd_end = viewopt_get(6);
}

void viewopt_release(struct ViewOpt *vo) {
  free_string(vo->blk_pre);
  free_string(vo->blk_ini);
  free_string(vo->blk_mid);
  free_string(vo->blk_end);
  free_string(vo->cmd_ini);
  free_string(vo->cmd_mid);
  free_string(vo->cmd_end);
}
