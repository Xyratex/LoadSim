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
 conditional jump, results on top of stack 
 jump if not zero or equal
 VM_CMD_JNZ
*/
int vm_jnz(struct stack_vm *vm, void *args);

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
 VM_CMD_UP
*/
int vm_up(struct stack_vm *vm, void *args);

/**
 get data from register and put into stack
*/
int vm_getr(struct stack_vm *vm, void *args);

/**
 get data from stack and put into register
*/
int vm_putr(struct stack_vm *vm, void *args);

/** strings */
/**
 ==
 */
int vm_cmps_eq(struct stack_vm *vm, void *args);
/**
 !=
 */
int vm_cmps_ne(struct stack_vm *vm, void *args);
/**
 >
 */
int vm_cmps_gr(struct stack_vm *vm, void *args);
/**
 <
 */
int vm_cmps_low(struct stack_vm *vm, void *args);
/**
 >=
 */
int vm_cmps_ge(struct stack_vm *vm, void *args);
/**
 <=
 */
int vm_cmps_le(struct stack_vm *vm, void *args);

/** numbers */
int vm_cmpl_eq(struct stack_vm *vm, void *args);
int vm_cmpl_ne(struct stack_vm *vm, void *args);
int vm_cmpl_gr(struct stack_vm *vm, void *args);
int vm_cmpl_low(struct stack_vm *vm, void *args);
int vm_cmpl_ge(struct stack_vm *vm, void *args);
int vm_cmpl_le(struct stack_vm *vm, void *args);


#endif
