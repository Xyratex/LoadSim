#include "encode.h"

struct ast_node *ast_op_eq(int line, struct ast_node *var1, struct ast_node *var3)
{
	union cmd_arg arg;
	int is_string;

	if (!ast_args_compatible(var1, var3))
		return NULL;

//	is_string = ast_check_type(var1, AST_STRING);
	is_string = 0;

	arg.cd_long = 0;
	return ast_op(line, is_string ? VM_CMD_CMPS_EQ : VM_CMD_CMPL_EQ,
		      arg, AST_NUMBER, 2, var1, var3);
}

struct ast_node *ast_op_ne(int line, struct ast_node *var1, struct ast_node *var3)
{
	union cmd_arg arg;
	int is_string;

	if (!ast_args_compatible(var1, var3))
		return NULL;

//	is_string = ast_check_type(var1, AST_STRING);
	is_string = 0;

	arg.cd_long = 0;
	return ast_op(line, is_string ? VM_CMD_CMPS_NE : VM_CMD_CMPL_NE,
		      arg, AST_NUMBER, 2, var1, var3);
}

struct ast_node *ast_op_gr(int line, struct ast_node *var1, struct ast_node *var3)
{                                                                      
	union cmd_arg arg;
	int is_string;

	if (!ast_args_compatible(var1, var3))
		return NULL;

//	is_string = ast_check_type(var1, AST_STRING);
	is_string = 0;

	arg.cd_long = 0;
	return ast_op(line, is_string ? VM_CMD_CMPS_GR : VM_CMD_CMPL_GR,
		      arg, AST_NUMBER, 2, var1, var3);
}

struct ast_node *ast_op_low(int line, struct ast_node *var1, struct ast_node *var3)
{
	union cmd_arg arg;
	int is_string;

	if (!ast_args_compatible(var1, var3))
		return NULL;

//	is_string = ast_check_type(var1, AST_STRING);
	is_string = 0;

	arg.cd_long = 0;
	return ast_op(line, is_string ? VM_CMD_CMPS_LOW : VM_CMD_CMPL_LOW,
		      arg, AST_NUMBER, 2, var1, var3);
}

struct ast_node *ast_op_le(int line, struct ast_node *var1, struct ast_node *var3)
{
	union cmd_arg arg;
	int is_string;

	if (!ast_args_compatible(var1, var3))
		return NULL;

//	is_string = ast_check_type(var1, AST_STRING);
	is_string = 0;

	arg.cd_long = 0;
	return ast_op(line, is_string ? VM_CMD_CMPS_LE : VM_CMD_CMPL_LE,
		      arg, AST_NUMBER, 2, var1, var3);
}

struct ast_node *ast_op_ge(int line, struct ast_node *var1, struct ast_node *var3)
{
	union cmd_arg arg;
	int is_string;

	if (!ast_args_compatible(var1, var3))
		return NULL;

	is_string = ast_check_type(var1, AST_STRING);

	arg.cd_long = 0;
	return ast_op(line, is_string ? VM_CMD_CMPS_GE : VM_CMD_CMPL_GE,
		      arg, AST_NUMBER, 2, var1, var3);
}
