/**********************************************************
   Author   : IN-Team
   Revision : 
   Description:
   Copyright (c) , Inc.
***********************************************************/
#include <smpp.h>

extern  int     smppQid, ixpcQid;
extern  int 	errno;

SMPP_FD_TBL client;
extern SMPP_CID  smpp_cid[MAX_SMPP_CID];


void ClearCid(int cid)
{
	if (cid < 0 || cid >= MAX_SMPP_CID)
		return;

    smpp_cid[cid].conidx = 0;
	smpp_cid[cid].cid = 0;
	smpp_cid[cid].prototype = 0;
    FreeCid(cid);
}


void audit_call(time_t  now)
{
	int i, idx;

    for (i = 0; i < MAX_SMPP_CID; i++) {
		if (smpp_cid[i].conidx == 0)
			continue;

		if ((now - smpp_cid[i].starttime) < 5) 
			continue;
		
		idx = smpp_cid[i].conidx;

		if (client.condata[idx].bindtry != 0) {
			client.condata[idx].bindtry = 0;
		}
		dAppLog (LOG_CRI, "[audit_call] ACK TimeOut: CID=%d SMC=%d\n", i, smpp_cid[i].conidx);	
		ClearCid(i);
	
		if (client.condata[idx].bind == 0)
			cut_socket(idx);		
	}
}


int getAllocCidCnt(void)
{
	int		i, cnt;

	cnt = 0;
	for (i = 0; i < MAX_SMPP_CID; i++) {
		if (smpp_cid[i].conidx == 0)
            continue;

		cnt++;
	}	

	return cnt;
}


int get_root_id(int npa, int ctn, int *root_id)
{
    *root_id = ((npa % 10) * 10000) + (ctn / 10000);
    if (*root_id < 0 || *root_id > PREFIX_MAX)
        return  -1;
    else
        return 1;
}

#if 0 /* by june */
int smpp_mkDir(char *dirName)
{
	DIR		*dp;
	struct	dirent	*direntp;
	struct	stat	*st;
	char	sysMmc[256];

	if ((dp = opendir(dirName)) == NULL) {
		if (errno == ENOENT) {
			if(dirName != NULL){
				sprintf(sysMmc,"mkdir %s",dirName);
				if(system(sysMmc) < 0) {
					dAppLog (LOG_CRI, "[smpp_mkDir] fail system system[%s]; err=%d(%s)n", sysMmc, errno, strerror(errno));
					return -1;
				}
			}
		}
	}
	closedir(dp);
	return 1;
} //----- End of cdrd_mkDir -----//
#endif

void smpp_makePFX(int index, char * spfx)
{
	char tmp[255];
	sprintf(tmp,"%04d",index%10000);
	switch(index/10000){
		case 0: strcpy(spfx,"010");break;
		case 1: strcpy(spfx,"011");break;
		case 6: strcpy(spfx,"016");break;
		case 7: strcpy(spfx,"017");break;
		case 8: strcpy(spfx,"018");break;
		case 9: strcpy(spfx,"019");break;
		default:strcpy(spfx,"");break;
	}
	strcat(spfx,tmp);
}


