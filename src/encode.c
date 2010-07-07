#include <errno.h>
#include <string.h>

#include "encode.h"
#include "vm_defs.h"
#include "vm_compile.h"

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

	arg.cd_call = VM_CALL_CD;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_mkdir(struct vm_program *vprg,char *dir)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = dir;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_CALL_MKDIR;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_readdir(struct vm_program *vprg,char *dir)
{
	int ret;
	union cmd_arg arg;

	arg.cd_string = dir;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_CALL_READIR;
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

	arg.cd_call = VM_CALL_UNLINK;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

/**
 pushl reg
 pushl flags
 pushs name
 call [open]
 */
int encode_open(struct vm_program *vprg, char *name, int flags, int reg)
{
	int ret;
	union cmd_arg arg;

	arg.cd_long = reg;
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

	arg.cd_call = VM_CALL_OPEN;
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

	arg.cd_call = VM_CALL_CLOSE;
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

	arg.cd_call = VM_CALL_STAT;
	return vm_encode(vprg, VM_CMD_CALL, arg);
}

int encode_setattr(struct vm_program *vprg, char *name, int flags)
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

	arg.cd_call = VM_CALL_SETATTR;
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

	arg.cd_call = VM_CALL_SOFTLINK;
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

	arg.cd_call = VM_CALL_SOFTLINK;
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

	arg.cd_call = VM_CALL_READLINK;
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
	ret = vm_encode(vprg, VM_CMD_JZ, arg);
	if (ret)
		return ret;
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

	loop_no ++;

	arg.cd_long = 1;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	/* arg non used */
	ret = vm_encode(vprg, VM_CMD_SUB, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_ST_LABEL, loop_no);
	vm_label_resolve(vprg, label);

	arg.cd_string = label;
	ret = vm_encode(vprg, VM_CMD_GOTO, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_END_LABEL, loop_no);
	vm_label_resolve(vprg, label);

	return 0;
}

/**
 dup
 push $exp
 if
 jz exit
 pop ??
 */
int encode_expected(struct vm_program *vprg, int exp)
{
	int ret;
	union cmd_arg arg;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_DUP, arg);
	if (ret)
		return ret;

	arg.cd_long = exp;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_string = END_LABEL;
	ret = vm_encode(vprg, VM_CMD_JZ,arg);
	if (ret)
		return ret;

	/** XXX need pop */
	return ret;
}


/******************/
static struct vm_program *vprg = NULL;

int procedure_start(char *name)
{
	int ret;

	ret = vm_program_init(&vprg);
	if (ret)
		return ret;

	vprg->vmp_name = strdup(name);
	if (vprg->vmp_name == NULL) {
		vm_program_fini(vprg);
		return -ENOMEM;
	}

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

	ret = vm_label_resolve(vprg, END_LABEL);
	if (ret)
		return ret;

	ret = vm_program_check(vprg);


	return ret;
}
