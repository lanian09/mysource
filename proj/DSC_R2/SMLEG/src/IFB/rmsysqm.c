#include "ifb_proto.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int ac, char *av[])
{
	char	userName[32], getBuf[256], token[8][32], cmdBuf[64];
	FILE	*fp;
	char  	*iv_home, choice;

	if ((iv_home = getenv(IV_HOME)) == NULL) {
		fprintf(stderr, "[killprc_init] not found %s environment name\n", IV_HOME);
		return -1;
	}
			
	fprintf(stdout, "\n");
////	if (ifb_checkLogin () < 0)
////		return -1;

	/* id, passwd check mml_passwd 팀 혌 */
////	if(interact_w(iv_home) < 0)
////		return -1;

	if (ac == 2) // 특정 user가 생성한 놈만 지운다.
		strcpy (userName, av[1]);
	else  // sysconfig에 등록된 operator 계정꺼만 지운다.
		ifb_getOpAccount (userName);

	fprintf(stderr,"\n  Do you want to remove Shared Memory and Message Queue ? (y/n) ");
	scanf("\n%c", &choice);
	
	if (!ifb_promptYesNo2(choice))
		return -1;
	fprintf(stderr,"\n");


#if 0
	unlink("/tmp/listqm");
	system("ipcs -qm > /tmp/listqm");

	if ((fp=fopen("/tmp/listqm","r")) == NULL ) {
		fprintf(stderr,"\n  fopen fail[/tmp/listqm] err=%d(%s)\n", errno, strerror(errno));
		return -1;
	}

	while (fgets(getBuf, sizeof(getBuf), fp) != NULL)
	{
		/*if (getBuf[0] != 'q' && getBuf[0] != 'm')
			continue;
		*/
		
		if(getBuf[0] == ' '&& getBuf[0] !='\n') continue;
		
		sscanf (getBuf, "%s%s%s%s%s", token[0], token[1], token[2], token[3], token[4]);
        //printf("POOPEE: [%s]\n",getBuf);

		if( !strcasecmp(token[1],"Shared") )
		{
//printf("shered Memory : %s\n", userName);
			kindflag = 1;
			continue;
		}
		else if( !strcasecmp(token[1],"Semaphore") )
		{
//printf("Semaphore Memory\n", userName);
			kindflag = 2;
			break;
		}
		else if( !strcasecmp(token[1],"Message") )
		{
//printf("Message Memory:%s\n", userName);
			kindflag = 3;
			//break;
			continue;
		}

		// 해당 user가 생성한 놈인지 확인한다.
		/*if (strcasecmp(userName,"all") && strcmp(userName, token[4]))
			continue;
			*/
		memset(tmpBuf, 0, sizeof(tmpBuf));
		memset(tmpBuf1, 0, sizeof(tmpBuf1));
		strncpy(tmpBuf, &token[0][6],2);
		strncpy(tmpBuf1, &token[0][5],4);
//printf("*********jean %s -> %s \n", token[0], tmpBuf);
//printf("*********jean %s -> %s \n", token[0], tmpBuf1);
		

		//if (getBuf[0] == 'q') 
		if ( kindflag ==1 && (!strcasecmp(tmpBuf,"28") || !strcasecmp(tmpBuf,"27")
			|| !strcasecmp(tmpBuf,"29") || !strcasecmp(tmpBuf, "30") || !strcasecmp(tmpBuf,"51")
			|| !strcasecmp(tmpBuf1,"1111")) )
		{
			sprintf(cmdBuf,"ipcrm -m %s", token[1]);
//printf("jean %s\n", cmdBuf);
			fprintf(stderr,"    shared msg Queue : ID=%-4s KEY=%s\n", token[1], token[2]);
		} 
		//else if (getBuf[0] == 'm') 
		else if (kindflag == 3 && (!strcasecmp(tmpBuf,"53") || !strcasecmp(tmpBuf,"54")))
		{
			sprintf(cmdBuf,"ipcrm -q %s", token[1]);
//printf("jean %s\n", cmdBuf);
			fprintf(stderr,"    remove shared Memory : ID=%-4s KEY=%s\n", token[1], token[2]);
		}
		system (cmdBuf);
	}
	fclose(fp);
	unlink("/tmp/listqm");

#else
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

