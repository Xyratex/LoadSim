%{
#include <stdio.h>
#include <string.h>
#include "encode.h"

void yyerror(const char *str)
{
	fprintf(stderr,"error: %s\n", str);
}

int yywrap()
{
	return 1;
}

%}

%union {
	char *strval;
	int   intval;
}

%token <strval> STRING TOK_IP TOK_ID TOK_NID
%token <intval> TOK_EXP_STATUS TOK_NUMBER

%token TOK_UNLINK_CMD TOK_CD_CMD
%token TOK_OPEN_CMD TOK_CLOSE_CMD
%token TOK_STAT_CMD TOK_SETATTR_CMD
%token TOK_MKDIR_CMD TOK_READDIR_CMD
%token TOK_SOFTLINK_CMD TOK_HARDLINK_CMD TOK_READLINK_CMD

%token TOK_UID TOK_GID
%token TOK_WAITRACE_CMD TOK_SLEEP_CMD

%token TOK_EXPECTED

%token TOK_PROC_BEGIN TOK_PROC_END
%token TOK_LOOP_BEGIN TOK_LOOP_END

%token TOK_SERVER TOK_CLIENT

%type<strval> literal quoted_name
%type<intval> expected open_flags

%%
main_commands:
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
	TOK_SERVER TOK_ID TOK_NID
	{
		int ret;
		ret = server_create($2, $3);
		free($2);
		free($3);

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
	TOK_CLIENT TOK_ID TOK_ID
	{
		int ret; 
		
		ret = client_create($2, $3);
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
	| proc_body proc_commands
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
	;

loop_begin:
	TOK_LOOP_BEGIN TOK_NUMBER
	{
		int ret;

		ret = encode_loop_start(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

loop_body:
	| proc_commands loop_body 
	;

loop_end:
	TOK_LOOP_END 
	{
		encode_loop_end(procedure_current());
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
	;

/**
 synchronize executions between two clients

 wait_race $number
 where is $number is some identifier 
 */
wait_race:
	TOK_WAITRACE_CMD TOK_NUMBER
	{
		int ret;

		ret = encode_race(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

/**
 delay executions for some time

 sleep $number
 where is $number is number of ms to delay.
 */
sleep:
	TOK_SLEEP_CMD TOK_NUMBER
	{
		int ret;

		ret = encode_sleep(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

chuid:
	TOK_UID	TOK_NUMBER
	{
		int ret;

		ret = encode_user(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

chgid:
	TOK_UID	TOK_NUMBER
	{
		int ret;

		ret = encode_group(procedure_current(), $2);
		if (ret)
			YYABORT;
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
	;

/**
 create a directory
 format of operation

 mkdir "name"
 whereis 
   "name" is quoted name of directory
 */
mkdir_cmd:
	TOK_MKDIR_CMD quoted_name
	{
		int ret;

		ret = encode_mkdir(procedure_current(), $2);
		free($2);

		if (ret)
			YYABORT;
	}

/**
 change work directory
 format of operation
 
 cd "name"
 
 whereis 
   "name" is quoted name of directory
 */
cd_cmd:
	TOK_CD_CMD quoted_name
	{
		int ret;

		ret = encode_cd(procedure_current(), $2);
		free($2);

		if (ret)
			YYABORT;
	}
	;

/**
 readdir operation
 
 */
readdir_cmd:
	TOK_READDIR_CMD quoted_name
	{
		int ret;

		ret = encode_readdir(procedure_current(), $2);
		free($2);

		if (ret)
			YYABORT;
	}

/**
 remove file of directory
 format of operation
 
 unlink "name"
 whereis 
   "name" is quoted name of directory
 */
unlink_cmd:
	TOK_UNLINK_CMD quoted_name
	{
		int ret;

		ret = encode_unlink(procedure_current(), $2);
		free($2);
		if (ret)
			YYABORT;
	}
	;

/**
 open file in directory
 format of operation is
 
 open "name" flags regnum

 where is
  name quoted file name
  open_mode - flags to open. see open(2)
  regnum - index in registers file to store file handle
 */
open_cmd:
	TOK_OPEN_CMD quoted_name open_flags TOK_NUMBER
	{
		int ret;

		ret = encode_open(procedure_current(), $2, $3, $4);
		free($2);
		if (ret)
			YYABORT;
	}
	;


open_flags:
	literal
	{
		/** verify */
		$$ = $1;
	}
	;

/**
 close file handle prevously opened by "open" command.
 format of operation is

  close $regnum

 where is 
 $regnum - index in register file where open store a file handle
*/ 
close_cmd:
	TOK_CLOSE_CMD TOK_NUMBER
	{
		int ret;

		ret = encode_close(procedure_current(), $2);
		if (ret)
			YYABORT;
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
	TOK_STAT_CMD quoted_name
	{
		int ret;

		ret = encode_stat(procedure_current(), $2);
		free($2);
		if (ret)
			YYABORT;
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
	TOK_SETATTR_CMD quoted_name TOK_NUMBER
	{
		int ret;

		ret = encode_setattr(procedure_current(), $2, $3);
		free($2);
		if (ret)
			YYABORT;
	}
	;

/**
 create softlink between md objects
 format of operation

 softlink "oldname" "newname" 
 */
mksln_cmd:
	TOK_SOFTLINK_CMD quoted_name quoted_name
	{
		int ret;

		ret = encode_softlink(procedure_current(), $2, $3);
		free($2);
		free($3);
		if (ret)
			YYABORT;
	}
	;

/**
 create hardlink between md objects
 format of operation

 hardlink "oldname" "newname" 
 */
mkhln_cmd:
	TOK_HARDLINK_CMD quoted_name quoted_name
	{
		int ret;

		ret = encode_hardlink(procedure_current(), $2, $3);
		free($2);
		free($3);
		if (ret)
			YYABORT;
	}
	;

/**
 read contents of softlink.
 format of operation
 
 readlink "name"
 */
readln_cmd:
	TOK_READLINK_CMD quoted_name
	{
		int ret;

		ret = encode_readlink(procedure_current(), $2);
		free($2);
		if (ret)
			YYABORT;
	}
	;

/**********************/
expected:
	TOK_EXPECTED TOK_EXP_STATUS
	{
		int ret;

		ret = encode_expected(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

quoted_name:
	  '"' literal '"'
	{
		$$ = $2;
	}
	;

literal:
	  STRING
	| TOK_ID
	;
%%
