#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "path.h"			/* DATA_PATH */
#include "commdef.h"		/* FILE_FLT_SERVICE */

#include "fltmng_file.h"


int	gszSvcID[SVC_ONOFF_LIST_CNT] 
					= {1000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 11000, 12000, 13000, 14000, 15000, 16000, 60000};
int	gszSvcOnOff[SVC_ONOFF_LIST_CNT] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};


//읽어 들일 데이터가 있으면 데이터를 읽어들인다.
int dGetRead(void)
{
	int				i;
    char            szBuf[1024];
    FILE            *fa;
    int				num=0, svc_id, svc_onoff;

    fa = fopen(FILE_FLT_SERVICE, "r");
    if(fa == NULL){
        return -1;
    }

    while(fgets(szBuf,1024,fa) != NULL)
    {
        if(szBuf[0] != '#') {
            fclose(fa);
            return -2;
        }

        if(szBuf[1] == '#')
            continue;
        else if(szBuf[1] == 'E')
            break;
        else if(szBuf[1] == '@'){
            num = sscanf(&szBuf[2],"%d %d", &svc_id, &svc_onoff);
            if(num == 2){
				for(i = 0; i < SVC_ONOFF_LIST_CNT; i++){
					if(gszSvcID[i] == svc_id){
						gszSvcOnOff[i] = svc_onoff;
						break;
					}
				}
			}
        }
    }

    return 0;
}

void dMakeFile(void)
{
    FILE *fa;

    fa = fopen(FILE_FLT_SERVICE ,"w");

    fputs("##\n", fa);
    fputs("## SVCID      FLAG\n", fa);
    fputs("#@ 1000        1\n",fa);
    fputs("#@ 3000        1\n",fa);//SVC_IDX_DN
    fputs("#@ 4000        1\n",fa);//SVC_IDX_STREAM
    fputs("#@ 5000        1\n",fa);//SVC_IDX_MMS`
    fputs("#@ 6000        1\n",fa);//SVC_IDX_ETC
    fputs("#@ 7000        1\n",fa);//SVC_IDX_FB
    fputs("#@ 8000        1\n",fa);//SVC_IDX_IV
    fputs("#@ 9000        1\n",fa);//SVC_IDX_EMS
    fputs("#@ 10000       1\n",fa);//SVC_IDX_FV
    fputs("#@ 11000       1\n",fa);//SVC_IDX_IM
    fputs("#@ 12000       1\n",fa);//SVC_IDX_BANK
    fputs("#@ 13000       1\n",fa);//SVC_IDX_WIDGET
    fputs("#@ 14000       1\n",fa);//SVC_IDX_VT
    fputs("#@ 15000       1\n",fa);//SVC_IDX_PHONE
    fputs("#@ 16000       1\n",fa);//SVC_IDX_CORP
    fputs("#@ 60000       1\n",fa);//SVC_IDX_INET
    fputs("#E\n",fa);
    fclose(fa);
}

int dWriteFile(unsigned int uisvc_code,unsigned int flag)
{
    FILE 	*fa;
    int 	i=0;
    char 	szBuf[1024];

    fa = fopen(FILE_FLT_SERVICE , "w");
    if(fa == NULL) {
		return -1;
    }

    fputs("##\n", fa);
    fputs("## SVCID      FLAG\n", fa);

    for(i = 0; i < SVC_ONOFF_LIST_CNT; i++)
    {
		if(uisvc_code == gszSvcID[i])
		{
			gszSvcOnOff[i] = flag;
		}

        sprintf(szBuf, "#@ %d       %d\n", gszSvcID[i], gszSvcOnOff[i]);
        fputs(szBuf, fa);
    }
    fputs("#E\n", fa);

    fclose(fa);

	return 1;
}

void Init_Service_Conf(void)
{
	int		dRet;

	dRet = dGetRead();
	if(dRet < 0)
		dMakeFile();

	return;	
}

int Serch_Service_Conf(unsigned int uisvc_code, unsigned uisvc_flag)
{
	int dRet;

	dRet = dWriteFile(uisvc_code, uisvc_flag);
	if(dRet < 0)
		return -1;

	return 1;
}

