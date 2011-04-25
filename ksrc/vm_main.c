#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/errno.h>

#include "kdebug.h"
#include "stack.h"
#include "reg.h"
#include "vm_defs.h"
#include "vm_api.h"
#include "vm_core.h"

int vm_interpret_init(struct stack_vm **vm, int stack_size,
		      char __user *prg, int size, int regs, struct simul_env *env)
{
	struct fifo *stack;
	char *program;
	struct stack_vm *v;
	struct reg_file *r;
	int rc;

	v = kmalloc(sizeof *v, GFP_KERNEL);
	if (v == NULL) {
		err_print("can't allocate memory for stack vm\n");
		return -ENOMEM;
	}

	stack = fifo_create(stack_size);
	if (stack == NULL) {
		rc = -ENOMEM;
		goto err1;
	}

	program = kmalloc(size, GFP_KERNEL);
	if (program == NULL) {
		err_print("can't allocate memory for program - %d\n", size);
		rc = -ENOMEM;
		goto err2;
	}

	r = reg_file_init(regs);
	if (r == NULL) {
		err_print("can't allocate memory for reg file %d\n", regs);
		rc = -ENOMEM;
		goto err3;
	}

	if (copy_from_user(program, prg, size)) {
		err_print("can't copy program from user %p - %d\n",
			prg, size);
		rc = -EFAULT;
		goto err3;
	}

	v->sv_ip = 0;
	v->sv_program = program;
	v->sv_size = size;
	v->sv_stack = stack;
	v->sv_env = env;
	v->sv_reg = r;

	*vm = v;

	return 0;
err3:
	kfree(program);
err2:
	fifo_destroy(stack);
err1:
	kfree(v);
	*vm = NULL;
	return rc;
}

void vm_interpret_fini(struct stack_vm *vm)
{
	kfree(vm->sv_program);
	fifo_destroy(vm->sv_stack);
	reg_file_fini(vm->sv_reg);
	kfree(vm);
}

const static vm_int_fn int_fn[VM_CMD_MAX] = {
	[VM_CMD_PUSHS] = vm_pushs,
	[VM_CMD_PUSHL] = vm_pushl,
	[VM_CMD_CMPS]  = vm_cmps,
	[VM_CMD_CMPL]  = vm_cmpl,
	[VM_CMD_CALL]  = vm_call,
	[VM_CMD_GOTO]  = vm_goto,
	[VM_CMD_JZ]    = vm_jz,
	[VM_CMD_JNZ]   = vm_jnz,
	[VM_CMD_ADD]   = vm_add,
	[VM_CMD_SUB]   = vm_sub,
	[VM_CMD_DUP]   = vm_dup,
	[VM_CMD_UP]    = vm_up,
	[VM_CMD_NOP]   = NULL,
	[VM_CMD_GETR]  = vm_getr,
	[VM_CMD_PUTR]  = vm_putr,
	[VM_CMD_LABEL] = NULL,
	[VM_CMD_CMPL_EQ] = vm_cmpl_eq,
	[VM_CMD_CMPL_NE]= vm_cmpl_ne,
	[VM_CMD_CMPL_LOW]= vm_cmpl_low,
	[VM_CMD_CMPL_GR]= vm_cmpl_gr,
	[VM_CMD_CMPL_LE]= vm_cmpl_le,
	[VM_CMD_CMPL_GE]= vm_cmpl_ge,
	[VM_CMD_CMPS_EQ]= vm_cmps_eq,
	[VM_CMD_CMPS_NE]= vm_cmps_ne,
	[VM_CMD_CMPS_LOW]= vm_cmps_low,
	[VM_CMD_CMPS_GR]= vm_cmps_gr,
	[VM_CMD_CMPS_LE]= vm_cmps_le,
	[VM_CMD_CMPS_GE]= vm_cmps_ge,
};


int vm_interpret_run(struct stack_vm *vm)
{
	unsigned char op;
	int rc = -ENODATA;
	long old_ip;
	struct timespec start = CURRENT_TIME;

	vm->sv_run = 1;
	while (vm->sv_ip < vm->sv_size) {
		old_ip = vm->sv_ip;
		op = vm->sv_program[vm->sv_ip ++];
		DPRINT("trace %lx op %d - stack %d\n", old_ip, op, stack_size(vm->sv_stack));
		if ((op > ARRAY_SIZE(int_fn)) || (int_fn[op] == NULL)) {
			err_print("unknow op (%c) !\n", op);
			rc = -EINVAL;
		} else {
			rc = int_fn[op](vm, &vm->sv_program[vm->sv_ip]);
		}
		if (rc) {
			/* if operation failed  - restore pointer to operation */
			err_print("operation %c (%u) - fail %d\n",
				  op, (unsigned int)old_ip, rc);
			vm->sv_ip = old_ip;
			break;
		}
	}
	vm->sv_time = timespec_sub(CURRENT_TIME, start);

	if (rc == 0) {
		long exit_rc;
		rc = stack_pop(vm->sv_stack, &exit_rc);
		if (rc == 0)
			rc = exit_rc;
	}
	vm->sv_run = 0;
	vm->sv_rc = rc;
	return rc;
}


