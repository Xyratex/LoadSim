#include <errno.h>
#include <stdint.h>
#include "list.h"

#include "fifo.h"

#include "vm_defs.h"
#include "vm_core.h"

int vm_interpret_init(struct stack_vm **vm, int stack_size,
		      char __user *prg, int size, void *env)
{
	struct fifo *stack;
	char *program;
	struct stack_vm *v;

	v = malloc(sizeof *v);
	if (v == NULL)
		return -ENOMEM;

	stack = fifo_create(stack_size);
	if (stack == NULL)
		goto err1;

	program = malloc(size);
	if (program == NULL) 
		goto err2;

	v->sv_ip = 0;
	v->sv_program = program;
	v->sv_size = size;
	v->sv_stack = stack;
	v->sv_env = env;

	*vm = v;
	return 0;
err2:
	fifo_destroy(stack);
err1:
	free(v);
	return -ENOMEM;
}

void vm_interpret_fini(struct stack_vm *vm)
{
	free(vm->sv_program);
	fifo_destroy(sv->stack);
	free(vm);
}

int vm_interpret_run(struct stack_vm *vm)
{
	char op;
	int rc;
	long old_ip;

	while(vm->sv_ip < vm->sv_size) {
		old_ip = vm->sv_ip;
		op = vm_program[vm->sv_ip ++];
		switch (op) {
		case VM_CMD_PUSHS:
			rc = vm_pushs(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_PUSHL:
			rc = vm_pushl(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_CMPS:
			rc = vm_cmps(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_CMPL:
			rc = vm_cmpl(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_CALL:
			rc = vm_call(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_GOTO:
			rc = vm_goto(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_JZ:
			rc = vm_jz(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_ADD:
			rc = vm_add(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_SUB:
			rc = vm_sub(vm, vm_program[vm->sv_ip]);
			break;
		case VM_CMD_DUP:
			rc = vm_dup(vm, vm_program[vm->sv_ip]);
			break
		case VM_CMD_NOP:
			rc = vm_nop(vm, vm_program[vm->sv_ip]);
			break
		default:
			printk(KERN_ERR, "unknow op (%c) !\n", op);
			rc = -EINVAL;
			break;
		};
		if (rc) {
			/* if operation failed  - restore pointer to operation */
			vm->sv_ip = old_ip;
			break;
		}
	}
	if (rc == 0) {
		long exit_rc;
		rc = fifo_pop(vm->sv_stack, &exit_rc);
		if (rc == 0)
			rc = exit_rc;
	}
	return rc;
}


