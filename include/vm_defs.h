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
	/* conditional jump */
	VM_CMD_IF,
	/* */
	VM_CMD_ADD,
	/* */
	VM_CMD_SUB,
	VM_CMD_MAX
};

#endif