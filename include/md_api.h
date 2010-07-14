#ifndef _MD_API_H_
#define _MD_API_H_

/**
 procedures called via VM_CMD_CALL function
 */
enum vm_md_calls {
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

int md_handlers_register(void);

void md_handlers_unregister(void);

#endif