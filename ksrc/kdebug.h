#ifndef __MD_SIM_KDEBUG__
#define __MD_SIM_KDEBUG__

#include <linux/kernel.h>

#define DPRINT(a, ...) if (1) { printk(a, ##__VA_ARGS__); }
#define ENTER()		if (1) {printk("enter %s\n", __FUNCTION__); }
#define LEAVE()		if (1) {printk("leave %s\n", __FUNCTION__); }	

#define err_print(a, ...) printk(a, ##__VA_ARGS__)

#endif
