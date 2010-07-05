#ifndef _STACK_VM_DEFS_H_
#define _STACK_VM_DEFS_H_

#include "vm_defs.h"

struct fifo;

/**
 function handler.
 \a IP pointer to 'instruction pointer' variable. used to implementation
  of jump or conditional jump.

 \return 0 if function is executed ok, non zero return code has indicate
  fatal error
 */
typedef int (*vm_func)(struct fifo *fifo, uint32_t *IP);

/**
 register handler for specificed function
 \a id - unique identifier of the function
 \a ptr - pointer to handler function
 
 \retval 0 - function registered successfully
 \retval -EEXIST - function with that id already exist.
 */
int vm_register_handler(unsigned long id, vm_func ptr);

/**
 remove handler from a system.
 \a id - unique identifier of the function
 
 \retval 0 - function unregistered successfully.
 \retval -ENOSRC - function with that id not registered in system
 \retval <0 - other errors.
 */
int vm_unregister_handler(unsigned long id);


/**
 set interpreter intial state.
 \a stack_size - maximal stack usage
 
 \retval 0 - interperter fully init
 \retval -ENOMEM - not have memory
 \retval <0 - other errors
 */
int vm_interpret_init(int stack_size);

/**
 destroy interpreter
 */
void vm_interpret_fini(void);

/**
 run program on virtual machine.
 
 fuction has returned data from top of stack and assume that is return code.
 */
int vm_interpret_run(char *program, size_t size);

#endif