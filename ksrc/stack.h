#ifndef _MDSIM_FIFO_
#define _MDSIM_FIFO_

struct stack;


/**
 allocate FIFO and fill members with initial data.
 \a size - maximal size of FIFO
 
 \retval NULL - don't have memory for new structure.
 \retval !NULL - FIFO object.
 */
struct stack *fifo_create(int size);

/**
 destroy FIFO structure
 */
void fifo_destroy(struct stack *fifo);

/**
 push element into stack.
 
 */
int stack_push(struct stack *f, long data);

/**
 get element from top of the stack.
 
 */
int stack_pop(struct stack *f, long *data);

/**
 return current stack size
 */
int stack_size(struct stack *f);

/**
 debug stack dump
 */
void stack_dump(struct stack *f);

#endif
