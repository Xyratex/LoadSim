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

static int label_add_wait(struct vm_program *vprg, struct vm_label *label,
			  char *name)
{
	struct vm_label_wait *w;

	w = malloc(sizeof *w);
	if (w == NULL)
		return -ENOMEM;
	w->vlw_addr = (long *)&vprg->vmp_data[vprg->vmp_enc_idx];
	list_add(&w->vlw_link, &label->vl_waits);

	return 0;
}

/**
 try to find label by name or allocate new
 */
static struct vm_label *label_find(struct vm_program *vprg, char *name)
{
	struct vm_label *ret;

	list_for_each_entry(ret, &vprg->vmp_labels, vl_link) {
		if (strcasecmp(ret->vl_name, name) == 0) {
			/* forward reference to label.
			   create new wait reference.
			 */
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

/**
 find for goto/jz/..etc
 if label not resolved - create a new wait point
 */
struct vm_label *vm_label_find(struct vm_program *vprg, char *name)
{
	struct vm_label *ret;

	ret = label_find(vprg, name);
	if (ret == NULL)
		return NULL;

	if (ret->vl_addr == VM_LABEL_NEW) {
		if (label_add_wait(vprg, ret, name) < 0)
			return NULL;
	}

	return ret;
}


int vm_label_resolve(struct vm_program *vprg, char *label_name)
{
	struct vm_label *label;
	struct vm_label_wait *tmp;
	struct vm_label_wait *w;

	DPRINT("resolve label %s\n", label_name);

	label = label_find(vprg, label_name);
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
		    (!list_empty(&label->vl_waits))) {
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
	ret->vmp_regs = 0;
	INIT_LIST_HEAD(&ret->vmp_labels);
	list_add(&ret->vmp_link, &vm_programs);

	*vprg = ret;
	DPRINT("program '%s' created - %p\n", name, ret->vmp_data);
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


static int add_array_to_buffer(struct vm_program *vprg, int size, char *data)
{
	int i;
	int len;
	int ret;

	ret = add_long_to_buffer(vprg, size);
	if (ret != 0)
		return ret;

	for(i = 0; i < size; i++) {
		if (add_byte_to_buffer(vprg, data ? data[i] : '\0'))
			return -ENOMEM;
	}
	return 0;
}

static int add_string_to_buffer(struct vm_program *vprg, char *str)
{
	return add_array_to_buffer(vprg, strlen(str) + 1, str);
}

static int enc_pushs(struct vm_program *vprg, union cmd_arg data)
{
	int ret;
	DPRINT("'%s'", data.cd_string);

	return add_string_to_buffer(vprg, data.cd_string);
}

static int enc_pushl(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("%lu", data.cd_long);

	return add_long_to_buffer(vprg, data.cd_long);
}

static int enc_pusha(struct vm_program *vprg, union cmd_arg data)
{
//	DPRINT("pusha %lu\n", data.cd_arr.size);

//	return add_array_to_buffer(vprg, data.cd_arr.size, data.cd_arr.data);
}

static int enc_call(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("[%d]", data.cd_call);

	return add_long_to_buffer(vprg, data.cd_call);
}

static int enc_goto(struct vm_program *vprg, union cmd_arg data)
{
	struct vm_label *l;
	DPRINT("%s", data.cd_string);

	l = vm_label_find(vprg, data.cd_string);
	if (l == NULL)
		return -ENOMEM;

	return add_long_to_buffer(vprg, l->vl_addr);
}

static int enc_jz(struct vm_program *vprg, union cmd_arg data)
{
	struct vm_label *l;
	DPRINT("%s", data.cd_string);

	l = vm_label_find(vprg, data.cd_string);
	if (l == NULL)
		return -ENOMEM;

	return add_long_to_buffer(vprg, l->vl_addr);
}

static int enc_jnz(struct vm_program *vprg, union cmd_arg data)
{
	struct vm_label *l;
	DPRINT("%s", data.cd_string);

	l = vm_label_find(vprg, data.cd_string);
	if (l == NULL)
		return -ENOMEM;

	return add_long_to_buffer(vprg, l->vl_addr);
}

static int enc_none(struct vm_program *vprg, union cmd_arg data)
{
	return 0;
}

static int enc_nop(struct vm_program *vprg, union cmd_arg data)
{
	/* kill nop from commands */
	vprg->vmp_enc_idx --;
	return 0;
}

static int enc_getr(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT(" %lu", data.cd_long);

	if (vprg->vmp_regs < data.cd_long)
		vprg->vmp_regs = data.cd_long;

	return add_long_to_buffer(vprg, data.cd_long);
}

static int enc_putr(struct vm_program *vprg, union cmd_arg data)
{
	DPRINT("%lu", data.cd_long);

	if (vprg->vmp_regs < data.cd_long)
		vprg->vmp_regs = data.cd_long;

	return add_long_to_buffer(vprg, data.cd_long);
}

/**
 XXX to encode.c
*/
static int enc_label(struct vm_program *vprg, union cmd_arg data)
{
	/* kill label from commands */
	vprg->vmp_enc_idx --;

	return vm_label_resolve(vprg, data.cd_string);
}

struct vcmd {
	const char 	*name;
	enc_h_t		help;
};

const static struct vcmd en_helpers[VM_CMD_MAX] = {
	[VM_CMD_PUSHS]	= { "PUSHS", enc_pushs},
	[VM_CMD_PUSHL]	= { "PUSHL", enc_pushl},
	[VM_CMD_PUSHA]	= { "PUSHA", enc_pusha},
	[VM_CMD_CMPS]	= { "CMPS", enc_none},
	[VM_CMD_CMPL]	= { "CMPL", enc_none},
	[VM_CMD_CALL]	= { "CALL", enc_call},
	[VM_CMD_GOTO]	= { "GOTO", enc_goto},
	[VM_CMD_JZ]	= { "JZ", enc_jz},
	[VM_CMD_JNZ]	= { "JNZ", enc_jnz},
	[VM_CMD_ADD]	= { "ADD", enc_none},
	[VM_CMD_SUB]	= { "SUB", enc_none},
	[VM_CMD_DUP]	= { "DUP", enc_none},
	[VM_CMD_UP]	= { "UP", enc_none},
	[VM_CMD_NOP]	= { "NOP", enc_nop},
	[VM_CMD_GETR]	= { "GETR", enc_getr},
	[VM_CMD_PUTR]	= { "PUTR", enc_putr},
	[VM_CMD_LABEL]	= { "", enc_label},
	[VM_CMD_CMPL_EQ] = {"CMPL_EQ", enc_none},
	[VM_CMD_CMPL_NE]= {"CMPL_NE", enc_none},
	[VM_CMD_CMPL_LOW]= { "CMPL_LOW", enc_none},
	[VM_CMD_CMPL_GR]= { "CMPL_GR", enc_none},
	[VM_CMD_CMPL_LE]= { "CMPL_LE", enc_none},
	[VM_CMD_CMPL_GE]= { "CMPL_GE", enc_none},
	[VM_CMD_CMPS_EQ]= { "CMPS_EQ", enc_none},
	[VM_CMD_CMPS_NE]= { "CMPS_NE", enc_none},
	[VM_CMD_CMPS_LOW]={ "CMPS_LOW", enc_none},
	[VM_CMD_CMPS_GR]= { "CMPS_GR", enc_none},
	[VM_CMD_CMPS_LE]= { "CMPS_LE", enc_none},
	[VM_CMD_CMPS_GE]= { "CMPS_GE", enc_none},
};

int vm_encode(struct vm_program *vprg, int line, enum vm_cmd cmd, union cmd_arg data)
{
	int rc;

	if (cmd > VM_CMD_MAX)
		return -EINVAL;

	DPRINT("%s ", en_helpers[cmd].name);
	rc = add_byte_to_buffer(vprg, cmd);
	if (rc)
		goto out;

	rc = en_helpers[cmd].help(vprg, data);
out:
	DPRINT("\n");
	return rc;
}

int vm_cmd_want_string(enum vm_cmd cmd)
{
	if ((cmd == VM_CMD_PUSHS) ||
	    (cmd == VM_CMD_GOTO)  ||
	    (cmd == VM_CMD_JZ)    ||
	    (cmd == VM_CMD_JNZ)   ||
	    (cmd == VM_CMD_LABEL))
		return 1;

	return 0;
}