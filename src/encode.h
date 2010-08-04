#ifndef __MD_SIM_ENCODE__
#define __MD_SIM_ENCODE__

struct vm_program;
#define START_LABEL	"start:"
#define END_LABEL	"end:"

/**
 functions to convert into asm code
 */
 
/**
 VM_MD_CALL
 */
int encode_cd(struct vm_program *prg, char *dir);
int encode_mkdir(struct vm_program *prg, char *dir, int mode);
int encode_readdir(struct vm_program *prg, char *dir);
int encode_unlink(struct vm_program *prg, char *name);
int encode_open(struct vm_program *prg, char *name, int flags, int mode, int reg);
int encode_close(struct vm_program *prg, int reg);
int encode_stat(struct vm_program *prg, char *file);
int encode_chown(struct vm_program *prg, char *name, int uid, int gid);
int encode_chmod(struct vm_program *prg, char *name, int mode);
int encode_chtime(struct vm_program *prg, char *name, int time);
int encode_truncate(struct vm_program *prg, char *name, int size);
int encode_softlink(struct vm_program *prg, char *old, char *new);
int encode_hardlink(struct vm_program *prg, char *old, char *new);
int encode_readlink(struct vm_program *prg, char *name);


/* VM_SYS */
int encode_user(struct vm_program *prg, int uid);
int encode_group(struct vm_program *prg, int gid);
int encode_sleep(struct vm_program *prg, int ms);
int encode_race(struct vm_program *prg, int raceid);

/* specials */
int encode_loop_do(struct vm_program *prg);
int encode_loop_loop(struct vm_program *prg, int num);
int encode_expected(struct vm_program *prg, int val);

int procedure_start(char *name);
struct vm_program *procedure_current(void);
int procedure_finish(void);

#endif
