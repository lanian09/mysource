#include "ifb_proto.h"

//extern SFM_SysCommMsgType   *loc_sadb;
extern int	displayQkeyFlag;
extern IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{

	// login-name을 확인한다.
	if (ifb_checkLogin () < 0) {
		//return -1;
	}

	// input argument 확인
	if (disprc_getArgs (ac, av) < 0)
		return -1;

	// sysconf 파일에서 등록된 process들의 list를 table에 저장한다.
	//
	if (ifb_setConfProcTbl () < 0)
		return -1;

    init_ver_shm();

	/* by june
	 * - 화면에 Operation Mode 표시 
	 */
	if (init_sadb_shm() < 0)
		return -1;

	// /proc를 검색하여 실행중인 process들의 정보를 table에 저장한다.
	if (ifb_getProcStatus (1) < 0)
		return -1;

	// process table의 내용을 출력한다.
	ifb_printProcStatus ();

    detatch_ver_shm();

	return 0;

} //----- End of main -----//




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int disprc_getArgs (int ac, char *av[])
{
	int		a;
	char	*usage =
"  disprc [-h | -q] \n\
          -h : display this\n\
          -q : display queue key\n\n";

	while ((a = getopt(ac,av,"hq")) != EOF)
	{
		switch (a) {
			case 'q':
				displayQkeyFlag = 1;
				break;

			default:
				fprintf(stderr,"%s",usage);
				return -1;
		}
	}

	if (optind < ac) {
		fprintf(stderr,"%s",usage);
		return -1;
	}

	return 1;

} //----- End of disprc_getArgs -----//
