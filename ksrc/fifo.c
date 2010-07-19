#include <linux/slab.h>
#include "fifo.h"

struct fifo {
	int ff_size;
	int ff_top;
	int ff_bottom;
	long ff_data[0];
};

struct fifo *fifo_create(int size)
{
	struct fifo *f;

	f = kmalloc(sizeof(struct fifo) + size *sizeof(void *), GFP_KERNEL);
	if (f == NULL)
		return NULL;

	f->ff_size = size;
	f->ff_top = size;
	f->ff_bottom = size;

	return f;
}

/**
 destroy fifo structure
 */
void fifo_destroy(struct fifo *fifo)
{
	if (fifo->ff_bottom != fifo->ff_top) {
		printk(KERN_ERR "leak\n");
	}

	kfree(fifo);
}


int fifo_push(struct fifo *f, long data)
{
	if (f->ff_bottom == f->ff_top)
		return -ENOMEM;

	f->ff_data[f->ff_top] = data;
	f->ff_top = (f->ff_top + 1) % f->ff_size;

	return 0;
}

int fifo_pop(struct fifo *f, long *data)
{
	if (f->ff_bottom == f->ff_top)
		return -ENODATA;

	*data = f->ff_data[f->ff_bottom];
	f->ff_bottom = (f->ff_bottom + 1) % f->ff_size;

	return 0;
}
