#ifndef __MD_SIM_ENCODE__
#define __MD_SIM_ENCODE__

#include <vm_compile.h>

enum ast_type {
	AST_STRING,   // string type
	AST_NUMBER,   // numberic type
	AST_REGISTER, // compatible with any type except NONE
	AST_TYPE_NONE, // none data exist
};

#define START_LABEL	"start:"
#define END_LABEL	"end:"


struct ast_node;
struct vm_program;

int ast_check_type(struct ast_node *arg, enum ast_type type);
struct ast_node *ast_op(int line, enum vm_cmd cmd, union cmd_arg arg, enum ast_type type, int nchilds, ...);
int ast_encode(struct vm_program *vprg, struct ast_node *root);

int procedure_start(char *name);
struct vm_program *procedure_current(void);
int procedure_end(void);

#endif

