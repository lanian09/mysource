#include "samd.h"


//------------------------------------------------------------------------------
// mmcd로 명령어를 보낸다.
// history buffer에 기록한다.
//------------------------------------------------------------------------------
//static int cliReqId = 100;
int	samd_send2mmcd (char *input, char confirm, int batchFlag)
{
#if 0
    int     txLen;
    SockLibMsgType      txSockMsg;
    MMLClientReqMsgType *txCliReqMsg;

    txCliReqMsg = (MMLClientReqMsgType*)txSockMsg.body;

    txCliReqMsg->head.cliReqId = htonl(cliReqId++);
    txCliReqMsg->head.batchFlag = batchFlag;

    txCliReqMsg->head.confirm = confirm;

    txCliReqMsg->head.clientType = 0; /* RMI */

    strcpy (txCliReqMsg->body, input);

    txSockMsg.head.bodyLen = sizeof(txCliReqMsg->head) + strlen(input);
    txLen = sizeof(txSockMsg.head) + txSockMsg.head.bodyLen;
                
    if (socklib_sndMsg_hdr_chg (sockFd, (char*)&txSockMsg, txLen) < 0) {           
        fprintf(stderr,"\n    >>> socklib_sndMsg fail \n\n");                      
        rmi_terminate (0);                                                         
    }                                                                              
#endif                                                                              
 //   if (!strncasecmp(input,"log-in",6))
  //      return 1;
                                                                                   
    // history buffer에 기록한다. 
    // 단 confirm이 아닌경우만
//    if ( confirm < 0 ) rmi_saveInputHistory (input);

  //  batchend=0;

    return 1;

}
