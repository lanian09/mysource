#include "ifb_proto.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	char	userName[32], getBuf[256], token[8][32], cmdBuf[64];
	char	*iv_home;
	char	choice;	
	FILE	*fp;


	if ((iv_home = getenv(IV_HOME)) == NULL) {
		fprintf(stderr, "[main] not found %s environment name\n", IV_HOME);
		return -1;
	}
	fprintf(stdout, "\n"); 	
	if (ifb_checkLogin () < 0)
		return -1;
	
	/* id, passwd check mml_passwd 파일 참조 */
	if(interact_w(iv_home) < 0)
		return -1;
	
	if (ac == 2) // 특정 user가 생성한 놈만 지운다.
		strcpy (userName, av[1]);
	else  // sysconfig에 등록된 operator 계정꺼만 지운다.
		ifb_getOpAccount (userName);

	fprintf(stderr,"\n  Do you want to remove Shared Memory and Message Queue ? (y/n) ");
	scanf("\n%c", &choice); // y/n받기 위해서 여기서 대기..

	if (!ifb_promptYesNo2(&choice))
        	return -1;
	fprintf(stderr,"\n");

	commlib_microSleep(1000);
#if 1
	unlink("/tmp/listqm");
	system("ipcs -qmo > /tmp/listqm");

	if ((fp=fopen("/tmp/listqm","r")) == NULL ) {
		fprintf(stderr,"\n  fopen fail[/tmp/listqm] err=%d(%s)\n", errno, strerror(errno));
		return -1;
	}

	while (fgets(getBuf, sizeof(getBuf), fp) != NULL)
	{
		if (getBuf[0] != 'q' && getBuf[0] != 'm')
			continue;
		sscanf (getBuf, "%s%s%s%s%s", token[0], token[1], token[2], token[3], token[4]);

		// 해당 user가 생성한 놈인지 확인한다.
		if (strcasecmp(userName,"all") && strcasecmp(userName, token[4]))
			continue;

		if (getBuf[0] == 'q') {
			sprintf(cmdBuf,"ipcrm -q %s", token[1]);
			fprintf(stderr,"    remove msg Queue : ID=%-4s KEY=%s\n", token[1], token[2]);
		} else if (getBuf[0] == 'm') {
			sprintf(cmdBuf,"ipcrm -m %s", token[1]);
			fprintf(stderr,"    remove shared Memory : ID=%-4s KEY=%s\n", token[1], token[2]);
		}
		system (cmdBuf);
	}
	fclose(fp);
	unlink("/tmp/listqm");
#endif
	return 0;

} //----- End of main -----//
