#include <errno.h>

#include "vm_compile.h"
#include "vm_defs.h"

#include "debug.h"

int vm_program_init(struct vm_program **vprg)
{
	DPRINT("%s", "program created\n");
	return 0;
}

void vm_program_fini(struct vm_program *vprg)
{
	DPRINT("%s", "program destroyed\n");
}

int vm_program_check(struct vm_program *vprg)
{
	DPRINT("program check \n");
	return 0;
}

int vm_program_upload(struct vm_program *vprg)
{
	DPRINT("upload program into kernel\n");
	return 0;
}

typedef int (*enc_h_t)(struct vm_program *vprg, union cmd_arg data);

static int enc_pushs(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("pushs '%s'\n", data.cd_string);

	return 0;
}

static int enc_pushl(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("pushl %lu\n", data.cd_long);

	return 0;
}

static int enc_cmps(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("cmps\n");
	return 0;
}

static int enc_cmpl(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("cmpl\n");

	return 0;
}

static int enc_call(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("call [%d]\n", data.cd_call);

	return 0;
}

static int enc_goto(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("goto %s\n", data.cd_string);
	return 0;
}

static int enc_if(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("if\n");
	return 0;
}

static int enc_jz(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("jz %s\n", data.cd_string);

	return 0;
}

static int enc_add(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("add\n");
	return 0;
}

static int enc_sub(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("sub\n");
	return 0;
}

static enc_h_t en_helpers[VM_CMD_MAX] = {
	[VM_CMD_PUSHS]	= enc_pushs,
	[VM_CMD_PUSHL]	= enc_pushl,
	[VM_CMD_CMPS]	= enc_cmps,
	[VM_CMD_CMPL]	= enc_cmpl,
	[VM_CMD_CALL]	= enc_call,
	[VM_CMD_GOTO]	= enc_goto,
	[VM_CMD_JZ]	= enc_jz,
	[VM_CMD_IF]	= enc_if,
	[VM_CMD_ADD]	= enc_add,
	[VM_CMD_SUB]	= enc_sub
};

int vm_encode(struct vm_program *vprg, enum cmd_base cmd, union cmd_arg data)
{
	if (cmd > VM_CMD_MAX)
		return -EINVAL;

	return en_helpers[cmd](vprg, data);

	return 0;
}

void vm_label_resolve(struct vm_program *vprg, char *label_name)
{
	DPRINT("resolve label %s\n", label_name);
}
