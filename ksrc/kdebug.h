#ifndef __SIM_KDEBUG__
#define __SIM_KDEBUG__

#include "loadsim_c.h"
#include <linux/kernel.h>

#define DPRINT(a, ...)  if (DEBUG) { printk(a, ##__VA_ARGS__); }
#define ENTER()		if (DEBUG) {printk("enter %s\n", __FUNCTION__); }
#define LEAVE()		if (DEBUG) {printk("leave %s\n", __FUNCTION__); }	

#define err_print(a, ...) printk(KERN_ERR a, ##__VA_ARGS__)

#endif
