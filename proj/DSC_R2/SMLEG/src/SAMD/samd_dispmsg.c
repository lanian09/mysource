#include "samd.h"

extern  char             mySysName[COMM_MAX_NAME_LEN];
extern SFM_SysCommMsgType       *loc_sadb;

void doDisSessLoad(IxpcQMsgType *rxIxpcMsg )
{
    MMLReqMsgType   *rxReqMsg;
    char    tmpbuf[1024], msgbuf[2048];
	int		rsrcload[SFM_MAX_RSRC_LOAD_CNT];
	int 	i,j;

	memset(rsrcload, 0x00, (sizeof(int)*SFM_MAX_RSRC_LOAD_CNT));

    rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

	for(i=0; i<SFM_MAX_RSRC_LOAD_CNT; i++){
		for(j=0; j<MAX_MP_NUM1; j++){
			rsrcload[i] += loc_sadb->rsrc_load.rsrcload[i][j];
		}
	}
    /* get input paramters */
    sprintf(msgbuf,"    SYSTEM = %s\n",mySysName);
    strcat (msgbuf,"    RESULT = SUCCESS       \n");
    strcat (msgbuf,"    ======================================================\n");
    strcat (msgbuf,"      SESSION_TYPE    CURRENT_SESSION_COUNT(MAX_SESSION)       \n");
	strcat (msgbuf,"    ======================================================\n");
    sprintf(tmpbuf,"      CDR_TCPIP                  %10d(     500000)\n", rsrcload[DEF_MMDB_SESS]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      CDR_SESSION                %10d(     150000)\n", rsrcload[DEF_MMDB_OBJ]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      PCDR_TCPIP                 %10d(     500000)\n", rsrcload[DEF_MMDB_SESS2]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      PCDR_SESSION               %10d(     150000)\n", rsrcload[DEF_MMDB_OBJ2]);
    strcat (msgbuf, tmpbuf);	   
	sprintf(tmpbuf,"      TRCDR_SESSION              %10d(     150000)\n", rsrcload[DEF_MMDB_CALL]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      WAP1_SESSION               %10d(       5000)\n", rsrcload[DEF_MMDB_WAP1]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      WAP2_SESSION               %10d(       5000)\n", rsrcload[DEF_MMDB_WAP2]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      HTTP_SESSION               %10d(       5000)\n", rsrcload[DEF_MMDB_HTTP]);
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      VOD_SESSION                %10d(      20000)\n", rsrcload[DEF_MMDB_VODS]);	// 080313, poopee
    strcat (msgbuf, tmpbuf);
    sprintf(tmpbuf,"      VT_CALL                    %10d(      40000)\n", rsrcload[DEF_MMDB_VT]);
    strcat (msgbuf, tmpbuf);	
	sprintf(tmpbuf,"      UDRGEN_CALL                %10d(     150000)\n", rsrcload[DEF_MMDB_UDR]);
    strcat (msgbuf, tmpbuf);
    strcat (msgbuf,"    ------------------------------------------------------\n");
    strcat (msgbuf,"      TOTAL COUNT = 10                    \n");
    strcat (msgbuf,"    ======================================================\n");
    
    MMCResSnd (rxIxpcMsg, msgbuf, 0, 0);
    return;
}
