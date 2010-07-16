#ifndef _MDSIM_FIFO_
#define _MDSIM_FIFO_

struct fifo;


/**
 allocate fifo and fill memebers with initial data.
 \a size - maximal size of fifo
 
 \retval NULL - don't have memory for new structure.
 \retval !NULL - fifo object.
 */
struct fifo *fifo_create(int size);

/**
 destroy fifo structure
 */
void fifo_destroy(struct fifo *fifo);

/**
 push element into stack.
 
 */
int fifo_push(struct fifo *f, long data);

/**
 get element from top of the stack.
 
 */
int fifo_pop(struct fifo *f, long *data);

#endif