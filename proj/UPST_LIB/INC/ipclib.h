#ifndef __IPCLIB_H__
#define __IPCLIB_H__


#define SHMPERM       0666
#define QPERM         0666

#define E_SHM_GENERIC -1
#define E_SHM_CREATE  -2
#define E_SHM_ATTACH  -3
#define E_SHM_ATTACH2 -4
#define E_SHM_NOENT   -5
#define E_SHM_REMOVE  -6

#define E_MSGQ_GENERIC -1
#define E_MSGQ_GET     -2
#define E_MSGQ_RCV     -3
#define E_MSGQ_BUF     -4
#define E_MSGQ_SND     -5

#define SHM_EXIST     0
#define SHM_CREATE    1
#define SHM_ATTACH    2
#define SHM_REMOVE    3

#define MAXOBN  25592

typedef struct {
	long mtype;
	char mtext[MAXOBN];
} st_Qentry;

/* API */
extern int shm_init(int, long int, void**);
extern int shm_remove(int);

extern int msgq_init(key_t qkey);
extern int msgq_read(int qid, char *msg, int size, int flags);
extern int msgq_write(int qid, char *msg, int size, int flags);

#endif /* __IPCLIB_H__ */
