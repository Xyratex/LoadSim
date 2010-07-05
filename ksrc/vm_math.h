#ifndef _STACK_VM_MATH_H_
#define _STACK_VM_MATH_H_

#include <stdint.h>

struct fifo;
/**
 */
int vm_add(struct fifo *f, uint32_t *ip);

/**
 */
int vm_sub(struct fifo *f, uint32_t *ip);

#endif