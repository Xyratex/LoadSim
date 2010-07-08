#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> /* htonl */

#include "vm_compile.h"
#include "vm_defs.h"

#include "debug.h"

static LIST_HEAD(vm_programs);

int vm_program_init(struct vm_program **vprg, char *name)
{
	struct vm_program *ret;

	ret = malloc(sizeof *ret);
	if (ret == NULL)
		return -ENOMEM;

	ret->vmp_data = malloc(VM_INIT_PROG_SIZE);
	if (ret->vmp_data == NULL) {
		free(ret);
		return -ENOMEM;
	}

	ret->vmp_name = strdup(name);
	if (ret->vmp_name == NULL) {
		free(ret->vmp_data);
		free(ret);
		return -ENOMEM;
	}

	ret->vmp_size = VM_INIT_PROG_SIZE;
	ret->vmp_enc_idx = 0;
	INIT_LIST_HEAD(&ret->vmp_labels);
	list_add(&ret->vmp_link, &vm_programs);

	*vprg = ret;
	DPRINT("program '%s' created\n", name);
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

/******************/
typedef int (*enc_h_t)(struct vm_program *vprg, union cmd_arg data);

static int add_byte_to_buffer(struct vm_program *vprg, char byte)
{
	if (vprg->vmp_size <= vprg->vmp_enc_idx) {
		/* XXX resize */
		return -ENOMEM;
	}

	vprg->vmp_data[vprg->vmp_enc_idx] = byte;
	vprg->vmp_enc_idx ++;

	return 0;
}

static int add_long_to_buffer(struct vm_program *vprg, long ldata)
{
	long l = htonl(ldata);

	if (add_byte_to_buffer(vprg, l & 0xFF))
		return -ENOMEM;

	if (add_byte_to_buffer(vprg, (l >> 8) & 0xFF))
		return -ENOMEM;

	if (add_byte_to_buffer(vprg, (l >> 16) & 0xFF))
		return -ENOMEM;

	if (add_byte_to_buffer(vprg, (l >> 24) & 0xFF))
		return -ENOMEM;

	return 0;
}

static add_string_to_buffer(struct vm_program *vprg, char *str)
{
	int i;
	int len;

	len = strlen(str);
	for(i = 0; i < len; i++) {
		if (add_byte_to_buffer(vprg, str[i]))
			return -ENOMEM;
	}
	return 0;
}

static int enc_pushs(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("pushs '%s'\n", data.cd_string);

	return add_string_to_buffer(vprg, data.cd_string);
}

static int enc_pushl(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("pushl %lu\n", data.cd_long);

	return add_long_to_buffer(vprg, data.cd_long);
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

	return add_long_to_buffer(vprg, data.cd_call);
}

static int enc_goto(struct vm_program *vprg, union cmd_arg data)
{
	long address;
	DPRINT("goto %s\n", data.cd_string);

	/** XXX find label or create waiting link */

	return add_long_to_buffer(vprg, address);
}

static int enc_jz(struct vm_program *vprg, union cmd_arg data)
{
	long address;
	DPRINT("jz %s\n", data.cd_string);

	return add_long_to_buffer(vprg, address);
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

static int enc_dup(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("dup\n");
	return 0;
}

static int enc_nop(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("nop\n");
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
	[VM_CMD_ADD]	= enc_add,
	[VM_CMD_SUB]	= enc_sub,
	[VM_CMD_DUP]	= enc_dup,
	[VM_CMD_NOP]	= enc_nop,
};

int vm_encode(struct vm_program *vprg, enum cmd_base cmd, union cmd_arg data)
{
	int rc;

	if (cmd > VM_CMD_MAX)
		return -EINVAL;

	rc = add_byte_to_buffer(vprg, cmd);
	if (rc)
		return rc;

	return en_helpers[cmd](vprg, data);
}

int vm_label_resolve(struct vm_program *vprg, char *label_name)
{
	DPRINT("resolve label %s\n", label_name);

	return 0;
}
