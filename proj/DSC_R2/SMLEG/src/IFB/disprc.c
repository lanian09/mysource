#include "ifb_proto.h"

//extern SFM_SysCommMsgType   *loc_sadb;
extern int	displayQkeyFlag;
extern IFB_ProcInfoContext	confProcTbl[SYSCONF_MAX_APPL_NUM];


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{

	// login-name�� Ȯ���Ѵ�.
	if (ifb_checkLogin () < 0) {
		//return -1;
	}

	// input argument Ȯ��
	if (disprc_getArgs (ac, av) < 0)
		return -1;

	// sysconf ���Ͽ��� ��ϵ� process���� list�� table�� �����Ѵ�.
	//
	if (ifb_setConfProcTbl () < 0)
		return -1;

    init_ver_shm();

	/* by june
	 * - ȭ�鿡 Operation Mode ǥ�� 
	 */
	if (init_sadb_shm() < 0)
		return -1;

	// /proc�� �˻��Ͽ� �������� process���� ������ table�� �����Ѵ�.
	if (ifb_getProcStatus (1) < 0)
		return -1;

	// process table�� ������ ����Ѵ�.
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
