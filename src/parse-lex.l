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

PROCEDURE	[p][r][o][c][e][d][u][r][e]
ENDPROC		[e][n][d][p][r][o][c]
CLIENT		[c][l][i][e][n][t]
SERVER		[s][e][r][v][e][r]

LOOP_BEGIN	[l][o][o][p]
LOOP_END	[e][n][d][l][o][o][p]

CD		[c][d]
UNLINK		[u][n][l][i][n][k]
OPEN		[o][p][e][n]
CLOSE		[c][l][o][s][e]
MKDIR		[m][k][d][i][r]
STAT		[s][t][a][t]
READDIR		[r][e][a][d][d][i][r]
SETATTR		[s][e][t][a][t][t][r]
SOFTLINK	[s][o][f][t][l][i][n][k]
HARDLINK	[h][a][r][d][l][i][n][k]
READLINK	[r][e][a][d][l][i][n][k]

WAITRACE	[w][a][i][t][_][r][a][c][e]
SLEEP		[s][l][e][e][p]

EXPECTED	[e][x][p][e][c][t][e][d]

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

{CLIENT}	return TOK_CLIENT;
{SERVER}	return TOK_SERVER;

{IP}@{LNDS}	{ yylval.strval = strdup(yytext); return TOK_NID; }
{ID}		{ yylval.strval = strdup(yytext); return TOK_ID; }
{IP}		{ yylval.strval = strdup(yytext); return TOK_IP; }

{digit}+	{ yylval.intval = atoi(yytext); return TOK_NUMBER; }
{STRING}	{ yylval.strval = yytext; return STRING; }

.		return yytext[0];
%%

