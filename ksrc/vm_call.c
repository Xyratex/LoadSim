#inclide "fifo.h"

static struct list_head vm_calls;

struct vm_handler {
	struct list_head vh_link;
	int		vh_id;
	vm_func		vh_handler;
};

int vm_register_handler(unsigned long id, vm_func ptr)
{

	return 0;
}

int vm_unregister_handler(unsigned long id)
{

	return 0;
}
