#include <linux/slab.h>

#include "fifo.h"
#include "kdebug.h"

struct fifo {
	int ff_size;
	int ff_used;
	int ff_top;
	int ff_bottom;
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

	f->ff_size = size;
	f->ff_top = 0;
	f->ff_bottom = 0;
	f->ff_used = 0;

	return f;
}

/**
 destroy fifo structure
 */
void fifo_destroy(struct fifo *fifo)
{
	if (fifo->ff_used) {
		err_print("leak\n");
	}

	kfree(fifo);
}


int fifo_push(struct fifo *f, long data)
{
	if (f->ff_used == f->ff_size) {
		err_print("not free space in fifo for push\n");
		return -ENOMEM;
	}

	f->ff_data[f->ff_top] = data;
	f->ff_top = (f->ff_top + 1) % f->ff_size;
	f->ff_used ++;

	return 0;
}

int fifo_pop(struct fifo *f, long *data)
{
	if (f->ff_used == 0) {
		err_print("fifo empty for pop\n");
		return -ENODATA;
	}

	*data = f->ff_data[f->ff_bottom];
	f->ff_bottom = (f->ff_bottom + 1) % f->ff_size;
	f->ff_used --;

	return 0;
}
