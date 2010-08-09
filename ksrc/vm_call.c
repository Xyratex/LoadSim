#include <linux/slab.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include "vm_api.h"
#include "kdebug.h"

static LIST_HEAD(vm_calls);

struct vm_handler {
	struct list_head 	vh_link;
	int			vh_nr;
	struct handler_reg	*vh_handler;
};

int vm_handler_register(int nr, struct handler_reg *h)
{
	struct vm_handler *link;

	link = kmalloc(sizeof *link, GFP_KERNEL);
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
			kfree(link);

			return 0;
		}
	}

	return -ESRCH;
}

static vm_func vm_handler_find(unsigned long id)
{
	struct vm_handler *link;
	int i;

	list_for_each_entry(link, &vm_calls, vh_link) {
		for (i = 0; i < link->vh_nr; i++)
			if (link->vh_handler[i].hr_id == id)
				return link->vh_handler[i].hr_func;
	}
	return NULL;
}


/**
  VM_CMD_CALL
  call handler, function id is long after OP id.
*/
int vm_call(struct stack_vm *vm, void *args)
{
	vm_func f;
	long fn;

	fn = *(long *)args;
	f = vm_handler_find(fn);
	DPRINT("callf %ld - %p\n", fn, f);
	if (f == NULL)
		return -ENOSYS;

	vm->sv_ip += sizeof(long);

	return f(vm->sv_env, vm->sv_stack, &vm->sv_ip);
}
