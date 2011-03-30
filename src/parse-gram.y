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

%token <strval> QSTRING TOK_IP TOK_ID TOK_NID
%token <intval> TOK_EXP_STATUS TOK_NUMBER TOK_O_FLAG

%token TOK_UNLINK_CMD TOK_CD_CMD
%token TOK_OPEN_CMD TOK_CLOSE_CMD
%token TOK_STAT_CMD TOK_CHMOD_CMD TOK_CHOWN_CMD TOK_CHTIME_CMD TOK_TRUNCATE_CMD
%token TOK_MKDIR_CMD TOK_READDIR_CMD
%token TOK_SOFTLINK_CMD TOK_HARDLINK_CMD TOK_READLINK_CMD
%token TOK_RENAME

%token TOK_UID_CMD TOK_GID_CMD
%token TOK_WAITRACE_CMD TOK_SLEEP_CMD
%token TOK_TMPNAME

%token TOK_EXPECTED

%token TOK_PROC_BEGIN TOK_PROC_END
%token TOK_LOOP_BEGIN TOK_LOOP_END

%token TOK_SERVER TOK_CLIENT

%type<intval> calc_o_flags

%type<node> proc_body proc_commands 
%type<node> statements expression tmpname
%type<node> open_flags

%type<node> loop loop_body

%type<node> md_ops misc_ops md_cmd expected
%type<node> cd_cmd mkdir_cmd readdir_cmd unlink_cmd open_cmd
%type<node> close_cmd stat_cmd setattr_cmd mksln_cmd mkhln_cmd
%type<node> chmod_cmd chown_cmd chtime_cmd truncate_cmd
%type<node> readln_cmd rename_cmd

%type<node> wait_race sleep chuid chgid 


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
	TOK_SERVER TOK_ID TOK_NID QSTRING
	{
		int ret;
		ret = server_create($2, $4, $3);
		free($2);
		free($3);
		free($4);

		if (ret)
			YYABORT;
	}
	;

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

		ret = ast_encode(procedure_current(), $2);
		ast_free($2);
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
		int ret;

		ret = procedure_end();
		if (ret)
			YYABORT;
	}
	;

proc_body:
	proc_commands { $$ = $1; }
	| proc_body proc_commands
	{
		union cmd_arg arg;
		arg.cd_long = 0;

		$$ = ast_op(yylloc.first_line, VM_CMD_NOP, arg, AST_TYPE_NONE, 2, $1, $2);
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
	| loop
	{
		$$ = $1;
	}
	;


/**
 loop format is

 do
  .. come operation
 loop $number

 where is $number is number of runs
*/
loop:
	loop_begin loop_body loop_end
	{
		union cmd_arg arg;
		arg.cd_long = 0;

		$$ = ast_op(yylloc.first_line, VM_CMD_NOP, arg, AST_TYPE_NONE, 1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;

loop_begin:
	TOK_LOOP_BEGIN TOK_NUMBER
	{
	}
	;

loop_body:
	proc_commands
	| loop_body proc_commands
	{
		union cmd_arg arg;
		arg.cd_long = 0;

		$$ = ast_op(yylloc.first_line, VM_CMD_NOP, arg, AST_TYPE_NONE, 2, $1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;

loop_end:
	TOK_LOOP_END 
	{
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
		union cmd_arg arg;
		arg.cd_long = 0;

		$$ = ast_op(yylloc.first_line, VM_CMD_NOP, arg, AST_TYPE_NONE,
			    2, $1, $2);
		if($$ == NULL)
			YYABORT;

		printf("md_ops %p - / %p %p /\n", $$, $1, $2);
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
				      4, $2, $3, $4, $5);
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
			arg.cd_call = VM_MD_CALL_CHMOD;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_NUMBER,
				      2, $2, $3);
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
				      3, $2, $3, $5);
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
				      2, $2, $3);
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
				      2, $2, $3);
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
				      2, $2, $3);
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
				      2, $2, $3);
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
				      2, $2, $3);
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
		union cmd_arg arg;
		arg.cd_long = 0;

		$$ = ast_op(yylloc.first_line, VM_CMD_NOP, arg, AST_TYPE_NONE, 2, $1, $2);
		if($$ == NULL)
			YYABORT;
	}
	;
	
expression:
	QSTRING		{ union cmd_arg arg; 
			arg.cd_string = $1; 
			$$ = ast_op(yylloc.first_line, VM_CMD_PUSHS, arg, AST_STRING, 0); 
			}
	| TOK_NUMBER	{ union cmd_arg arg; 
			arg.cd_long = $1;
			$$ = ast_op(yylloc.first_line, VM_CMD_PUSHL, arg, AST_NUMBER, 0);
			}
	| '(' expression ')' { $$ = $2; }
	| tmpname 	{ $$ = $1; }
	;

tmpname: TOK_TMPNAME expression
	{
		int ret;
		struct ast_node *call;
		union cmd_arg arg;
	
		ret = ast_check_type($2, AST_STRING);
		if (ret == 1) {
			arg.cd_call = VM_SYS_TMPNAME;
			call = ast_op(yylloc.first_line,
				      VM_CMD_CALL, arg, AST_STRING,
				      1, $2);
		}
		if ((ret < 1) || (call == NULL))
			YYABORT;
		$$ = call;
	}
	;

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
