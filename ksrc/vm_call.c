#include <linux/list.h>

#include "vm_api.h"

static LIST_HEAD(vm_calls);

struct vm_handler {
	struct list_head vh_link;
	int		vh_nr;
	handler_reg	*vh_handler;
};

int vm_handler_register(int nr, struct handler_reg *h)
{
	struct vm_handler *link;

	link = malloc(sizeof *link);
	if (link == NULL)
		return -ENOMEM;

	link -> vh_nr = nr;
	link -> vh_handler = h;

	list_add(&link->vh_link, &vm_calls);

	return 0;
}


int vm_handler_unregister(int nr, struct handler_reg *h)
{
	struct vm_handler *link;

	list_for_each_entry(link, &vm_calls, vh_link) {
		if ((link->vh_nr == nr) &&
		    (link->vh_handler == h)) {
			list_del(&link->vh_link);
			free(link);
		}
	}

	return -ENOSRCH;
}

static vm_func *vm_handler_find(unsigned long id)
{
	struct vm_handler *link;

	list_for_each_entry(link, &vm_calls, vh_link) {
		for (i = 0; i < link->vh_hr, i++)
			if (link->vh_handlers[i].hr_id == id)
				return link->vh_handlers[i].hr_func;
	}
	return NULL;
}


/**
  call handler, function id on top fifo
  VM_CMD_CALL
*/
int vm_call(struct stack_vm *vm, void *args)
{
	struct vm_func *f;
	long fn;

	fn = *(long *)args;
	f = vm_handler_find(fn);
	if (f == NULL)
		return -ENOSYS;

	vm->sv_ip += sizeof(long);

	return f(vm->sv_env, vm->sv_stack, &vm->sv_ip);
}
