#ifndef _MDSIM_FIFO_
#define _MDSIM_FIFO_

struct fifo;


/**
 allocate FIFO and fill members with initial data.
 \a size - maximal size of FIFO
 
 \retval NULL - don't have memory for new structure.
 \retval !NULL - FIFO object.
 */
struct fifo *fifo_create(int size);

/**
 destroy FIFO structure
 */
void fifo_destroy(struct fifo *fifo);

/**
 push element into stack.
 
 */
int stack_push(struct fifo *f, long data);

/**
 get element from top of the stack.
 
 */
int stack_pop(struct fifo *f, long *data);

/**
 return current stack size
 */
int stack_size(struct fifo *f);

#endif
