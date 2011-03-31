#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "kdebug.h"
#include "reg.h"

struct reg_file {
	int	rf_index;
	long	rf_data[0];
};

#define reg_size(i)	(sizeof(struct reg_file) + (i)*sizeof(long))

struct reg_file *reg_file_init(int index)
{
	struct reg_file *r;

	r = kmalloc(reg_size(index), GFP_KERNEL);
	if (r) {
		r->rf_index = index;
	}
	return r;
}


void reg_file_fini(struct reg_file *reg)
{
	if (reg)
		kfree(reg);
}

#define valid_index(r,i) (((i) >= 0) && ((reg)->rf_index >= (i)))

int reg_file_get(struct reg_file *reg, long index, long *data)
{
	if (!valid_index(reg, index))
		return -ERANGE;

	DPRINT("get reg %ld - %ld\n", index, reg->rf_data[index]);
	*data = reg->rf_data[index];
	return 0;
}

int reg_file_put(struct reg_file *reg, long index, long data)
{
	if (!valid_index(reg, index))
		return -ERANGE;

	reg->rf_data[index] = data;
	DPRINT("put reg %ld - %ld\n", index, reg->rf_data[index]);

	return 0;
}


