#ifndef _MDSIM_REG_H_
#define _MDSIM_REG_H_

struct reg_file;

/**
 allocate memory for register file and init object
 */
struct reg_file *reg_file_init(int size);

/**
 release a resources associated with register file
*/
void reg_file_fini(struct reg_file *reg);

/**
 get data from a register file
 */
int reg_file_get(struct reg_file *reg, long index, long *data);

/**
 store data into register file
 */
int reg_file_put(struct reg_file *reg, long index, long data);

#endif
