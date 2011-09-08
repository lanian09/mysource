/**A.1*  File Inclusion ***********************************/
#include <errno.h>
#include "init_shm.h"
#include <ipaf_stat.h>
#include <comm_timer.h>
#include "mmdb_svcopt.h"

/**B.1*  Definition of New Constants **********************/
/**B.2*  Definition of New Type  **************************/
/**C.1*  Declaration of Variables  ************************/

T_GENINFO   		*gen_info = NULL;
st_IPAF        		*fidb = NULL;
T_Keepalive         *keepalive = NULL;

st_VERSION	*version;	// 040114,poopee

extern int errno;

/**D.1*  Definition of Functions  *************************/

int Init_FIDB( int key );
int Init_KEEPALIVE( int key );
int Init_GEN_INFO( int key );


int Init_Heart_Beat();
int Heart_Beat(int dProcNo);
int chg_proc_normal(int idx);
int chg_proc_abnormal(int idx);

/* dFidb = 0 , 1 - IPC_EXCL */
/*
int Init_shm_all( int dFidb )
{
	int dRet = 0;

	if( dFidb == 0 )
	{
		dRet = Init_FIDB();
		if( dRet < 0 )
			return (dRet+ERR_SHM_FIDB);
	}

	dRet = Init_KEEPALIVE();
	if( dRet < 0 )
		return (dRet+ERR_SHM_KEEP);

	dRet = Init_Cap();
	if( dRet < 0 )
		return (dRet+ERR_SHM_CAP);

	dRet = Init_GEN_INFO();
	if( dRet < 0 )
		return (dRet+ERR_SHM_GEN);

	dRet = Init_MMDBOBJ();
	if( dRet < 0 )
		return (dRet+ERR_SHM_OBJ);

	dRet = Init_MMDBSESS();
	if( dRet < 0 )
		return (dRet+ERR_SHM_SESS);

	dRet = Init_MMDBCDR();
	if( dRet < 0 )
		return (dRet + ERR_SHM_CDR);

	dRet = Init_MMDBDESTIP();
	if( dRet < 0 )
		return (dRet+ERR_SHM_DESTIP);

	dRet = Init_MMDBDESTPORT();
	if( dRet < 0 )
		return (dRet+ERR_SHM_DESTPORT);

	dRet = Init_ME_STAT();
	if( dRet < 0 )
		return (dRet+ERR_SHM_MESTAT);

	dRet = Init_KUN_STAT();
    if( dRet < 0 )
        return (dRet+ERR_SHM_KUNSTAT);

	return dRet;
}
*/

int Init_GEN_INFO( int key )
{
    int dShm = 0;

    dShm = shmget( key, sizeof(T_GENINFO), 0666|IPC_CREAT|IPC_EXCL);
    if( dShm < 0 )
    {
        if(errno == EEXIST)
        {
            dShm = shmget( key, sizeof(T_GENINFO), 0666|IPC_CREAT);
            if(dShm < 0)
			{
				printf("FAILED1 [RVAL]:[%d] [REASON]:[%s]", errno, strerror(errno));
                return (SHM_ERROR1-SHM_EXIST);
			}

            if( (gen_info = (T_GENINFO *)shmat( dShm, (char*)0, 0)) == (T_GENINFO *)-1)
			{
				printf("FAILED2 [RVAL]:[%d] [REASON]:[%s]", errno, strerror(errno));
                return (SHM_ERROR2-SHM_EXIST);
			}

            return SHM_EXIST;
        }

		printf("FAILED3 [RVAL]:[%d] [REASON]:[%s]", errno, strerror(errno));
        return SHM_ERROR1;
    }

    if( (gen_info = (T_GENINFO *)shmat( dShm, (char*)0, 0)) == (T_GENINFO *)-1)
	{
		printf("FAILED4 [RVAL]:[%d] [REASON]:[%s]", errno, strerror(errno));
        return SHM_ERROR2;
	}

    return SHM_CREATE;
}


int Init_FIDB( int key )
{
    int dShm = 0;

    dShm = shmget( key, sizeof(st_IPAF), 0666|IPC_CREAT|IPC_EXCL );
    if( dShm < 0 )
    {
        if( errno == EEXIST )
        {
            dShm = shmget( key, sizeof(st_IPAF), 0666|IPC_CREAT );
            if( dShm < 0 )
                return (SHM_ERROR1-SHM_EXIST);

            if( (fidb = (st_IPAF *)shmat( dShm, (char*)0, 0)) == (st_IPAF *)-1 )
                return (SHM_ERROR2-SHM_EXIST);

            return SHM_EXIST;
        }

        return SHM_ERROR1;
    }

    if( (fidb = (st_IPAF *)shmat( dShm, (char*)0, 0)) == (st_IPAF *)-1 )
        return SHM_ERROR2;

    return SHM_CREATE;
}

int Init_KEEPALIVE( int key )
{
    int dShm = 0;

    dShm = shmget( key, sizeof(T_Keepalive), 0666|IPC_CREAT|IPC_EXCL);
    if( dShm < 0 )
    {
        if( errno == EEXIST )
        {
            dShm = shmget( key, sizeof( T_Keepalive ), 0666|IPC_CREAT);
            if( dShm < 0 )
                return (SHM_ERROR1-SHM_EXIST);

            if( (keepalive = (T_Keepalive *)shmat( dShm, (char*)0, 0)) == (T_Keepalive *)-1)
                return (SHM_ERROR2-SHM_EXIST);

            return SHM_EXIST;
        }

        return SHM_ERROR1;
    }

    if( (keepalive = (T_Keepalive *)shmat( dShm, (char*)0, 0)) == (T_Keepalive *)-1)
        return SHM_ERROR2;

    return SHM_CREATE;
}

int Init_Heart_Beat()
{
	int i;

    for( i = 0 ; i < MAX_SW_COUNT ; i++ )
    {
        keepalive->cnt[i] = 1;
        keepalive->oldcnt[i] = 1;
    }

	return 1;
}


int Heart_Beat( int dProcNo )
{
	if( keepalive->cnt[dProcNo] > 1000000000 )
		keepalive->cnt[dProcNo] = 1;

	keepalive->cnt[dProcNo] += 1;

	return 1;
}

int chg_proc_normal( int idx )
{

	if( idx >= MAX_SW_COUNT || idx < 0 )
		return -1;

	keepalive->procnorm[idx] = NORMAL;

	return 1;
}

int chg_proc_abnormal( int idx )
{

	if( idx >= MAX_SW_COUNT || idx < 0)
		return -1;

	keepalive->procnorm[idx] = CRITICAL;

	return 1;

}

/* 040114,poopee */
int set_version(int prc_idx, char *ver)
{
	int		shmid;

	if( (shmid=(int)shmget(S_SSHM_VERSION,sizeof(st_VERSION),0666|IPC_CREAT|IPC_EXCL)) < 0)
	{
		if(errno == EEXIST)
		{
			if( (shmid=(int)shmget(S_SSHM_VERSION, sizeof(st_VERSION), 0666|IPC_CREAT)) < 0)
				return (SHM_ERROR1-SHM_EXIST);

			if( (version=(st_VERSION*)shmat(shmid, (char*)0, 0)) == (st_VERSION*)-1)
				return (SHM_ERROR2-SHM_EXIST);

			// set version
			strncpy(version->ver[prc_idx], ver, DEF_VERSION_SIZE-1);
			version->ver[prc_idx][DEF_VERSION_SIZE-1] = 0x00;

			// detach?
			return SHM_EXIST;
		}

		return SHM_ERROR1;
	}

	if( (version=(st_VERSION*)shmat(shmid, (char*)0, 0)) == (st_VERSION*)-1)
		return SHM_ERROR2;

	memset(version, 0x00, sizeof(st_VERSION));

	// set version
	strncpy(version->ver[prc_idx], ver, DEF_VERSION_SIZE-1);
	version->ver[prc_idx][DEF_VERSION_SIZE-1] = 0;

	// detach?
	return SHM_CREATE;
}

// 040114,poopee
void get_version(int prc_idx, char *ver)
{
	strcpy(ver, version->ver[prc_idx]);
}
