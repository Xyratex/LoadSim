#include <linux/slab.h>

#include "stack.h"
#include "kdebug.h"

struct fifo {
	int ff_size;
	int ff_top;
	long ff_data[0];
};

struct fifo *fifo_create(int size)
{
	struct fifo *f;

	f = kmalloc(sizeof(struct fifo) + size *sizeof(void *), GFP_KERNEL);
	if (f == NULL){
		err_print("can't alloc memory for fifo\n");
		return NULL;
	}

	f->ff_size = size - 1;
	f->ff_top = 0;

	return f;
}

/**
 destroy fifo structure
 */
void fifo_destroy(struct fifo *fifo)
{
	if (fifo->ff_top != 0) {
		err_print("leak\n");
	}

	kfree(fifo);
}


int stack_push(struct fifo *f, long data)
{
	if (f->ff_top == f->ff_size) {
		err_print("not free space in fifo for push\n");
		return -ENOMEM;
	}

	f->ff_data[f->ff_top] = data;
	f->ff_top ++;

	return 0;
}

int stack_pop(struct fifo *f, long *data)
{
	if (f->ff_top == 0) {
		err_print("fifo empty for pop\n");
		return -ENODATA;
	}

	f->ff_top --;
	*data = f->ff_data[f->ff_top];

	return 0;
}
