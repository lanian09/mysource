#ifndef __UTILLIB_HEADER_FILE___
#define __UTILLIB_HEADER_FILE___

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <comm_typedef.h>

#define DEF_DUP_ID  1
#define NON_DUP_ID  2

int dTCPUDP_Duplicate_Message( unsigned int uiCompID[], unsigned int uiCompOffset[], 
										unsigned int uiCurID, unsigned int uiCurOffset );
int dONLY_TCP_Duplicate_Message( unsigned int *uiLowerID, unsigned int *uiHigherID, unsigned int uiCurID );

int Init_logdebug( pid_t proc_idx, char *proc_name, char *logfilepath );
int Init_MPAppLog( pid_t proc_idx, char *proc_name, int procnum, char *logfilepath );
int Init_logbeacon( char *logfilepath );
int Init_logerror( char *logfilepath );

int log_close();
int log_debug(char *fmt, ... );
int log_write(char *fmt, ... );
int log_hexa(unsigned char *fmt, int dSize );
int log_hexa2(unsigned char *fmt, int dSize );
int log_hexa3(unsigned char *fmt, int dSize );
int log_beacon( unsigned char *puBuf, int size, int iFlag, int iRetCode, int iLogLevel ); 
int log_packet( unsigned char *puBuf, int size, int iFlag, int iRetCode, int iLogLevel ); 
int log_message( unsigned char *puBuf, int size, int iFlag, int iRetCode, int iLogLevel ); 
int dAppWrite( int dLevel, char *szMsg );
int dAppDebug( int dType, int dLevel, char *szMsg, struct tm *pstCheckTime, char *szName );
int dAppLog(int dIndex, char *fmt, ...);

time_t convert_time_t( time_t value );
short convert_short( short value );
unsigned short convert_ushort( unsigned short value );
int convert_int( int value );

long convert_long( long value );
unsigned long convert_ulong( unsigned long value );
long long convert_llong(long long value);

int dGetHostIP();
int dGetHostIP2( char *ipaddr );
int dGetIPAddr();
int dGetIPAddr_TAS();

int dGetIPAddr2(char *ipaddr);

USHORT CVT_USHORT( USHORT value );
INT CVT_INT( INT value );
UINT CVT_UINT( UINT value );
INT64 CVT_INT64( INT64 value );
CHAR *CVT_INT2STR_IP(UINT uiIP);
CHAR *CVT_INT2STR_NIP(UINT uiIP);
#endif

