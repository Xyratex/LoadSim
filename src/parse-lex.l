%x str

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "parse-gram.h"
#include "debug.h"
#include "vm_defs.h"

enum {
	MAX_STRING_SIZE = 256
};

static char string_buf[MAX_STRING_SIZE];
static int string_idx;

static inline void string_start(void)
{
	string_idx = 0;
}

static inline int string_add(char c)
{
	string_buf[string_idx ++] = c;
	return (string_idx == MAX_STRING_SIZE);
}

#define str_add(c)	do {			\
			if (string_add(c)) {	\
				err_print("string size overflow\n");	\
				yyterminate();	\
			}			\
			} while (0);

%}


BLANK           [ \t]
CR              \n
alpha		[a-zA-Z]
digit		[0-9]
special		[()+\|\?\*,]

IP		{digit}{1,3}\.{digit}{1,3}\.{digit}{1,3}\.{digit}{1,3}

FAIL		[f|F][a|A][i|I][l|L]
OK		[o|O][k|K]

EXPECT_STAT	{OK}|{FAIL}

LO_NID		[l|L][o|O]
TCP_NID		[t|T][c|C][p|P]
IB_NID		[i|I][b|B]
LNDS		{TCP_NID}|{LO_NID}|{IB_NID}

PROCEDURE	[p|P][r|R][o|O][c|C][e|E][d|D][u|U][r|R][e|E]
ENDPROC		[e|E][n|N][d|D][p|P][r|R][o|O][c|C]
CLIENT		[c|C][l|L][i|I][e|E][n|N][t|T]
SERVER		[s|S][e|E][r|R][v|V][e|E][r|R]

LOOP_BEGIN	[l|L][o|O][o|O][p|P]
LOOP_END	[e|E][n|N][d|D][l|L][o|O][o|O][p|P]

CD		[c|C][d|D]
UNLINK		[u|U][n|N][l|L][i|I][n|N][k|K]
OPEN		[o|O][p|P][e|E][n|N]

O_RDONLY	[O|o][_][R|r][D][O][N][L][Y]
O_WRONLY	[O|o][_][W|w][R][O][N][L][Y]
O_RDWR		[O|o][_][R|r][D][W][R]
O_APPEND	[O|o][_][A|a][P][P][E][N][D]
O_CREATE	[O|o][_][C|c][R][E][A][T]

CLOSE		[c|C][l|L][o|O][s|S][e|E]
MKDIR		[m|M][k|K][d|D][i|I][r|R]
STAT		[s|S][t|T][a|A][t|T]
READDIR		[r|R][e|E][a|A][d|D][d|D][i|I][r|R]
CHMOD		[c][h][m][o][d]
CHOWN		[c][h][o][w][n]
CHTIME		[c][h][t][t][i][m][e]
TRUNCATE	[t][r][u][n][c][a][t][e]
SOFTLINK	[s|S][o|O][f|F][t|T][l|L][i|I][n|N][k|K]
HARDLINK	[h|H][a|A][r|R][d|D][l|L][i|I][n|N][k|K]
READLINK	[r|R][e|E][a|A][d|D][l|L][i|I][n|N][k|K]

UID		[u|U][i|I][d|D]
GID		[g|G][i|I][d|D]
WAITRACE	[w|W][a|A][i|I][t|T][_][r|R][a|A][c|C][e|E]
SLEEP		[s|S][l|L][e|E][e|E][p|P]

O_PREFIX	[o|O][_]{alpha}+

EXPECTED	[e|E][x|X][p|P][e|E][c|C][t|T][e|E][d|D]

STRING		({alpha}|{digit}|{special})+
ID		{alpha}({alpha}|{digit})+

%%
{CR}		/* ignore single EOL */
{BLANK}+	/* ignore spaces */;

{PROCEDURE}	return TOK_PROC_BEGIN;
{ENDPROC}	return TOK_PROC_END;

{LOOP_BEGIN}	return TOK_LOOP_BEGIN;
{LOOP_END}	return TOK_LOOP_END;

{EXPECTED}	return TOK_EXPECTED;
{EXPECT_STAT}	{
		    if (strcasecmp(yytext, "OK") == 0)
			yylval.intval = VM_RET_OK;
		    else
			yylval.intval = VM_RET_FAIL;

		    return TOK_EXP_STATUS;
		}

{CD}		return TOK_CD_CMD;
{MKDIR}		return TOK_MKDIR_CMD;
{READDIR}	return TOK_READDIR_CMD;
{UNLINK}	return TOK_UNLINK_CMD;
{OPEN}		return TOK_OPEN_CMD;
{O_RDONLY}	{ yylval.intval = O_RDONLY; return TOK_O_FLAG; }
{O_WRONLY}	{ yylval.intval = O_WRONLY; return TOK_O_FLAG; }
{O_RDWR}	{ yylval.intval = O_RDWR; return TOK_O_FLAG; }
{O_APPEND}	{ yylval.intval = O_APPEND; return TOK_O_FLAG; }
{O_CREATE}	{ yylval.intval = O_CREAT; return TOK_O_FLAG; }
{CLOSE}		return TOK_CLOSE_CMD;
{STAT}		return TOK_STAT_CMD;
{CHMOD}		return TOK_CHMOD_CMD;
{CHOWN}		return TOK_CHOWN_CMD;
{CHTIME}	return TOK_CHTIME_CMD;
{TRUNCATE}	return TOK_TRUNCATE_CMD;
{SOFTLINK}	return TOK_SOFTLINK_CMD;
{HARDLINK}	return TOK_HARDLINK_CMD;
{READLINK}	return TOK_READLINK_CMD;

{SLEEP}		return TOK_SLEEP_CMD;
{WAITRACE}	return TOK_WAITRACE_CMD;
{UID}		return TOK_UID_CMD;
{GID}		return TOK_GID_CMD;

{CLIENT}	return TOK_CLIENT;
{SERVER}	return TOK_SERVER;

{IP}@{LNDS}	{ yylval.strval = strdup(yytext); return TOK_NID; }
{IP}		{ yylval.strval = strdup(yytext); return TOK_IP; }

{digit}+	{
		    char *err;

		    errno = 0;
		    yylval.intval = strtol(yytext, &err, 0);
		    if ((*err != '\0') || (errno != 0)) {
				err_print("bad number %s\n", yytext);
				yyterminate();
		    }
		    return TOK_NUMBER;
		}

\"		{ string_start(); BEGIN(str); }
<str>\"		{ /* saw closing quote - all done */
		    BEGIN(INITIAL);
		    str_add('\0');
		    yylval.strval = strdup(string_buf);
		    return QSTRING;
		}
<str>\n		{ /* error - unterminated string constant */
		    err_print("unterminated string\n");
		    yyterminate();
		}
<str>\\[0-7]{1,3} { /* octal escape sequence */
		int result;
		(void) sscanf( yytext + 1, "%o", &result );
		if (result > 0xff) {
			/* error, constant is out-of-bounds */
			err_print("out-of-bound\n");
			yyterminate();
		 }
		str_add(result);
		}
<str>\\[0-9]+	{
		/* generate error - bad escape sequence; something
		 * like '\48' or '\0777777'
		 */
		err_print("bad escape sequence\n");
		yyterminate();
		}
<str>\\n	str_add('\n');
<str>\\t	str_add('\t');
<str>\\r	str_add('\r');
<str>\\b	str_add('\b');
<str>\\f	str_add('\f');
<str>\\(.|\n)  	str_add(yytext[1]);
<str>[^\\\n\"]+	{
		char *yptr = yytext;
		while ( *yptr ) {
			str_add(*yptr ++);
		}
		}

\|		{ return yytext[0]; }
{ID}		{ yylval.strval = strdup(yytext); return TOK_ID; }

%%

