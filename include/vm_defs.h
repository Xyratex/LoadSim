#ifndef _STACK_VM_DEFS_H_
#define _STACK_VM_DEFS_H_

enum cmd_base {
	/* push pointer to string */
	VM_CMD_PUSHS,
	/* push long */
	VM_CMD_PUSHL,
	/* compare string, result on top fifo */
	VM_CMD_CMPS,
	/* compare long's, result on top fifo */
	VM_CMD_CMPL,
	/* call handler, function id on top fifo */
	VM_CMD_CALL,
	/* goto, address on top of stack */
	VM_CMD_GOTO,
	/* conditional jump, results on top of stack */
	VM_CMD_JZ,
	/* */
	VM_CMD_ADD,
	/* */
	VM_CMD_SUB,
	/* duplicate data on stack top */
	VM_CMD_DUP,
	/* just remove entry on stack top */
	VM_CMD_NOP,
	VM_CMD_MAX,
};

enum vm_sys_calls {
	VM_SYS_USER	= 0,
	VM_SYS_GROUP,
	VM_SYS_SLEEP,
	VM_SYS_RACE,
	VM_SYS_CALL_MAX
};

int sys_handlers_register(void);

void sys_handlers_unregister(void);

/**
 procedures called via VM_CMD_CALL function
 */
enum vm_md_calls {
	VM_MD_CALL_CD	= 100,
	VM_MD_CALL_MKDIR,
	VM_MD_CALL_READIR,
	VM_MD_CALL_UNLINK,
	VM_MD_CALL_OPEN,
	VM_MD_CALL_CLOSE,
	VM_MD_CALL_STAT,
	VM_MD_CALL_SETATTR,
	VM_MD_CALL_SOFTLINK,
	VM_MD_CALL_HARDLINK,
	VM_MD_CALL_READLINK,
	VM_MD_CALL_MAX,
};

int md_handlers_register(void);

void md_handlers_unregister(void);


#endif