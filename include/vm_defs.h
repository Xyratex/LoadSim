#ifndef _STACK_VM_DEFS_H_
#define _STACK_VM_DEFS_H_

enum {
	/** default stack size */
	VM_DEF_STACK = 20,
	/**
	 function finished OK
	*/
	VM_RET_OK    = 0,
	/**
	 function finished with error
	*/
	VM_RET_FAIL  = 1,
	/**
	 maximal races in system
	*/
	VM_MAX_RACES = 200,
};

/**
 system commands for virtual machine.
 each system command has prototype

 long foo(struct stack_vm *vm, void *args);

 \arg vm - pointer to virtual machine to execute that operation
 \arg args - pointer to some area which store arguments for that operation.
 
 \retval 0 if operation executed correctly (has enough resources, arguments, etc)
 \retval <0 if operation hit fatal error, and execution should be stopped at that point.
 */
enum vm_cmd {
	/* push pointer to string */
	VM_CMD_PUSHS	= 0,
	/* push long */
	VM_CMD_PUSHL	= 1,
	/* compare string, result on top FIFO */
	VM_CMD_CMPS	= 2,
	/* compare long's, result on top FIFO */
	VM_CMD_CMPL	= 3,
	/* call handler, function id on top FIFO */
	VM_CMD_CALL	= 4,
	/* goto, address on top of stack */
	VM_CMD_GOTO	= 5,
	/* jump if equal or zero, results on top of stack */
	VM_CMD_JZ	= 6,
	/* jump if not zero or equal*/
	VM_CMD_JNZ	= 7,
	/* a + b */
	VM_CMD_ADD	= 8,
	/* a - b */
	VM_CMD_SUB	= 9,
	/* duplicate data on stack top */
	VM_CMD_DUP	= 10,
	/* just remove entry on stack top */
	VM_CMD_NOP	= 11,
	VM_CMD_MAX,
};


/**
 in additional to system commands, virtual machine
 has a many functions called via VM_CMD_CALL system command.
 each additional system function has prototype:

 \arg env constant pointer to environment passed when VM created
 \arg f FIFO with data. NOTE arguments are stored in reverse order,
        so a - b stored as [top][a][b][bottom]
 \arg ip instruction pointer, need if function want change execution order

 \retval 0 if function executed correctly
 \retval <0 if fatal error hit
 int foo_call(void *env, struct fifo *f, uint32_t *ip)

 each function get a parameters from a top of stack and must put a result
 
 typical implementation
int foo(void *env, struct fifo *f, uint32_t *ip)
{
	long gid;

	if (fifo_pop(f, &gid))
		return -ENODATA;

	current->gid = current->fsgid = gid;
	return fifo_push(f, 0);
}
*/

/**
 generic system calls
*/
enum vm_sys_calls {
	/**
	 change uid for current thread
	 */
	VM_SYS_USER	= 0,
	/**
	 change gid for current thread
	*/
	VM_SYS_GROUP	= 1,
	/**
	 delay execution for some time
	 */
	VM_SYS_SLEEP	= 2,
	/**
	 simulate race between threads
	 */
	VM_SYS_RACE	= 3,
	/**
	 generate unique name
	 */
	VM_SYS_TMPNAME  = 4,
	VM_SYS_CALL_MAX
};

/**
 register system calls in virtual machine
 */
int sys_handlers_register(void);

/**
 unregister system calls in virtual machine
 */
void sys_handlers_unregister(void);

/**
 external functions for lustre md backend
 */
enum vm_md_calls {
	/**
	 change work directory
	*/
	VM_MD_CALL_CD		= 100,
	/**
	 create directory
	 */
	VM_MD_CALL_MKDIR	= 101,
	/**
	 read directory contents
	 */
	VM_MD_CALL_READIR	= 102,
	/**
	 unlink metadata object
	 */
	VM_MD_CALL_UNLINK	= 103,
	/**
	 open file
	 */
	VM_MD_CALL_OPEN		= 104,
	/**
	 close already opened file.
	 */
	VM_MD_CALL_CLOSE	= 105,
	/**
	 get attributes for metadata object
	 */
	VM_MD_CALL_STAT		= 106,
	/**
	 change mode attribute for metadata object
	 */
	VM_MD_CALL_CHMOD	= 107,
	/**
	 change owner information for metadata object
	*/
	VM_MD_CALL_CHOWN	= 108,
	/**
	 change time in metadata object
	 */
	VM_MD_CALL_CHTIME	= 109,
	/**
	 change size of object
	 */
	VM_MD_CALL_TRUNCATE	= 110,
	/**
	 create a soft link between metadata objects
	 */
	VM_MD_CALL_SOFTLINK	= 111,
	/**
	 create a hard link between metadata objects
	 */
	VM_MD_CALL_HARDLINK	= 112,
	/**
	 read soft link contents
	 */
	VM_MD_CALL_READLINK	= 113,
	/**
	 rename
	 */
	VM_MD_CALL_RENAME	= 114,
	VM_MD_CALL_MAX,
};

/**
 register lustre md operations in virtual machine
 
 @return <0 if fatal error hit.
 */
int md_handlers_register(void);

/**
 unregister lustre md operation in virtual machine
 */
void md_handlers_unregister(void);


#endif
