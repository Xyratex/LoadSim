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
	/* compare two operands on stack, result put on stack*/
	VM_CMD_IF,
	/* */
	VM_CMD_ADD,
	/* */
	VM_CMD_SUB,
	VM_CMD_MAX,
};

/**
 procedures called via VM_CMD_CALL function
 */
enum vm_calls {
	VM_CALL_CD,
	VM_CALL_MKDIR,
	VM_CALL_READIR,
	VM_CALL_UNLINK,
	VM_CALL_OPEN,
	VM_CALL_CLOSE,
	VM_CALL_STAT,
	VM_CALL_SETATTR,
	VM_CALL_SOFTLINK,
	VM_CALL_HARDLINK,
	VM_CALL_READLINK,
	VM_CALL_MAX,
};

#endif