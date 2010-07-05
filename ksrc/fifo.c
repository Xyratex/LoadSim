#include <stdio.h>

struct fifo {
	int ff_size;
	int ff_top;
	void *ff_data[0];
};

int fifo_push(void *ptr)
{

	return 0;
}

void *fifo_pop()
{
	return NULL;
}