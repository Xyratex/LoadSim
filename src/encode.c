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

	return ret;
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

int encode_open(struct vm_program *vprg, char *name, int flags, int reg)
{
	int ret;

	return ret;
}

int encode_close(struct vm_program *vprg, int reg)
{
	int ret;

	return ret;
}

int encode_stat(struct vm_program *vprg, char *file)
{
	int ret;

	return ret;
}

int encode_setattr(struct vm_program *vprg, char *name, int flags)
{
	int ret;

	return ret;
}

int encode_softlink(struct vm_program *vprg, char *old, char *new)
{
	int ret;

	return ret;
}

int encode_hardlink(struct vm_program *vprg, char *old, char *new)
{
	int ret;

	return ret;
}

int encode_readlink(struct vm_program *vprg,char *name)
{
	int ret;

	return ret;
}

/****/
#define LOOP_ST_LABEL "loop_st%u:"
#define LOOP_END_LABEL "loop_end%u"

static int loop_no = 0;
/**
 pushl $num
loop_st:
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
	vm_label_resolve(vprg, label);


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
 push $exp
 if
 jz exit
 */
int encode_expected(struct vm_program *vprg, int exp)
{
	int ret;
	union cmd_arg arg;

	loop_no ++;

	arg.cd_long = exp;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_IF, arg);
	if (ret)
		return ret;

	arg.cd_string = END_LABEL;
	ret = vm_encode(vprg, VM_CMD_JZ,arg);
	if (ret)
		return ret;

	/** XXX need pop */
	return ret;
}