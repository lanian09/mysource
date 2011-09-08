
/** A. Definition of New Constants ******************************************/
//#define MESSAGE_FILE 		"/var/log/messages"
//#define SYS_MESSAGE_FILE 		"/home/june/test/messages"
//#define SYS_MESSAGE_FILE_1		"/home/june/test/messages.1"
#define SYS_MESSAGE_FILE 		"/var/adm/messages"
#define SYS_MESSAGE_FILE_1		"/var/adm/messages.0"
#define SYS_MESSAGE_LINE_SIZE	512
#define HW_COUNT				5
/** B. Definition of Functions  *********************************************/
int	dStatusPower(char *szMsgBuf, int *dIndex);
int	dStatusFan(char *szMsgBuf, int *dIndex);
int strLen_LF(const char* psz);

/** C. Definition of Extern Functions  **************************************/
extern int dCheckHW_SysMsg (void);

/** D. Definition of Extern Variables ***************************************/
extern int 	errno;

