#include <stdint.h>
#include "list.h"

#include "fifo.h"

#include "vm_defs.h"
#include "vm_math.h"

int32_t vm_ip;

struct fifo *stack;


int vm_interpret_init(int stack_size)
{

	return 0;
}

void vm_interpret_fini(void)
{
}

int vm_interpret_run(char *program, size_t size)
{

	return 0;
}


