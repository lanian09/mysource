#include "mmcd_proto.h"

char		trcBuf[4096], trcTmp[1024];


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	int		cmdCnt;
	char	errBuf[1024];
	MMLCmdContext	*cmdTbl;
	MMLHelpContext	*helpTbl;

	// 메모리를 할당한다.
	//
	if ((cmdTbl = (MMLCmdContext*) calloc (MML_NUM_TP_CMD_TBL, sizeof(MMLCmdContext))) == NULL) {
		fprintf(stderr,"calloc fail (cmdTbl)\n");
		return -1;
	}
	if ((helpTbl = (MMLHelpContext*) calloc (MML_NUM_TP_CMD_TBL, sizeof(MMLHelpContext))) == NULL) {
		fprintf(stderr,"calloc fail (helpTbl)\n");
		return -1;
	}

	// table을 구성한다.
	//

	if ((cmdCnt = mmcd_loadCmdTbl (cmdTbl, helpTbl, errBuf)) < 0) 
		return -1;

	// 명령어 리스트와 syntax를 DB에 넣는다.
	//
	mmcd_saveCmdInfo2DB (cmdTbl, helpTbl, cmdCnt);

	// 메모리 free 시킨다.
	//
	free(cmdTbl);
	free(helpTbl);

	fprintf(stderr,"\n      REBUILT %d COMMANDs SUCCESSFULLY \n\n", cmdCnt);

	return cmdCnt;

} //----- End of mmcd_rebuildCmdTbl -----//

