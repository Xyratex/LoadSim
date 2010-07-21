%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse-gram.h"
#include "vm_defs.h"
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
CLOSE		[c|C][l|L][o|O][s|S][e|E]
MKDIR		[m|M][k|K][d|D][i|I][r|R]
STAT		[s|S][t|T][a|A][t|T]
READDIR		[r|R][e|E][a|A][d|D][d|D][i|I][r|R]
SETATTR		[s|S][e|E][t|T][a|A][t|T][t|T][r|R]
SOFTLINK	[s|S][o|O][f|F][t|T][l|L][i|I][n|N][k|K]
HARDLINK	[h|H][a|A][r|R][d|D][l|L][i|I][n|N][k|K]
READLINK	[r|R][e|E][a|A][d|D][l|L][i|I][n|N][k|K]

UID		[u|U][i|I][d|D]
GID		[g|G][i|I][d|D]
WAITRACE	[w|W][a|A][i|I][t|T][_][r|R][a|A][c|C][e|E]
SLEEP		[s|S][l|L][e|E][e|E][p|P]

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
{CLOSE}		return TOK_CLOSE_CMD;
{STAT}		return TOK_STAT_CMD;
{SETATTR}	return TOK_SETATTR_CMD;
{SOFTLINK}	return TOK_SOFTLINK_CMD;
{HARDLINK}	return TOK_HARDLINK_CMD;
{READLINK}	return TOK_READLINK_CMD;

{SLEEP}		return TOK_SLEEP_CMD;
{WAITRACE}	return TOK_WAITRACE_CMD;
{UID}		return TOK_UID;
{GID}		return TOK_GID;

{CLIENT}	return TOK_CLIENT;
{SERVER}	return TOK_SERVER;

{IP}@{LNDS}	{ yylval.strval = strdup(yytext); return TOK_NID; }
{ID}		{ yylval.strval = strdup(yytext); return TOK_ID; }
{IP}		{ yylval.strval = strdup(yytext); return TOK_IP; }

{digit}+	{ yylval.intval = atoi(yytext); return TOK_NUMBER; }
{STRING}	{ yylval.strval = yytext; return STRING; }

.		return yytext[0];
%%

