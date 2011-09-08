#ifndef __FILELIB_H__
#define __FILELIB_H_

#define F_PERM			 0666
#define BUFLEN           256
#define PRC_NAME_LEN     12

#define E_FILE_GENERIC   -1
#define E_FILE_EXIST     -2
#define E_FILE_TYPE      -3
#define E_FILE_FUNC      -4
#define E_FILE_OPEN		 -5
#define E_FILE_WRITE	 -6
#define E_FILE_READ      -7
#define E_FILE_PARTIAL   -8
#define E_FILE_NIL		 -9

#define FILE_SUCCESS     0

#define MAX_QUE_CNT     400
#define MAX_SHM_CNT     400
#define MAX_SEM_CNT     400

/* QUEUE API STRUCTURE */
typedef struct _st_QUEUE_INFO
{
	int    dQueCnt;
	int    dNifoQueCnt;
	int    dQueList[MAX_QUE_CNT];
	int    dNifoQueList[MAX_QUE_CNT];
	char   sQueName[MAX_QUE_CNT][20];
	char   sNifoQueName[MAX_QUE_CNT][20];
} st_QUEUE_INFO, *pst_QUEUE_INFO;

/* SHARED MEMORY API STRUCTURE */
typedef struct _st_SHM_INFO
{
	int    dShmCnt;
	int    dShmList[MAX_SHM_CNT];
	char   sShmName[MAX_SHM_CNT][20];
} st_SHM_INFO, *pst_SHM_INFO;

/* SEMAPHORE API STRUCTURE */
typedef struct _st_SEM_INFO
{
	int    dSemCnt;
	int    dSemList[MAX_SEM_CNT];
	char   sSemName[MAX_SEM_CNT][20];
} st_SEM_INFO, *pst_SEM_INFO;

/* file_func.c */
extern int write_file(char *szFilePath, char *data_ptr, int write_len, int write_idx);
extern int read_file(char *szFilePath, char *data_ptr, int read_len, int read_idx);
extern int get_ip_conf(char *szFilePath, char *primary, char *secondary);
extern int get_db_conf(char *szFileName, char *szIP, char *szName, char *szPass, char *szAlias);

/* file_mcinit.c */
//GET QUEUE,SHARED MEMORY,SEMAPHORE KEY(Only One)
extern int get_que_key(char *szFileName, char *szBlockName);
extern int get_shm_key(char *szFileName, char *szShmName);
extern int get_sema_key(char *szFileName, char *szSemName);
//GET QUEUE,SHARED MEMORY,SEMAPHORE INFO 
extern int get_que_info(char *szFileName, st_QUEUE_INFO *pQueInfo);
extern int get_shm_info(char *szFileName, st_SHM_INFO *pShmInfo);
extern int get_sema_info(char *szFileName, st_SEM_INFO *pSemInfo);
//GET  NIFO Y/N, PROCESS SEQUENCE
extern int get_nifo_useyn(char *szFileName, char *szBlockName, char *szNifoYn);
extern int get_proc_seq(char *szFileName, char *szBlockName);

extern int get_block_num(char *szFileName, char *szBlockName);

#endif /* __FILELIB_H_ */
