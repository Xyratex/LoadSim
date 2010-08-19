#include <errno.h>
#include <string.h>

#include "encode.h"
#include "vm_defs.h"
#include "vm_compile.h"

/**
 convert test language commands into VM code
*/

/**
 cd := push "name"; call cd;
 */
int encode_cd(struct vm_program *vprg, char *dir)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = dir;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_CD;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_mkdir(struct vm_program *vprg, char *dir, int mode)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = mode;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_string = dir;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_MKDIR;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_readdir(struct vm_program *vprg, char *dir)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = dir;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_READIR;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_unlink(struct vm_program *vprg, char *name)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_UNLINK;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

/**
 pushl reg
 pushl flags
 pushs name
 call [open]
 */
int encode_open(struct vm_program *vprg, char *name, int flags, int mode, int reg)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = reg;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_long = mode;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_long = flags;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_OPEN;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_close(struct vm_program *vprg, int reg)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = reg;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_CLOSE;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_stat(struct vm_program *vprg, char *name)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_STAT;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_chown(struct vm_program *vprg, char *name, int uid, int gid)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_long = uid;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_long = gid;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_CHOWN;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_chmod(struct vm_program *vprg, char *name, int flags)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_long = flags;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_CHMOD;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_chtime(struct vm_program *vprg, char *name, int flags)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_long = flags;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_CHTIME;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_truncate(struct vm_program *vprg, char *name, int flags)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_long = flags;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_TRUNCATE;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_softlink(struct vm_program *vprg, char *old, char *new)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = old;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_string = new;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_SOFTLINK;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_hardlink(struct vm_program *vprg, char *old, char *new)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = old;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_string = new;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_HARDLINK;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_readlink(struct vm_program *vprg,char *name)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_READLINK;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_rename(struct vm_program *vprg, char *oldname, char *newname)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = newname;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_string = oldname;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_RENAME;
	return vm_encode(vprg, VM_CMD_CALL, arg);

}

/** **/
int encode_user(struct vm_program *vprg, int uid)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = uid;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_SYS_USER;
	return vm_encode(vprg, VM_CMD_CALL, arg);

}

int encode_group(struct vm_program *vprg, int gid)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = gid;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_SYS_GROUP;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_sleep(struct vm_program *vprg, int ms)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = ms;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_SYS_SLEEP;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_race(struct vm_program *vprg, int raceid)
{
	int ret;
	union cmd_arg arg;

	if (raceid > VM_MAX_RACES)
		return -EINVAL;

	arg.cd_long = raceid;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_SYS_RACE;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}


/****/
#define LOOP_ST_LABEL "loop_st%u:"
#define LOOP_END_LABEL "loop_end%u:"

static int loop_no = 0;
/**
 pushl $num
loop_st:
 dup 
 jz loop_end;
 */
int encode_loop_start(struct vm_program *vprg, int num)
{
	int ret;
	char label[20];
	union cmd_arg arg;

	arg.cd_long = num;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_ST_LABEL, loop_no);
	ret = vm_label_resolve(vprg, label);
	if (ret)
		return ret;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_DUP, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_END_LABEL, loop_no);
	arg.cd_string = label;
	return vm_encode(vprg, VM_CMD_JZ, arg);
}
/**
 pushl 1
 sub
 goto loop_st:
loop_end:
 */
int encode_loop_end(struct vm_program *vprg)
{
	int ret;
	union cmd_arg arg;
	char label[20];

	arg.cd_long = 1;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	/* arg non used */
	ret = vm_encode(vprg, VM_CMD_SUB, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_ST_LABEL, loop_no);
	arg.cd_string = label;
	ret = vm_encode(vprg, VM_CMD_GOTO, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_END_LABEL, loop_no);
	loop_no ++;

	return vm_label_resolve(vprg, label);
}

/**
 dup
 push $exp
 if
 jz exit
 pop (nop)
 */
int encode_expected(struct vm_program *vprg, int exp)
{
	int ret;
	union cmd_arg arg;

	/* dup need to read status on finish */
	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_DUP, arg);
	if (ret)
		return ret;

	arg.cd_long = exp;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_CMPL, arg);
	if (ret)
		return ret;

	arg.cd_string = END_LABEL;
	ret = vm_encode(vprg, VM_CMD_JNZ,arg);
	if (ret)
		return ret;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_NOP, arg);
	if (ret)
		return ret;

	return ret;
}

/**
 create tmp_name; mkdir ; cd;
 */
int encode_make_workdir(struct vm_program *prg)
{
	char name[256] = {0};
	int rc;

	strcpy(name, "workd-XXXXXXX");
	rc = mkstemp(name);
	if (rc < 0)
		return -EINVAL;

	rc = encode_mkdir(prg, name, 0777);
	if (rc < 0)
		return rc;

	rc = encode_expected(prg, VM_RET_OK);
	if (rc < 0)
		return rc;

	rc = encode_cd(prg, name);
	if (rc < 0)
		return rc;

	rc = encode_expected(prg, VM_RET_OK);
	if (rc < 0)
		return rc;

	return 0;
}


/******************/
static struct vm_program *vprg = NULL;

int procedure_start(char *name)
{
	int ret;

	ret = vm_program_init(&vprg, name);
	if (ret)
		return ret;

	ret = vm_label_resolve(vprg, START_LABEL);

	return ret;
}

struct vm_program *procedure_current(void)
{
	return vprg;
}

int procedure_end(void)
{
	int ret;
	union cmd_arg arg;

	/* program return code if finished ok */
	arg.cd_long = 0;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	ret = vm_label_resolve(vprg, END_LABEL);
	if (ret)
		return ret;

	ret = vm_program_check(vprg);

	return ret;
}
