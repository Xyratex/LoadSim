%{
#include <stdio.h>
#include <string.h>
#include "encode.h"


void yyerror(const char *str);

%}

/* Location tracking.  */
%locations

%union {
	char *strval;
	int   intval;
	struct ast_node *node;
}

%token <strval> QSTRING TOK_IP TOK_ID TOK_NID TOK_INTERNAL_FUNC
%token <intval> TOK_EXP_STATUS TOK_NUMBER TOK_O_FLAG TOK_REGISTER

%token TOK_UNLINK_CMD TOK_CD_CMD
%token TOK_OPEN_CMD TOK_CLOSE_CMD
%token TOK_STAT_CMD TOK_CHMOD_CMD TOK_CHOWN_CMD TOK_CHTIME_CMD TOK_TRUNCATE_CMD
%token TOK_MKDIR_CMD TOK_READDIR_CMD
%token TOK_SOFTLINK_CMD TOK_HARDLINK_CMD TOK_READLINK_CMD
%token TOK_RENAME

%token TOK_UID_CMD TOK_GID_CMD
%token TOK_WAITRACE_CMD TOK_SLEEP_CMD
%token TOK_TMPNAME TOK_PRINTF

%token TOK_EXPECTED

%token TOK_PROC_BEGIN TOK_PROC_END
%token TOK_LOOP_BEGIN TOK_LOOP_END
%token TOK_WHILE_BEGIN TOK_WHILE_END

%token TOK_SERVER TOK_CLIENT
%token TOK_FS_LOCAL TOK_FS_LUSTRE

%type<intval> calc_o_flags

%type<node> proc_body proc_end proc_commands 
%type<node> statements expression var
%type<node> open_flags

%type<node> loops
%type<node> while_loop while_begin while_body while_end

%type<node> md_ops misc_ops md_cmd expected
%type<node> cd_cmd mkdir_cmd readdir_cmd unlink_cmd open_cmd
%type<node> close_cmd stat_cmd setattr_cmd mksln_cmd mkhln_cmd
%type<node> chmod_cmd chown_cmd chtime_cmd truncate_cmd
%type<node> readln_cmd rename_cmd

%type<node> wait_race sleep chuid chgid 
%type<node> tmpname printf
%type<node> arg_list 

%type<node> math 
%type<node> logical
%token TOK_L_EQ TOK_L_NE TOK_L_GR TOK_L_LOW TOK_L_GE TOK_L_LE

%left	'|'

%%
main_commands:
	/* empty */
	| main_commands main_command
	;

main_command:
	  server_set
	| client_set
	| procedure
	;

/**
 define server settings
 
 server mds-id nid
 where is
 'server' is reserved word
 'mds-id' is server uuid
 'nid' is network address to connect
 */
server_set:
	TOK_SERVER server_param
	;

server_param:
	lustre_server 
	| local_server
	;


lustre_server:
	TOK_FS_LUSTRE TOK_NID QSTRING
	{
		int ret;
		ret = server_create_lustre($2, $3);
		free($2);
		free($3);

		if (ret)
			YYABORT;
	}
	;

local_server:
	TOK_FS_LOCAL QSTRING
	{
		int ret;

		ret = server_create_local($2);
		free($2);

		if (ret)
			YYABORT;
	}

/**
 define one client used on test.
 example
 
 client cli-name test-name
 where is 
 'client' is reserved word.
 'cli-name' is unique name (identifier) assigned to the client
 'test-name' is name of some group of operations.
 */
client_set:
	TOK_CLIENT QSTRING TOK_ID
	{
		int ret;

		ret = clients_create($2, $3);
		free($2);
		free($3);

		if(ret)
			YYABORT;
	}
       ;
/**
 define a procedure - group of operations.
 group can be assigned to one or more clients.

 example
 
 procedure $test-name
  .. some operations ..
 endproc

 $test-name used in client command.
*/
procedure:
	proc_begin proc_body proc_end
	{
		int ret;
		struct ast_node *proc;

		proc = ast_op_link(yylloc.first_line, $2, $3);
		if(proc == NULL)
			YYABORT;

		ret = ast_encode(procedure_current(), proc);
		ast_free(proc);
		if (ret)
			YYABORT;
		
		ret = procedure_end();
		if (ret)
			YYABORT;
	}
	;

proc_begin:
	TOK_PROC_BEGIN TOK_ID 
	{
		int ret;

		ret = procedure_start($2);
		free($2);
		if (ret)
			YYABORT;
	}
	;

proc_end:
	TOK_PROC_END
	{
		/* return code */
		union cmd_arg arg; 
		arg.cd_long = 0;
		$$ = ast_op(yylloc.first_line, VM_CMD_PUSHL, arg, AST_NUMBER, 0);
		if ($$ == NULL)
			YYABORT;
	}
	;

proc_body:
	proc_commands
	| proc_body proc_commands
	{
		$$ = ast_op_link(yylloc.first_line, $1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;

/**
 groups commands used in procedure
 - metadata operations
 - loop
 - synchronization and other not related to MD commands
 */
proc_commands:
	  md_ops
	| misc_ops
	| loops
	| var
	{
		$$ = $1;
	}
	;


loops: 
	while_loop
	{
		$$ = $1;
	}

/**
 "while" loop format is

 while ($statement)
  .. come operation
 endw

 where is $number is number of runs
*/
while_loop:
	while_begin while_body while_end
	{
		union cmd_arg arg;
		arg.cd_long = 0;

		$$ = ast_op(yylloc.first_line, VM_CMD_NOP, arg, AST_TYPE_NONE, 3, $1, $2,$3);
		if($$ == NULL)
			YYABORT;
	}
	;

while_begin:
	TOK_WHILE_BEGIN expression
	{
		$$ = ast_op_while_start(yylloc.first_line, $2);
		if($$ == NULL)
			YYABORT;
	}
	;

while_body:
	proc_commands
	| while_body proc_commands
	{
		$$ = ast_op_link(yylloc.first_line, $1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;

while_end:
	TOK_WHILE_END
	{
		$$ = ast_op_while_end(yylloc.first_line);
		if ($$ == NULL)
			YYABORT;
	}
	;

/**
 group of misc operations.
 now it
  - synchronization between threads, that is wait_race.
  - delay execution for some time
*/
misc_ops:
	  wait_race
	| sleep
	| chuid
	| chgid
	{
		$$ = $1;
	}
	;

/**
 synchronize executions between two clients

 wait_race $number
 where is $number is some identifier 
 */
wait_race:
	TOK_WAITRACE_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_NUMBER);
		if (ret == 1) {
			arg.cd_call = VM_SYS_RACE;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 delay executions for some time

 sleep $number
 where is $number is number of ms to delay.
 */
sleep:
	TOK_SLEEP_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_NUMBER);
		if (ret == 1) {
			arg.cd_call = VM_SYS_SLEEP;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

chuid:
	TOK_UID_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_NUMBER);
		if (ret == 1) {
			arg.cd_call = VM_SYS_USER;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

chgid:
	TOK_GID_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_NUMBER);
		if (ret == 1) {
			arg.cd_call = VM_SYS_GROUP;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;


/**
 format of metadata operating
 
 $md_operation 'expected' [OK|FAIL]
 where is 
 $md_operation is one of metadata specific operations.
 'expected' is reserved word
 OK - operation want to be finished successfully
 FAIL - operation is wait to be finished with errro
 
 if operation finished with unexpected result, execution finished 
 and error code returned to application.
 */
md_ops:
	md_cmd expected
	{
		$$ = ast_op_link(yylloc.first_line, $1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;

/**
 list of metadata specific operations.
 currently supported
 - change work directory
 - create a directory
 - read directory contents
 - remove file or directory
 - open a file
 - close a file
 - get attributes of a file
 - set attributes of a file
 */
md_cmd:
	  cd_cmd
	| mkdir_cmd
	| readdir_cmd
	| unlink_cmd
	| open_cmd
	| close_cmd
	| stat_cmd
	| setattr_cmd
	| mksln_cmd
	| mkhln_cmd
	| readln_cmd
	| rename_cmd
	{
		$$ = $1;
	}
	;

/**
 create a directory
 format of operation

 mkdir "name" mode
 whereis 
   "name" is quoted name of directory
   mode is
 */
mkdir_cmd:
	TOK_MKDIR_CMD statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_NUMBER);
		if (ret == 2) {
			arg.cd_call = VM_MD_CALL_MKDIR;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $2, $3);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;

		$$ = call;
	}
	;
/**
 change work directory
 format of operation
 
 cd "name"
 
 whereis 
   "name" is quoted name of directory
 */
cd_cmd:
	TOK_CD_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		if (ret == 1) {
			arg.cd_call = VM_MD_CALL_CD;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 readdir operation
 
 */
readdir_cmd:
	TOK_READDIR_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		if (ret == 1) {
			arg.cd_call = VM_MD_CALL_READIR;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 remove file of directory
 format of operation
 
 unlink "name"
 whereis 
   "name" is quoted name of directory
 */
unlink_cmd:
	TOK_UNLINK_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		if (ret == 1) {
			arg.cd_call = VM_MD_CALL_UNLINK;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 open file in directory
 format of operation is
 
 open "name" flags mode regnum

 where is
  name quoted file name
  open_mode - flags to open. see open(2)
  mode - 
  regnum - index in registers file to store file handle
 */
open_cmd:
	TOK_OPEN_CMD statements open_flags statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_NUMBER);
		ret += ast_check_type($4, AST_NUMBER);
		ret += ast_check_type($5, AST_NUMBER);
		if (ret == 4) {
			arg.cd_call = VM_MD_CALL_OPEN;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      4, $5, $4, $3, $2);
		}
		if ((ret < 4) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

open_flags:
	calc_o_flags 
	{
		union cmd_arg arg; 
		arg.cd_long = $1;
		$$ = ast_op(yylloc.first_line, VM_CMD_PUSHL, arg, AST_NUMBER, 0);
		if ($$ == NULL)
			YYABORT;
	}
	;

calc_o_flags:
	TOK_O_FLAG	{ $$ = $1; }
	| calc_o_flags '|' TOK_O_FLAG { $$ = $1 | $3; }
	;

/**
 close file handle prevously opened by "open" command.
 format of operation is

  close $regnum

 where is 
 $regnum - index in register file where open store a file handle
*/ 
close_cmd:
	TOK_CLOSE_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_NUMBER);
		if (ret == 1) {
			arg.cd_call = VM_MD_CALL_CLOSE;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 get attributes of a file
 format of operation

 stat "name"
 
 where is 
  "name" quoted file name
 */
stat_cmd:
	TOK_STAT_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		if (ret == 1) {
			arg.cd_call = VM_MD_CALL_STAT;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 set attributes of file
 format of operation
 
 setattr "name" mode
 where is 
  "name" quoted file name
   mode is numberic value of file mode bits
 */
setattr_cmd:
	chmod_cmd
	| chown_cmd
	| chtime_cmd
	| truncate_cmd
	{
		$$ = $1;
	}
	;

chmod_cmd:
	TOK_CHMOD_CMD statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_NUMBER);
		if (ret == 2) {
			/* read mode / name */
			arg.cd_call = VM_MD_CALL_CHMOD;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $3, $2);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

chown_cmd:
	TOK_CHOWN_CMD statements statements ':' statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_NUMBER);
		ret += ast_check_type($5, AST_NUMBER);
		if (ret == 3) {
			arg.cd_call = VM_MD_CALL_CHOWN;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      3, $5, $3, $2);
		}
		if ((ret < 3) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

chtime_cmd:
	TOK_CHTIME_CMD statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_NUMBER);
		if (ret == 2) {
			arg.cd_call = VM_MD_CALL_CHTIME;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $3, $2);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

truncate_cmd:
	TOK_TRUNCATE_CMD statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_NUMBER);
		if (ret == 2) {
			arg.cd_call = VM_MD_CALL_TRUNCATE;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $3, $2);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 create softlink between md objects
 format of operation

 softlink "oldname" "newname" 
 */
mksln_cmd:
	TOK_SOFTLINK_CMD statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_STRING);
		if (ret == 2) {
			arg.cd_call = VM_MD_CALL_SOFTLINK;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $3, $2);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 create hardlink between md objects
 format of operation

 hardlink "oldname" "newname" 
 */
mkhln_cmd:
	TOK_HARDLINK_CMD statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_STRING);
		if (ret == 2) {
			arg.cd_call = VM_MD_CALL_HARDLINK;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $3, $2);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

/**
 read contents of softlink.
 format of operation
 
 readlink "name"
 */
readln_cmd:
	TOK_READLINK_CMD statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		if (ret == 1) {
			arg.cd_call = VM_MD_CALL_READLINK;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

rename_cmd:
	TOK_RENAME statements statements
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;

		ret = ast_check_type($2, AST_STRING);
		ret += ast_check_type($3, AST_STRING);
		if (ret == 2) {
			arg.cd_call = VM_MD_CALL_RENAME;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $3, $2);
		}
		if ((ret < 2) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

expected:
	TOK_EXPECTED TOK_EXP_STATUS
	{
		struct ast_node *ret;

		ret = ast_op_expected(yylloc.first_line, $2);
		if (ret == NULL)
			YYABORT;
		$$ = ret;
	}
	;

statements:
	expression
	| statements expression
	{
		$$ = ast_op_link(yylloc.first_line, $1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;
	
expression:
	QSTRING	
		{
		union cmd_arg arg; 
		arg.cd_string = $1; 
		$$ = ast_op(yylloc.first_line, VM_CMD_PUSHS, arg, AST_STRING, 0);
		if ($$ == NULL)
			YYABORT;
		}
	| TOK_NUMBER
		{ 
		union cmd_arg arg; 
		arg.cd_long = $1;
		$$ = ast_op(yylloc.first_line, VM_CMD_PUSHL, arg, AST_NUMBER, 0);
		if ($$ == NULL)
			YYABORT;
		}
	| TOK_REGISTER  {
		/* read register */
		union cmd_arg arg; 

		arg.cd_long = $1;
		$$ = ast_op(yylloc.first_line, VM_CMD_GETR, arg, AST_REGISTER, 0);
		if ($$ == NULL)
			YYABORT;
		}
	| '(' expression ')' { $$ = $2; }
	| math { $$ = $1; }
	| logical { $$ = $1; }
	| tmpname 	{ $$ = $1; }
	| printf	{ $$ = $1; }
	| TOK_INTERNAL_FUNC
		{
		$$ = ast_op_internal(yylloc.first_line, $1);
		free($1);
		if ($$ == NULL)
			YYABORT;
		}
	;

math:
	expression '+' expression
		{
		union cmd_arg arg; 
		struct ast_node *call = NULL;
		int ret;

		ret = ast_check_type($1, AST_NUMBER);
		ret += ast_check_type($3, AST_NUMBER);
		if (ret == 2) {
			arg.cd_long = 0;
			call = ast_op(yylloc.first_line, VM_CMD_ADD, arg, AST_NUMBER, 2, $3, $1);
		}
		if (call == NULL)
			YYABORT;
		$$ = call;
		}
	| expression '-' expression
		{
		union cmd_arg arg; 
		struct ast_node *call = NULL;
		int ret;

		ret = ast_check_type($1, AST_NUMBER);
		ret += ast_check_type($3, AST_NUMBER);
		if (ret == 2) {
			arg.cd_long = 0;
			call = ast_op(yylloc.first_line, VM_CMD_SUB, arg, AST_NUMBER, 2, $3, $1);
		}
		if (call == NULL)
			YYABORT;
		$$ = call;
		}
        ;

logical:
	expression TOK_L_EQ expression 
	{ 
		$$ = ast_op_eq(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	} 
	| expression TOK_L_NE expression 
	{ 
		$$ = ast_op_ne(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	} 
	| expression TOK_L_GR expression 
	{ 
		$$ = ast_op_gr(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	}
	| expression TOK_L_LOW expression
	{ 
		$$ = ast_op_low(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	}
	| expression TOK_L_GE expression
	{ 
		$$ = ast_op_ge(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	}
	| expression TOK_L_LE expression
	{ 
		$$ = ast_op_le(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	}
	;

var:
	TOK_REGISTER '=' expression
	{
		 /* assign to the register */
		union cmd_arg arg; 

		arg.cd_long = $1;
		$$ = ast_op(yylloc.first_line, VM_CMD_PUTR, arg, AST_REGISTER, 1, $3);
		if ($$ == NULL)
			YYABORT;
	}
	;
tmpname: TOK_TMPNAME expression
	{
		int ret;
		struct ast_node *call = NULL;
		struct ast_node *buff;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		buff = ast_buffer(yylloc.first_line, VM_STRING_SZ);
		if (ret == 1 && buff) {
			arg.cd_call = VM_SYS_TMPNAME;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_STRING,
				      2, $2, buff);
		}
		if (call == NULL) {
			if (buff != NULL)
				ast_free(buff);
			YYABORT;
		}
		$$ = call;
	}
	;

printf: TOK_PRINTF expression '[' arg_list ']'
	{
		int ret;
		struct ast_node *call = NULL;
		struct ast_node *buff;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		buff = ast_buffer(yylloc.first_line, VM_STRING_SZ);
		if (ret == 1 && buff) {
			arg.cd_call = VM_SYS_PRINTF;
			/** need to put format expression as second argument
			  to get it from stack first 
			  */
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_STRING,
				      3, $4, $2, buff);
		}
		if (call == NULL) {
			if (buff != NULL)
				ast_free(buff);
			YYABORT;
		}
		$$ = call;
	}
	;

arg_list:
	expression
	| arg_list ',' expression
	{
		$$ = ast_op_link(yylloc.first_line, $3, $1);
		if ($$ == NULL)
			YYABORT;
	}

%%

extern char *yytext;

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s - between ", str);
	YY_LOCATION_PRINT(stderr, yylloc);
	fprintf(stderr, "(line.column-line.column) - text: %s\n", yytext);
}

int yywrap()
{
	return 1;
}
