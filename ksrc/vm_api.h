#ifndef _STACK_VM_API_H_
#define _STACK_VM_API_H_

#include <linux/types.h>

struct fifo;

/**
 function handler.
 \a IP pointer to 'instruction pointer' variable. used to implementation
  of jump or conditional jump.

 \return 0 if function is executed ok, non zero return code has indicate
  fatal error
 */
typedef int (*vm_func)(void *env, struct fifo *fifo, uint32_t *IP);


struct handler_reg {
	int	hr_id;
	vm_func	hr_func;
};

/**
 register handler for external function
 
 \retval 0 - function registered successfully
 \retval -EEXIST - function with that id already exist.
 */
int vm_handler_register(int nr, struct handler_reg *h);

/**
 remove handler to external function from a system.
 \a id - unique identifier of the function
 
 \retval 0 - function unregistered successfully.
 \retval -ENOSRC - function with that id not registered in system
 \retval <0 - other errors.
 */
int vm_handler_unregister(int nr, struct handler_reg *h);


/**
 internal VM state
 */
struct stack_vm {
	char		*sv_program;
	int		sv_size;
	int32_t		sv_ip;
	struct fifo	*sv_stack;
	void		*sv_env;
	long		sv_run:1;
	int		sv_rc;
};

/* vm_main.c */

/**
 set interpreter initial state.
 \a vm new create vm pointer
 \a stack_size - maximal stack usage
 \a prg program to copy from user memory
 \a size program size
 \a env environment to run external function
 
 \retval 0 - interpreter fully init
 \retval -ENOMEM - not have memory
 \retval <0 - other errors

 if initialization failed, vm pointer set to NULL.
 */
int vm_interpret_init(struct stack_vm **vm, int stack_size,
		      char __user *prg, int size, void *env);

/**
 destroy virtual machine and release owned resources.
 */
void vm_interpret_fini(struct stack_vm *vm);

/**
 run program on virtual machine.
 
 function has returned data from top of stack and assume that is return code.
 */
int vm_interpret_run(struct stack_vm *vm);

#endif
