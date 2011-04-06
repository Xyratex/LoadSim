#ifndef __MD_SIM_ENCODE__
#define __MD_SIM_ENCODE__

#include <vm_compile.h>

struct sim_arr {
	int size;
	char *data;
};

/**
 Module implenting a Abstract Sytax Tree (AST) for a program
 that tree used to converting expressions to correct order of operations.
 */

enum ast_type {
	AST_STRING,    // operation retur string
	AST_NUMBER,    // numberic type
	AST_REGISTER,  // compatible with any type except NONE
	AST_TYPE_NONE, // none data exist
};

#define START_LABEL	"start:"
#define END_LABEL	"end:"


struct ast_node;
struct vm_program;

/**
 check type of node via compatible matrix.
 \retval 1 - if node compatible with requested type
 \retval 0 - if node is incompatible
*/
int ast_check_type(struct ast_node *arg, enum ast_type type);

/**
 is two args has a compatible types.
 
 \retval 1 - if node compatible with requested type
 \retval 0 - if node is incompatible
*/
int ast_args_compatible(struct ast_node *arg1, struct ast_node *arg2);

/**
 add operations into AST
*/
struct ast_node *ast_op(int line, enum vm_cmd cmd, union cmd_arg arg, enum ast_type type, int nchilds, ...);

/**
 link two childs tree into one root
 */
struct ast_node *ast_op_link(int line, struct ast_node *child1, struct ast_node *child2);

/**
 create operation with storing for data
 */
struct ast_node *ast_buffer(int line, int size);

/**
 create ast tree for while() statement
 */
struct ast_node *ast_op_while_start(int line, struct ast_node *expr);
/**
 create a ast tree for endw statement
*/
struct ast_node *ast_op_while_end(int line);


/**
 generate tree for "expected" part
 */
struct ast_node *ast_op_expected(int line, int val);

/**
 logical operation.
 see encode_expr.c
 */
/** 
 OP ==
*/
struct ast_node *ast_op_eq(int line, struct ast_node *var1, struct ast_node *var3);
/**
 OP !=
 */
struct ast_node *ast_op_ne(int line, struct ast_node *var1, struct ast_node *var3);
/**
 OP >
 */
struct ast_node *ast_op_gr(int line, struct ast_node *var1, struct ast_node *var3);
/**
 OP <
 */
struct ast_node *ast_op_low(int line, struct ast_node *var1, struct ast_node *var3);
/**
 OP <=
 */
struct ast_node *ast_op_le(int line, struct ast_node *var1, struct ast_node *var3);
/**
 OP >=
 */
struct ast_node *ast_op_ge(int line, struct ast_node *var1, struct ast_node *var3);



/**
 Convert AST into binary code
*/
int ast_encode(struct vm_program *vprg, struct ast_node *root);
/**
 release AST objects
 */
void ast_free(struct ast_node *root);

/**
 */
int procedure_start(char *name);
struct vm_program *procedure_current(void);
int procedure_end(void);

#endif

