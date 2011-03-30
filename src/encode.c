#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "encode.h"

/**
 abstract syntax tree node 
*/
struct ast_node {
	enum vm_cmd      an_cmd;   /** < VM command */
	enum ast_type    an_type;  /** < node type */
	int              an_line;  /** < line for syntax node */
	struct vm_label	 an_label; /** < label in that line */
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

	printf("add op %d - %d = %p\n", cmd, type, node);
	return node;
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

	arg.cd_string = END_LABEL;
	cmd1 = ast_op(line, exp == VM_RET_OK ? VM_CMD_JNZ : VM_CMD_JZ, 
		      arg, AST_TYPE_NONE, 1, cmd3);
	if (cmd1 == NULL)
		goto error;

	arg.cd_string = END_LABEL;
	cmd3 = ast_op(line, VM_CMD_UP, 
		      arg, AST_TYPE_NONE, 1, cmd1);
	if (cmd3 == NULL)
		goto error;

	return cmd1;
error:
	ast_free(cmd1);
	ast_free(cmd2);
	ast_free(cmd3);
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

	if (root->an_cmd == VM_CMD_PUSHS)
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

#if 0
/****/
#define LOOP_ST_LABEL "loop_st%u:"
#define LOOP_END_LABEL "loop_end%u:"

static int loop_no = 0;
/**
 pushl $num
loop_st:
 dup 
 jz loop_end;
 */
int encode_loop_start(struct vm_program *vprg)
{
	int ret;
	char label[20];
	union cmd_arg arg;

	snprintf(label, sizeof label, LOOP_ST_LABEL, loop_no);
	ret = vm_label_resolve(vprg, label);
	if (ret)
		return ret;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_DUP, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_END_LABEL, loop_no);
	arg.cd_string = label;
	return vm_encode(vprg, VM_CMD_JZ, arg);
}
/**
 pushl 1
 sub
 goto loop_st:
loop_end:
 */
int encode_loop_end(struct vm_program *vprg)
{
	int ret;
	union cmd_arg arg;
	char label[20];

	arg.cd_long = 1;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	/* arg non used */
	ret = vm_encode(vprg, VM_CMD_SUB, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_ST_LABEL, loop_no);
	arg.cd_string = label;
	ret = vm_encode(vprg, VM_CMD_GOTO, arg);
	if (ret)
		return ret;

	snprintf(label, sizeof label, LOOP_END_LABEL, loop_no);
	loop_no ++;

	return vm_label_resolve(vprg, label);
}

/**
 dup
 push $exp
 if
 jz exit
 pop (nop)
 */
int encode_expected(struct vm_program *vprg, int exp)
{
	int ret;
	union cmd_arg arg;

	/* dup need to read status on finish */
	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_DUP, arg);
	if (ret)
		return ret;

	arg.cd_long = 0;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	/* arg not used */
	ret = vm_encode(vprg, VM_CMD_CMPL, arg);
	if (ret)
		return ret;

	arg.cd_string = END_LABEL;
	ret = vm_encode(vprg, exp == VM_RET_OK ? VM_CMD_JNZ : VM_CMD_JZ, arg);
	if (ret)
		return ret;

	/* return code same as expected, drop it */
	ret = vm_encode(vprg, VM_CMD_NOP, arg);
	if (ret)
		return ret;

	return ret;
}

/**
 create tmp_name; mkdir ; cd;
 */
int encode_make_workdir(struct vm_program *vprg, int mode)
{
	char name[256] = {0};
	int ret;
	union cmd_arg arg;

	strcpy(name, "workd-XXXXXXX");
	arg.cd_string = name;
	ret = vm_encode(vprg, VM_CMD_PUSHS, arg);
	if (ret < 0)
		return ret;

	arg.cd_call = VM_SYS_TMPNAME;
	ret = vm_encode(vprg, VM_CMD_CALL, arg);
	if (ret < 0)
		return ret;

	ret = vm_encode(vprg, VM_CMD_DUP, arg);
	if (ret)
		return ret;

	arg.cd_long = mode;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;

	arg.cd_call = VM_MD_CALL_MKDIR;
	ret = vm_encode(vprg, VM_CMD_CALL, arg);
	if (ret < 0)
		return ret;

	ret = encode_expected(vprg, VM_RET_OK);
	if (ret < 0)
		return ret;

	arg.cd_call = VM_MD_CALL_CD;
	ret = vm_encode(vprg, VM_CMD_CALL, arg);
	if (ret < 0)
		return ret;

	ret = encode_expected(vprg, VM_RET_OK);
	if (ret < 0)
		return ret;

	return 0;
}
#endif


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
#if 0
	union cmd_arg arg;

	/* program return code if finished ok */
	arg.cd_long = 0;
	ret = vm_encode(vprg, VM_CMD_PUSHL, arg);
	if (ret)
		return ret;
#endif
	ret = vm_label_resolve(vprg, END_LABEL);
	if (ret)
		return ret;

	ret = vm_program_check(vprg);

	return ret;
}
