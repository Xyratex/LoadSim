#include "vm_compile.h"
#include "vm_defs.h"

int vm_program_init(struct vm_program **vprg)
{
	return 0;
}

void vm_program_fini(struct vm_program *vprg)
{

}

int vm_program_check(struct vm_program *vprg)
{

	return 0;
}

int vm_program_upload(struct vm_program *vprg)
{
	return 0;
}

typedef int (*enc_h_t)(char *start, void *data);

static int enc_pushs(char *start, void *data)
{
	return 0;
}

static int enc_pushl(char *start, void *data)
{
	return 0;
}

static int enc_cmps(char *start, void *data)
{
	return 0;
}

static int enc_cmpl(char *start, void *data)
{
	return 0;
}

static int enc_call(char *start, void *data)
{
	return 0;
}

static int enc_goto(char *start, void *data)
{
	return 0;
}

static int enc_if(char *start, void *data)
{
	return 0;
}

static int enc_add(char *start, void *data)
{
	return 0;
}

static int enc_sub(char *start, void *data)
{
	return 0;
}

static enc_h_t en_helpers[VM_CMD_MAX] = {
	[VM_CMD_PUSHS]	= enc_pushs,
	[VM_CMD_PUSHL]	= enc_pushl,
	[VM_CMD_CMPS]	= enc_cmps,
	[VM_CMD_CMPL]	= enc_cmpl,
	[VM_CMD_CALL]	= enc_call,
	[VM_CMD_GOTO]	= enc_goto,
	[VM_CMD_IF]	= enc_if,
	[VM_CMD_ADD]	= enc_add,
	[VM_CMD_SUB]	= enc_sub
};

int vm_encode(struct vm_program *vprg, enum cmd_base cmd, void *data)
{
	return 0;
}

void vm_label_resolve(struct vm_program *vprg, char *label_name)
{

}
