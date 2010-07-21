#include <linux/string.h>
#include <linux/errno.h>

#include "kdebug.h"
#include "fifo.h"
#include "vm_api.h"

/** 
 push pointer to string
 VM_CMD_PUSHS
*/
int vm_pushs(struct stack_vm *vm, void *args)
{
	DPRINT("pushs\n");
	vm->sv_ip += strlen(args) + 1;

	return fifo_push(vm->sv_stack, (long)args);
}

/**
 push long
 VM_CMD_PUSHL
*/
int vm_pushl(struct stack_vm *vm, void *args)
{
	long a;

	a = *(long *)args;
	DPRINT("pushl %ld\n", a);
	vm->sv_ip += sizeof(long);

	return fifo_push(vm->sv_stack, a);
}

/** 
 compare string, result on top fifo
 VM_CMD_CMPS
*/
int vm_cmps(struct stack_vm *vm, void *args)
{
	long a;
	long b;
	long rc;

	if ((fifo_pop(vm->sv_stack, &a) < 0) ||
            (fifo_pop(vm->sv_stack, &b) < 0))
		return -ENODATA;

	rc = strcmp((char *)a, (char *)b);

	return fifo_push(vm->sv_stack, rc);
}

/**
 compare long's, result on top fifo
 VM_CMD_CMPL
*/
int vm_cmpl(struct stack_vm *vm, void *args)
{
	long a;
	long b;
	long rc;

	if ((fifo_pop(vm->sv_stack, &a) < 0)||
            (fifo_pop(vm->sv_stack, &b) < 0))
		return -ENODATA;

	if (a > b) 
		rc = 1;
	else if (a == b)
		rc = 0;
	     else
		rc = -1;

	return fifo_push(vm->sv_stack, rc);
}

/**
 goto, address on top of stack
 VM_CMD_GOTO
*/
int vm_goto(struct stack_vm *vm, void *args)
{
	long addr;

	addr = *(long *)args;
	vm->sv_ip = addr;

	return 0;
}


/**
 VM_CMD_JZ
*/
int vm_jz(struct stack_vm *vm, void *args)
{
	long a;
	long addr;

	if (fifo_pop(vm->sv_stack, &a) < 0 )
		return -ENODATA;

	addr = *(long *)args;
	DPRINT("jz %ld (%ld)\n", addr, a);
	if(a == 0)
		vm->sv_ip = addr;
	else
		vm->sv_ip += sizeof(long);

	return 0;
}

/**
 VM_CMD_JNZ
*/
int vm_jnz(struct stack_vm *vm, void *args)
{
	long a;
	long addr;

	if (fifo_pop(vm->sv_stack, &a) < 0 )
		return -ENODATA;

	addr = *(long *)args;
	DPRINT("jnz %ld (%ld)\n", addr, a);
	if(a != 0)
		vm->sv_ip = addr;
	else
		vm->sv_ip += sizeof(long);

	return 0;
}

/**
 VM_CMD_ADD
 */
int vm_add(struct stack_vm *vm, void *args)
{
	long a;
	long b;
	long res;

	if ((fifo_pop(vm->sv_stack, &a) < 0) ||
	    (fifo_pop(vm->sv_stack, &b) < 0))
		return -ENODATA;

	res = a + b;
	return fifo_push(vm->sv_stack, res);
}

/**
 VM_CMD_SUB
 a - b
 fifo state:
 [b][a][fifo-bottom]
*/
int vm_sub(struct stack_vm *vm, void *args)
{
	long a;
	long b; 
	long res;

	if ((fifo_pop(vm->sv_stack, &b) < 0) ||
	    (fifo_pop(vm->sv_stack, &a) < 0))
		return -ENODATA;

	res = a - b;

	return fifo_push(vm->sv_stack, res);
}

/**
 duplicate data on stack top
 VM_CMD_DUP
*/
int vm_dup(struct stack_vm *vm, void *args)
{
	long a;
	int rc;

	if (fifo_pop(vm->sv_stack, &a) < 0)
		return -ENODATA;

	rc = fifo_push(vm->sv_stack, a);
	if (rc < 0)
		return rc;

	rc = fifo_push(vm->sv_stack, a);
	if (rc < 0)
		return rc;

	return 0;
}

/**
 just remove entry on stack top
 VM_CMD_NOP
*/
int vm_nop(struct stack_vm *vm, void *args)
{
	long a;

	return fifo_pop(vm->sv_stack, &a);
}


