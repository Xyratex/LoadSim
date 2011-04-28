#ifndef __SIM_MD_CLI__
#define __SIM_MD_CLI__

struct simul_mnt;

int md_cli_init(struct simul_env *env, const struct simul_mnt *mnt);

void md_cli_fini(struct simul_env *env);

int md_cli_prerun(struct simul_env *env);

#endif
