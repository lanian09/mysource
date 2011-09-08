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

	// �޸𸮸� �Ҵ��Ѵ�.
	//
	if ((cmdTbl = (MMLCmdContext*) calloc (MML_NUM_TP_CMD_TBL, sizeof(MMLCmdContext))) == NULL) {
		fprintf(stderr,"calloc fail (cmdTbl)\n");
		return -1;
	}
	if ((helpTbl = (MMLHelpContext*) calloc (MML_NUM_TP_CMD_TBL, sizeof(MMLHelpContext))) == NULL) {
		fprintf(stderr,"calloc fail (helpTbl)\n");
		return -1;
	}

	// table�� �����Ѵ�.
	//

	if ((cmdCnt = mmcd_loadCmdTbl (cmdTbl, helpTbl, errBuf)) < 0) 
		return -1;

	// ��ɾ� ����Ʈ�� syntax�� DB�� �ִ´�.
	//
	mmcd_saveCmdInfo2DB (cmdTbl, helpTbl, cmdCnt);

	// �޸� free ��Ų��.
	//
	free(cmdTbl);
	free(helpTbl);

	fprintf(stderr,"\n      REBUILT %d COMMANDs SUCCESSFULLY \n\n", cmdCnt);

	return cmdCnt;

} //----- End of mmcd_rebuildCmdTbl -----//

