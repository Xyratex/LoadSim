#ifndef _STACK_VM_COMPILE_H_
#define _STACK_VM_COMPILE_H_

#include "vm_defs.h"
#include "list.h"

enum {
	/**
	 initial program size
	*/
	VM_INIT_PROG_SIZE = 10*1024
};

/**
 one point to want fixup
 after label address is resolved.
 */
struct vm_label_wait {
	/**
	 link into global wait list
	*/
	struct list_head vlw_link;
	/**
	 address of code, which want to fix
	*/
	long		*vlw_addr;
};

/**
 labels in a program
 */
struct vm_label {
	/**
	 link into global labels list
	*/
	struct list_head vl_link;
	/**
	 label name
	*/
	char	*vl_name;
	/**
	 address in program
	*/
	long	vl_addr;
	/**
	 list of addresses which want fix label reference
	*/
	struct list_head vl_waits;
};


/**
 program to load into vm
 */
struct vm_program {
	/**
	 link to the list of the programs for clients
	 */
	struct list_head vmp_link;
	/**
	 name of this program
	*/
	char		*vmp_name;
	/**
	 last position to encode
	*/
	long		vmp_enc_idx;
	/**
	 current program size (allocated)
	*/
	long		vmp_size;
	/** 
	 program code 
	*/
	char		*vmp_data;
	/**
	 labels in the program
	*/
	struct list_head vmp_labels;
};

/**
 create structure to store program and fill with initial values

 @param vprg pointer to new program or NULL if program can't created.
 */
int vm_program_init(struct vm_program **vprg, char *name);

/**
 release resources allocated for the program.
 */
void vm_program_fini(struct vm_program *vprg);

/**
 check program to correctness.
 (currently have check all labels to be resolved)
 
 @retval 0 program is valid
 @retval <0 program is invalid
 */
int vm_program_check(struct vm_program *vprg);


/**
 find progmram for vm by name
 */
struct vm_program *vm_program_find(char *name);

/**
 destroy all existent programs for virtual machine
 */
void vm_programs_fini(void);


/**
 arguments of the VM functions
 */
union cmd_arg {
	int		cd_call; /** call id */
	long		cd_long; /** integer value */
	char		*cd_string; /** string */
};
/**
 add command to the program.
 need to adjust vmp_end_idx to point to byte after command.
 
 @param vprg - program to add operation.
 @param cmd - command
 @param data - command arguments
 */
int vm_encode(struct vm_program *vprg, enum cmd_base cmd, union cmd_arg data);

/**
 add label to the list and fix all jump point to using correct address.
 called by parser when it found label.

 @param vprg - program to add label.
 */
int vm_label_resolve(struct vm_program *vprg, char *label_name);

#endif
