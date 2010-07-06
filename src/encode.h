#ifndef __MD_SIM_ENCODE__
#define __MD_SIM_ENCODE__

struct vm_program;
#define START_LABEL	"start:"
#define END_LABEL	"end:"

int encode_cd(struct vm_program *prg, char *dir);
int encode_mkdir(struct vm_program *prg, char *dir);
int encode_readdir(struct vm_program *prg, char *dir);
int encode_unlink(struct vm_program *prg, char *name);
int encode_open(struct vm_program *prg, char *name, int flags, int reg);
int encode_close(struct vm_program *prg, int reg);
int encode_stat(struct vm_program *prg, char *file);
int encode_setattr(struct vm_program *prg, char *name, int flags);
int encode_softlink(struct vm_program *prg, char *old, char *new);
int encode_hardlink(struct vm_program *prg, char *old, char *new);
int encode_readlink(struct vm_program *prg, char *name);

int encode_loop_do(struct vm_program *prg);
int encode_loop_loop(struct vm_program *prg, int num);
int encode_expected(struct vm_program *prg, int val);


#endif