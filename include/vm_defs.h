#ifndef _STACK_VM_DEFS_H_
#define _STACK_VM_DEFS_H_

enum {
	VM_DEF_STACK = 20,
	VM_RET_OK    = 0,
	VM_RET_FAIL  = 1,
};

enum cmd_base {
	/* push pointer to string */
	VM_CMD_PUSHS	= 0,
	/* push long */
	VM_CMD_PUSHL	= 1,
	/* compare string, result on top fifo */
	VM_CMD_CMPS	= 2,
	/* compare long's, result on top fifo */
	VM_CMD_CMPL	= 3,
	/* call handler, function id on top fifo */
	VM_CMD_CALL	= 4,
	/* goto, address on top of stack */
	VM_CMD_GOTO	= 5,
	/* conditional jump, results on top of stack */
	VM_CMD_JZ	= 6,
	/* jump if not zero or equal*/
	VM_CMD_JNZ	= 7,
	/* */
	VM_CMD_ADD	= 8,
	/* */
	VM_CMD_SUB	= 9,
	/* duplicate data on stack top */
	VM_CMD_DUP	= 10,
	/* just remove entry on stack top */
	VM_CMD_NOP	= 11,
	VM_CMD_MAX,
};

enum vm_sys_calls {
	VM_SYS_USER	= 0,
	VM_SYS_GROUP	= 1,
	VM_SYS_SLEEP	= 2,
	VM_SYS_RACE	= 3,
	VM_SYS_CALL_MAX
};

int sys_handlers_register(void);

void sys_handlers_unregister(void);

/**
 procedures called via VM_CMD_CALL function
 */
enum vm_md_calls {
	VM_MD_CALL_CD		= 100,
	VM_MD_CALL_MKDIR	= 101,
	VM_MD_CALL_READIR	= 102,
	VM_MD_CALL_UNLINK	= 103,
	VM_MD_CALL_OPEN		= 104,
	VM_MD_CALL_CLOSE	= 105,
	VM_MD_CALL_STAT		= 106,
	VM_MD_CALL_SETATTR	= 107,
	VM_MD_CALL_SOFTLINK	= 108,
	VM_MD_CALL_HARDLINK	= 109,
	VM_MD_CALL_READLINK	= 110,
	VM_MD_CALL_MAX,
};

int md_handlers_register(void);

void md_handlers_unregister(void);


#endif
