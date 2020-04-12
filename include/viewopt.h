struct ViewOpt {
  struct MdrString *blk_pre;
  struct MdrString *blk_ini;
  struct MdrString *blk_mid;
  struct MdrString *blk_end;
  struct MdrString *cmd_ini;
  struct MdrString *cmd_mid;
  struct MdrString *cmd_end;
};

void viewopt_fill(struct ViewOpt*);
void viewopt_release(struct ViewOpt*);
