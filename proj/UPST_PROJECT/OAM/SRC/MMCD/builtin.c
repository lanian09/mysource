/**
	@file		builtin.c
	@author
	@date		2011-07-13
	@version
	@brief		설명
*/

/**
	Include headers
*/

/* SYS HEADER */
#include <stdio.h>
/* LIB HEADER */
#include "loglib.h"
/* PRO HEADER */
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "builtin.h"

/**
 *	Declare func.
 */
extern int usr_login( In_Arg Par[], int sfd, mml_msg *ml );
extern int usr_logout( In_Arg Para[], int sfd, mml_msg *ml );
extern int kill_user( In_Arg Para[], int sfd, mml_msg *ml );
extern int dis_his_cmd( In_Arg Para[], int sfd, mml_msg *ml );
extern int dis_cmd_exe( In_Arg Par[], int sfd, mml_msg *ml );
extern int del_cmd_exe( In_Arg Par[], int sfd, mml_msg *ml );
extern int dGetCOMString(In_Arg Par[], int sfd, mml_msg *ml);

/**
	@brief		Exe_Builtin 설명
				OPERATED MMC COMMAND IN MMCD
	@param		mml_msg
	@param		sockfd
	@param		In_Arg
*/
int Exe_Builtin(mml_msg *ml, int sockfd, In_Arg  in_para[])
{
	log_print(LOGN_INFO, "sockfd[%d] MSG_ID[%d]", sockfd, ml->msg_id );

	switch(ml->msg_id) {

		case MI_USER_LOGIN:
			usr_login(&in_para[0], sockfd, ml);
			break;

		case MI_USER_LOGOUT:
			usr_logout(&in_para[0], sockfd, ml);
			fflush(stdout);
			break;

		case MI_KILL_USER:
			kill_user(&in_para[0], sockfd, ml);
			fflush(stdout);
            break;

		/* needed test by uamyd 2008.01.08 */
		case MI_DIS_HIS_CMD:
			dis_his_cmd(&in_para[0], sockfd, ml);
			break;

		case MI_DIS_CMD_EXE:
			dis_cmd_exe(&in_para[0], sockfd, ml);
			break;
	
		case MI_DEL_CMD_EXE:
            del_cmd_exe(&in_para[0], sockfd, ml);
            break;

        case MI_GET_COM:
            dGetCOMString(&in_para[0], sockfd, ml);
            break;
	
		default :
			break;
	}
	return 1;
}
