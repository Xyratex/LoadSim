#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "vm_defs.h"
#include "vm_compile.h"

#include "debug.h"

enum {
	VM_LABEL_NEW = -1
};

/**
 try to find label by name or allocate new
 */
static struct vm_label *vm_label_find(struct vm_program *vprg, char *name)
{
	struct vm_label *ret;
	struct vm_label_wait *w;

	list_for_each_entry(ret, &vprg->vmp_labels, vl_link) {
		if (strcasecmp(ret->vl_name, name) == 0) {
			/* forward reference to label.
			   create new wait reference.
			 */
			if (ret->vl_addr == VM_LABEL_NEW) {
				w = malloc(sizeof *w);
				if (w == NULL)
					return NULL;
				w->vlw_addr = (long *)&vprg->vmp_data[vprg->vmp_enc_idx];
				list_add(&w->vlw_link, &ret->vl_waits);
			}
			return ret;
		}
	}

	ret = malloc(sizeof *ret);
	if (ret == NULL)
		return NULL;

	ret->vl_name = strdup(name);
	if (ret->vl_name == NULL) {
		free(ret);
		return NULL;
	}
	ret->vl_addr = VM_LABEL_NEW;
	INIT_LIST_HEAD(&ret->vl_waits);

	list_add(&ret->vl_link, &vprg->vmp_labels);

	return ret;
}

int vm_label_resolve(struct vm_program *vprg, char *label_name)
{
	struct vm_label *label;
	struct vm_label_wait *tmp;
	struct vm_label_wait *w;

	DPRINT("resolve label %s\n", label_name);

	label = vm_label_find(vprg, label_name);
	if (label == NULL)
		return -ENOMEM;

	/* second label with same name ?... */
	if (label->vl_addr != VM_LABEL_NEW) {
		err_print("label %s - already exist ?\n", label_name);
		return -EINVAL;
	}

	/* resolve forward reference */
	label->vl_addr = vprg->vmp_enc_idx;
	list_for_each_entry_safe(w, tmp, &label->vl_waits, vlw_link) {
		*(w->vlw_addr) = vprg->vmp_enc_idx;
		list_del(&w->vlw_link);
		free(w);
	}

	return 0;
}

static void vm_labels_fini(struct vm_program *vprg)
{
	struct vm_label *label;
	struct vm_label *ltmp;
	struct vm_label_wait *tmp;
	struct vm_label_wait *w;

	list_for_each_entry_safe(label, ltmp, &vprg->vmp_labels, vl_link) {
		list_for_each_entry_safe(w, tmp, &label->vl_waits, vlw_link) {
			list_del(&w->vlw_link);
			free(w);
		}
		free(label->vl_name);
		list_del(&label->vl_link);
		free(label);
	}

}

static int vm_labels_is_resolved(struct vm_program *vprg)
{
	struct vm_label *label;

	list_for_each_entry(label, &vprg->vmp_labels, vl_link) {
		if ((label->vl_addr == VM_LABEL_NEW) ||
		    (!list_empty(&label->vl_waits)) {
			err_print("unresolved label  %s:%s\n",
				  vprg->vmp_name, label->vl_name);
			return -EINVAL;
		}
	}

	return 0;
}


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

	vm_labels_fini(vprg);
	free(vprg->vmp_data);
	free(vprg->vmp_name);
	free(vprg);
}

int vm_program_check(struct vm_program *vprg)
{
	int rc;
	DPRINT("program check \n");

	/* first check all labels resolved */
	rc = vm_labels_is_resolved(vprg);
	if (rc < 0)
		return rc;

	return 0;
}

struct vm_program *vm_program_find(char *name)
{
	struct vm_program *pos;

	list_for_each_entry(pos, &vm_programs, vmp_link) {
		if (strcasecmp(pos->vmp_name, name) == 0)
			return pos;
	}
	return NULL;
}

void vm_programs_fini()
{
	struct vm_program *pos;
	struct vm_program *tmp;

	list_for_each_entry_safe(pos, tmp, &vm_programs, vmp_link) {
		list_del(&pos->vmp_link);
		vm_program_fini(pos);
	}

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
	long *l;

	if (vprg->vmp_size <= (vprg->vmp_enc_idx + sizeof(long)))
		return -ENOMEM;

	l = (long *)&vprg->vmp_data[vprg->vmp_enc_idx];
	*l = ldata;
	vprg->vmp_enc_idx += sizeof(long);

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
	struct vm_label *l;
	DPRINT("goto %s\n", data.cd_string);

	l = vm_label_find(vprg, data.cd_string);
	if (l == NULL)
		return -ENOMEM;

	return add_long_to_buffer(vprg, l->vl_addr);
}

static int enc_jz(struct vm_program *vprg, union cmd_arg data)
{
	struct vm_label *l;
	DPRINT("jz %s\n", data.cd_string);

	l = vm_label_find(vprg, data.cd_string);
	if (l == NULL)
		return -ENOMEM;

	return add_long_to_buffer(vprg, l->vl_addr);
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

const static enc_h_t en_helpers[VM_CMD_MAX] = {
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

