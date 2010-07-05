#include "fifo.h"

#include "vm_math.h"

int vm_add(struct fifo *f, uint32_t *ip)
{
	long a;
	long b;
	long res;

	a = (long)fifo_pop(f);
	b = (long)fifo_pop(f);
	res = a + b;
	fifo_push(f, (void *)res);

	return 0;
}

int vm_sub(struct fifo *f, uint32_t *ip)
{
	long a;
	long b;
	long res;

	a = (long)fifo_pop(f);
	b = (long)fifo_pop(f);
	res = a - b;
	fifo_push(f, (void *)res);

	return 0;
}
