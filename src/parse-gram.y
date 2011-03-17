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
%token TOK_MKWD

%token TOK_EXPECTED

%token TOK_PROC_BEGIN TOK_PROC_END
%token TOK_LOOP_BEGIN TOK_LOOP_END

%token TOK_SERVER TOK_CLIENT

%type<intval> expected open_flags

%left	'|'

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
	| mkwd
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
	TOK_UID_CMD TOK_NUMBER
	{
		int ret;

		ret = encode_user(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

chgid:
	TOK_GID_CMD TOK_NUMBER
	{
		int ret;

		ret = encode_group(procedure_current(), $2);
		if (ret)
			YYABORT;
	}
	;

mkwd:
	TOK_MKWD TOK_NUMBER
	{
		int ret;

		ret = encode_make_workdir(procedure_current(), $2);
		if (ret)
			YYABORT;
	}

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
	| rename_cmd
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
	TOK_MKDIR_CMD QSTRING TOK_NUMBER
	{
		int ret;

		ret = encode_mkdir(procedure_current(), $2, $3);
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
	TOK_CD_CMD QSTRING
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
	TOK_READDIR_CMD QSTRING
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
	TOK_UNLINK_CMD QSTRING
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
 
 open "name" flags mode regnum

 where is
  name quoted file name
  open_mode - flags to open. see open(2)
  mode - 
  regnum - index in registers file to store file handle
 */
open_cmd:
	TOK_OPEN_CMD QSTRING open_flags TOK_NUMBER TOK_NUMBER
	{
		int ret;

		ret = encode_open(procedure_current(), $2, $3, $4, $5);
		free($2);
		if (ret)
			YYABORT;
	}
	;


open_flags:
	TOK_O_FLAG	{ $$ = $1; }
	| open_flags '|' TOK_O_FLAG { $$ = $1 | $3; }
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
	TOK_STAT_CMD QSTRING
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
	chmod_cmd
	| chown_cmd
	| chtime_cmd
	| truncate_cmd
	;

chmod_cmd:
	TOK_CHMOD_CMD QSTRING TOK_NUMBER
	{
		int ret;

		ret = encode_chmod(procedure_current(), $2, $3);
		free($2);
		if (ret)
			YYABORT;
	}
	;

chown_cmd:
	TOK_CHOWN_CMD QSTRING TOK_NUMBER ':' TOK_NUMBER
	{
		int ret;

		ret = encode_chown(procedure_current(), $2, $3, $5);
		free($2);
		if (ret)
			YYABORT;
	}
	;

chtime_cmd:
	TOK_CHTIME_CMD QSTRING TOK_NUMBER
	{
		int ret;

		ret = encode_chtime(procedure_current(), $2, $3);
		free($2);
		if (ret)
			YYABORT;
	}
	;

truncate_cmd:
	TOK_TRUNCATE_CMD QSTRING TOK_NUMBER
	{
		int ret;

		ret = encode_truncate(procedure_current(), $2, $3);
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
	TOK_SOFTLINK_CMD QSTRING QSTRING
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
	TOK_HARDLINK_CMD QSTRING QSTRING
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
	TOK_READLINK_CMD QSTRING
	{
		int ret;

		ret = encode_readlink(procedure_current(), $2);
		free($2);
		if (ret)
			YYABORT;
	}
	;

rename_cmd:
	TOK_RENAME QSTRING QSTRING
	{
		int ret;

		ret = encode_rename(procedure_current(), $2, $3);
		free($2);
		free($3);
		if (ret)
			YYABORT;

	}

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
