#include <linux/slab.h>

#include "stack.h"
#include "kdebug.h"

struct stack {
	int ff_size;
	int ff_top;
	long ff_data[0];
};

struct stack *fifo_create(int size)
{
	struct stack *f;

	f = kmalloc(sizeof(struct stack) + size *sizeof(void *), GFP_KERNEL);
	if (f == NULL){
		err_print("can't alloc memory for fifo\n");
		return NULL;
	}

	f->ff_size = size - 1;
	f->ff_top = 0;

	return f;
}

/**
 destroy stack structure
 */
void fifo_destroy(struct stack *fifo)
{
	if (fifo->ff_top != 0) {
		err_print("leak\n");
	}

	kfree(fifo);
}


int stack_push(struct stack *f, long data)
{
	if (f->ff_top == f->ff_size) {
		err_print("not free space in stack for push\n");
		return -ENOMEM;
	}

	f->ff_data[f->ff_top] = data;
	f->ff_top ++;

	return 0;
}

int stack_pop(struct stack *f, long *data)
{
	if (f->ff_top == 0) {
		err_print("fifo empty for pop\n");
		return -ENODATA;
	}

	f->ff_top --;
	*data = f->ff_data[f->ff_top];

	return 0;
}


int stack_size(struct stack *f)
{
	return f->ff_top;
}

void stack_dump(struct stack *f)
{
	int i;

	for(i = 0; i < f->ff_top; i++)
		printk("stack %d - %lx\n", i, f->ff_data[i]);
}
