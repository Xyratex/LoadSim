#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "debug.h"
#include "encode.h"

/**
 abstract syntax tree node 
*/
struct ast_node {
	enum vm_cmd      an_cmd;   /** < VM command */
	enum ast_type    an_type;  /** < node type */
	int              an_line;  /** < line for syntax node */
	char            *an_label; /** < label in that line */
	union cmd_arg    an_local; /** < local argument (like call number 
				    or pointer to string) if need */
	unsigned int     an_nargs; /** < number of child expressions */
	struct ast_node *an_arg[0]; /** child expressions */
};

int ast_check_type(struct ast_node *arg, enum ast_type type)
{
	if(type == AST_REGISTER || arg->an_type == AST_REGISTER ||
	   arg->an_type == type)
		return 1;
	else
		return 0;
}

int ast_args_compatible(struct ast_node *arg1, struct ast_node *arg2)
{
	return ast_check_type(arg1, arg2->an_type);
}

struct ast_node *ast_op_link(int line, struct ast_node *child1, struct ast_node *child2)
{
	union cmd_arg arg;
	arg.cd_long = 0;

	return ast_op(line, VM_CMD_NOP, arg, AST_TYPE_NONE, 2, child1, child2);
}


struct ast_node *ast_op(int line, enum vm_cmd cmd, union cmd_arg local, enum ast_type type, int nchilds, ...)
{
	struct ast_node *node;
	int i;
	int size;
	va_list ap;

	size = sizeof(*node) + sizeof(node->an_arg[0])*nchilds;
	node = malloc(size);
	if (node) {
		memset(node, 0, size);
		node->an_line = line;
		node->an_cmd = cmd;
		node->an_type = type;
		node->an_local = local;

		node->an_nargs = nchilds;
		va_start(ap, nchilds);
		for(i = 0; i < nchilds; i++)
			node->an_arg[i] = va_arg(ap, struct ast_node *);
		va_end(ap);
	}

	DPRINT("add op %d - %d = %p\n", cmd, type, node);
	return node;
}

struct ast_node *ast_buffer(int line, int size)
{
	char *buff;
	union cmd_arg arg;
	struct ast_node *ret;

	buff = malloc(VM_STRING_SZ + 1);
	if (buff == NULL)
		return NULL;

	memset(buff, ' ', VM_STRING_SZ);
	buff[VM_STRING_SZ] = '\0';

	arg.cd_string = buff;
	ret = ast_op(line, VM_CMD_PUSHS, arg, AST_STRING, 0);
	if (ret == NULL)
		free(buff);
	return ret;
}

struct ast_node *ast_op_expected(int line, int exp)
{
	struct ast_node *cmd1 = NULL, *cmd2 = NULL, *cmd3 = NULL;
	union cmd_arg arg;

	/* dup need to read status on finish */
	/* arg not used */
	arg.cd_long = 0;
	cmd1 = ast_op(line, VM_CMD_DUP, arg, AST_REGISTER, 0);
	if (cmd1 == NULL)
		return NULL;

	arg.cd_long = 0;
	cmd2 = ast_op(line, VM_CMD_PUSHL, arg, AST_NUMBER, 0);
	if (cmd2 == NULL)
		goto error;

	/* arg not used */
	cmd3 = ast_op(line, VM_CMD_CMPL, arg, AST_NUMBER, 2, cmd1, cmd2);
	if (cmd3 == NULL)
		goto error;

	cmd2 = NULL;

	/** XXX */
	arg.cd_string = strdup(END_LABEL);
	cmd1 = ast_op(line, exp == VM_RET_OK ? VM_CMD_JNZ : VM_CMD_JZ, 
		      arg, AST_TYPE_NONE, 1, cmd3);
	if (cmd1 == NULL)
		goto error;

	cmd3 = ast_op(line, VM_CMD_UP, 
		      arg, AST_TYPE_NONE, 1, cmd1);
	if (cmd3 == NULL)
		goto error;

	return cmd3;
error:
	ast_free(cmd1);
	ast_free(cmd2);
	ast_free(cmd3);
	return NULL;
}


#define WHILE_START_LABEL "loop_st%u:"
#define WHILE_END_LABEL "loop_end%u:"
#define while_start_sz() (sizeof(WHILE_START_LABEL)+10)
#define while_end_sz()	(sizeof(WHILE_END_LABEL)+10)

static int while_no = 0;

/**
while_start:
    [expr]
    jz while_end
    ... 
    jump while_start
while_end:
 */
struct ast_node *ast_op_while_start(int line, struct ast_node *expr)
{
	struct ast_node *cmd1, *cmd2;
	union cmd_arg arg;
	char *label1;

	while_no ++;

	label1 = malloc(while_start_sz());
	if (label1 == NULL)
		return NULL;

	snprintf(label1, while_start_sz(), WHILE_START_LABEL, while_no);
	arg.cd_string = label1;
	cmd2 = ast_op(line, VM_CMD_LABEL, arg, AST_TYPE_NONE, 0);
	if (cmd2 == NULL)
		goto out1;

	label1 = malloc(while_end_sz());
	if (label1 == NULL)
		goto out1;

	snprintf(label1, while_end_sz(), WHILE_END_LABEL, while_no);
	arg.cd_string = label1;
	cmd1 = ast_op(line, VM_CMD_JZ, arg, AST_TYPE_NONE, 2, cmd2, expr);
	if (cmd1 == NULL)
		goto out2;

	return cmd1;
out2:
	ast_free(cmd2);
out1:
	if (label1 != NULL)
		free(label1);

	return NULL;
}

struct ast_node *ast_op_while_end(int line)
{
	struct ast_node *cmd1, *cmd2;
	union cmd_arg arg;
	char *label;

	label = malloc(while_start_sz());
	if (label == NULL)
		return NULL;

	snprintf(label, while_start_sz(), WHILE_START_LABEL, while_no);
	arg.cd_string = label;
	cmd1 = ast_op(line, VM_CMD_GOTO, arg, AST_TYPE_NONE, 0);
	if (cmd1 == NULL)
		goto out1;

	label = malloc(while_end_sz());
	if (label == NULL)
		goto out2;

	snprintf(label, while_end_sz(), WHILE_END_LABEL, while_no);
	arg.cd_string = label;

	arg.cd_string = label;
	cmd2 = ast_op(line, VM_CMD_LABEL, arg, AST_TYPE_NONE, 1, cmd1);
	if (cmd2 == NULL)
		goto out2;

	return cmd2;
out2:
	ast_free(cmd1);
out1:
	if(label)
		free(label);
	return NULL;
}


void ast_free(struct ast_node *root)
{
	int i;
	int rc;

//	printf("start free %p\n", root);
	for(i = 0; i < root->an_nargs; i++) {
		ast_free(root->an_arg[i]);
	}

	/** XXX move into vm_compile */
	if (vm_cmd_want_string(root->an_cmd))
		free(root->an_local.cd_string);

	free(root);
}


int ast_encode(struct vm_program *vprg, struct ast_node *root)
{
	int i;
	int rc;

//	printf("start encode %p\n", root);
	for(i = 0; i < root->an_nargs; i++) {
		rc = ast_encode(vprg, root->an_arg[i]);
		if (rc)
			goto out;
	}

	rc = vm_encode(vprg, root->an_line, root->an_cmd, root->an_local);
out:
	return rc;
}


/******************/
static struct vm_program *vprg = NULL;

int procedure_start(char *name)
{
	int ret;

	ret = vm_program_init(&vprg, name);
	if (ret)
		return ret;

	ret = vm_label_resolve(vprg, START_LABEL);

	return ret;
}

struct vm_program *procedure_current(void)
{
	return vprg;
}

int procedure_end(void)
{
	int ret;

	ret = vm_label_resolve(vprg, END_LABEL);
	if (ret)
		return ret;

	ret = vm_program_check(vprg);

	return ret;
}
