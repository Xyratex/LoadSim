#ifndef _STACK_VM_CORE_H_
#define _STACK_VM_CORE_H_

struct stack_vm;

/**
 prototype for core functions
 */
typedef int (* vm_int_fn)(struct stack_vm *vm, void *args);


/** 
 push pointer to string
 VM_CMD_PUSHS
*/
int vm_pushs(struct stack_vm *vm, void *args);


/**
 push long
 VM_CMD_PUSHL
*/
int vm_pushl(struct stack_vm *vm, void *args);


/** 
 compare string, result on top fifo
 VM_CMD_CMPS
*/
int vm_cmps(struct stack_vm *vm, void *args);


/**
 compare long's, result on top fifo
 VM_CMD_CMPL
*/
int vm_cmpl(struct stack_vm *vm, void *args);

/**
  call handler, function id on top fifo
  VM_CMD_CALL
*/
int vm_call(struct stack_vm *vm, void *args);


/**
 goto, address on top of stack
 VM_CMD_GOTO
*/
int vm_goto(struct stack_vm *vm, void *args);

/**
 conditional jump, results on top of stack 
 VM_CMD_JZ
*/
int vm_jz(struct stack_vm *vm, void *args);

/**
 VM_CMD_ADD
 */
int vm_add(struct stack_vm *vm, void *args);

/**
 VM_CMD_SUB
*/
int vm_sub(struct stack_vm *vm, void *args);

/**
 duplicate data on stack top
 VM_CMD_DUP
*/
int vm_dup(struct stack_vm *vm, void *args);

/**
 just remove entry on stack top
 VM_CMD_NOP
*/
int vm_nop(struct stack_vm *vm, void *args);

#endif
