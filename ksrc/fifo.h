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
 
 \return <0 if none freespace in stack; 0, if successfully added.
 */
int fifo_push(struct fifo *f, void *ptr);

/**
 get element from top of the stack.
 
 \return NULL if stack is empty, otherwise non NULL pointer is returned.
 */
void* fifo_pop(struct fifo *f);

#endif