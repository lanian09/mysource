/**
	@file		help_proc.c
	@author
	@version
	@date		2011-07-14
	@brief		help_proc.c
*/

/**
	Include headers
*/

/* SYS HEADER */
#include <string.h>
#include <stdio.h>
#include <ctype.h>
/* LIB HEADER */
#include "loglib.h"
/* PRO HEADER */
#include "mmcdef.h"
#include "config.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "para_load.h"
#include "help_proc.h"

/**
	Define constants
*/
#define MAX_HELP_STRING				2500

/**
	Declare variables
*/
int			lcnt = 0;
static char *user_class_str[] = {
	"UNDEFINED", "SYSTEM ADMIN", "NORMAL USER", "3", "4", "5", "6", "7", "8", "9", "GAME OPER", "GAME SUPER" };

extern COM_TBL				cmd_tbl[];			/*< mmcd_main.h */
extern int					num_cmd_tbl;		/*< mmcd_main.h */
extern tENUMLIST			enumlist[];			/*< mmcd_main.h */		
extern char					SmsName[];			/*< mmcd_main.h */

/**
 *	Declare extern func.
 */
extern int dIsExceptionCmdSe(char *cmd);
extern char *get_string(char *str);
extern COM_TBL *get_cmd_tbl(char *str);
extern char *ComposeHead();
extern PARA_TBL *get_para_tbl(char *str);
extern char *get_type_str(int idx);

/**
 *	Implement func.
 */
int append_help(int i, char *outstr, int *slen, int *lline)
{
	int tlen;

	tlen = strlen(cmd_tbl[i].com_name);

	if( dIsExceptionCmdSe(cmd_tbl[i].com_name) )
		return 1;

	if (tlen + *slen > MAX_HELP_STRING)
	{
		strcpy(&outstr[*slen],
		"\nIF YOU INPUT ??, YOU GET DETAILED INFORMATION\n");
		return 0;

	}
	else if( lcnt > 4 )
	{
/*	else if (tlen + *lline > MAX_HELP_ALINE) { */
		outstr[(*slen)++] = '\n';
		outstr[*slen] = 0;
		lcnt=0;
		*lline = 0;
	}
/*	strcpy(&outstr[*slen], cmd_tbl[i].com_name); */
	sprintf(&outstr[*slen], "%-20s ", cmd_tbl[i].com_name);
	(*slen)  +=  strlen(&outstr[*slen]);
	lcnt += 1;

	return 1;
}

int cmd_description(int slen, COM_TBL *cmd, char *outstr)
{
	int i, j;
	PARA_TBL	*para;

	/*
	* DISPLAY COMMAND
	*/
	sprintf(&outstr[slen], "COMMAND:\n    %s ", cmd->com_name);
	slen += strlen(&outstr[slen]);
	for (i = 0;i < cmd->num_para;i++) {
		if (i < cmd->p_man)
			sprintf(&outstr[slen], "%s=%c ", cmd->para[i].para_name, 'a' + i);
		else
			sprintf(&outstr[slen], "[%s=%c] ", cmd->para[i].para_name, 'a' + i);
		slen += strlen(&outstr[slen]);
	}

	/*
	* COMMAND DESCRIPTION
	*/
	sprintf(&outstr[slen], "\nDESCRIPTION:\n    %s\n", cmd->help[0]);
	slen += strlen(&outstr[slen]);
	if (cmd->help[1] != NULL) {
		sprintf(&outstr[slen], "    %s\n", cmd->help[1]);
		slen += strlen(&outstr[slen]);
	}
	if (cmd->help[2] != NULL) {
		sprintf(&outstr[slen], "    %s\n", cmd->help[2]);
		slen += strlen(&outstr[slen]);
	}

	/*
	* PARAMETER DESCRIPTION
	*/
	sprintf(&outstr[slen], "PARAMETER:\n");
	slen += strlen(&outstr[slen]);

	for (i = 0;i < cmd->num_para;i++) {
		sprintf(&outstr[slen], "    %c = %s ", 'a' + i, get_type_str(cmd->para[i].type));
		slen += strlen(&outstr[slen]);

		/*
		* ENUM
		*/
		if (cmd->para[i].type == VT_ENUM || cmd->para[i].type == VT_EPOS) {
			strcpy(&outstr[slen], "{ ");
			slen += strlen(&outstr[slen]);
			for (j = cmd->para[i].start;j < cmd->para[i].end;j++) {
				sprintf(&outstr[slen], "%s ", enumlist[j].para_name);
				slen += strlen(&outstr[slen]);
			}
			sprintf(&outstr[slen], "} ");

		/*
		* HEXA
		*/
		} else if (cmd->para[i].type == VT_HEXA) {
/* modified by uamyd 2008.01.08
			if (cmd->para[i].def_value)
				sprintf(&outstr[slen],"(%X:%X) ",cmd->para[i].start,cmd->para[i].end);
			else
				sprintf(&outstr[slen],"(%X:%X) ", cmd->para[i].start, cmd->para[i].end);
*/
			sprintf(&outstr[slen],"(%X:%X) ",(int)cmd->para[i].start,(int)cmd->para[i].end);
		/*
		* YTIME
		*/
		} else if (cmd->para[i].type == VT_YTIME) {
			sprintf(&outstr[slen],"(YYYYMMDDHH) ");

		/*
		* MTIME
		*/
		} else if (cmd->para[i].type == VT_MTIME) {
			sprintf(&outstr[slen],"(YYYYMMDDHHMM) ");

		/*
		* HTIME
		*/
		} else if (cmd->para[i].type == VT_HTIME) {
			sprintf(&outstr[slen],"(HHMM) ");

		/*
		* STRING, DECIMAL
		*/
		} else {
			if (cmd->para[i].def_value)
				sprintf(&outstr[slen],"(%lld:%lld)",cmd->para[i].start,cmd->para[i].end);
			else
				sprintf(&outstr[slen],"(%lld:%lld) ", cmd->para[i].start, cmd->para[i].end);
		}

		slen += strlen(&outstr[slen]);

		/*
		* PARAMETER HELP
		*/
		para = get_para_tbl(cmd->para[i].para_name);
		if (para == NULL || para->para_help == NULL)
			strcpy(&outstr[slen], "\n");
		else
			sprintf(&outstr[slen],"\n      [%s] = %s\n",para->para_name, para->para_help);
		slen += strlen(&outstr[slen]);
	}

	/*
	* USER LEVEL
	*/
	sprintf(&outstr[slen], "\nUSER LEVEL:\n    %s\n", user_class_str[(int)cmd->access]);
	slen += strlen(&outstr[slen]);

	return slen;
}

void help_proc(char *outstr)
{
	int  	i;
	int		slen, lline, chklen ;
	char 	*sp, upstr[1024];
	COM_TBL *cmd;


	*outstr = '\n';
	slen = 1;
	if ((sp = get_string(NULL)) == NULL) return;



	/* ? : ALL COMMAND */
	/* ? str : ALL COMMAND INCLUDING str */
	if (*sp == '?' || !strcasecmp(sp, "help") ) {
		if ((sp = get_string(NULL)) == NULL) {
			/*
			* ?, help
			*/
			lline = 0;
			lcnt = 0;
			for (i = 0;i < num_cmd_tbl;i++) {
				if (append_help(i, outstr, &slen, &lline) == 0)
				{
					lcnt = 0;
					return;
				}
			}
		} else if (*sp == '?') {
			/*
			* ??
			*/
			strcpy(outstr, "\n HELP USAGE\n");
            strcat(outstr, "  ?        : DISPLAY TOTAL COMMAND\n");
            strcat(outstr, "  ??       : DISPLAY HELP USAGE\n");
            strcat(outstr, "  STRING ? : DISPLAY COMMAND DESCRIPTION STARTING WITH THIS STRING\n");
            strcat(outstr, "             DISPLAY COMMAND DESCRIPTION MATCHED THIS STRING\n");
            strcat(outstr, "  ? STRING : DISPLAY COMMAND DESCRIPTION MATCHED THIS STRING\n");
            strcat(outstr, "             DISPLAY COMMAND DESCRIPTION INCLUDING THIS STRING\n");
            strcat(outstr, "             Contact uPRESTO Co. LTD  \n");
			return;

		} else {
			if ((cmd = get_cmd_tbl(sp)) != NULL) {
				/*
				* COMMAND DESCRIPTION
				*/
				slen = cmd_description(slen, cmd, outstr);
			} else {
				for (i = 0; *sp;i++)
					upstr[i] = toupper(*sp++);
				upstr[i] = 0;
				lline = 0;
				lcnt = 0;
				for (i = 0;i < num_cmd_tbl;i++) {
					if (strstr(cmd_tbl[i].com_name, upstr) == NULL)
						continue;
					if (append_help(i, outstr, &slen, &lline) == 0)
						return;
				}
			}
		}

	/*
	* NOT START ? IN sp
	*/
	} else {
		if ((cmd = get_cmd_tbl(sp)) == NULL) {

			chklen = strlen(sp);
			for (i = 0; *sp;i++)
				upstr[i] = toupper(*sp++);

			log_print(LOGN_INFO, "NOT START HELP : [%s]", upstr );
			log_print(LOGN_INFO, "outstr : [%s]", outstr );

			upstr[i] = 0;
			lline = 0;
			lcnt = 0;
			for (i = 0;i < num_cmd_tbl;i++) {
				if (strncmp(cmd_tbl[i].com_name, upstr, chklen) != 0)
					continue;
				if (append_help(i, outstr, &slen, &lline) == 0)
					return;
			}
		} else {
			slen = cmd_description(slen, cmd, outstr);
		}
	}

	/*
	* ANYTHING ELSE
	*/
	if (slen <= 1)
	{
		sprintf( outstr,
				"\n%s %s  CANNOT FIND MATCHED COMMAND\n",
				SmsName, (char *)ComposeHead ());
	}
	else {
		outstr[slen++] = '\n';
		outstr[slen] = 0;
	}
}
