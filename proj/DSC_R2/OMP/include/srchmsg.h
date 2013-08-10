#ifndef _SRCHMSG_H
#define _SRCHMSG_H


#include <sys/types.h>
#include <sys/stat.h>
		   
#include <ipaf_svc.h>
#include <udrgen_define.h>

#define PW_ACCOUNTING_REQUEST   4
#define PW_ACCOUNTING_RESPONSE  5
#define PW_QUALIFICATION_DIR    125
#define PW_QUALIFICATION_RET    124
#define PW_DISCONNECT_REQ       40
#define PW_ACK                  41
#define PW_NACK                 42

#define MAX_FILENAME_LEN        128
#define MAX_IMSI_LEN            15
#define MAX_MSISDN_LEN          12

#define VIEWTYPE_NORM           1
#define VIEWTYPE_LINE           2
#define MSGTYPE_NAS             1
#define MSGTYPE_QUD             2
#define MSGTYPE_AAA             3
#define MSGTYPE_DSCP            4
#define MSGTYPE_IDR             5
#define SRCHTYPE_TIME           1
#define SRCHTYPE_FILE           2
#define NUMTYPE_IMSI            1
#define NUMTYPE_UDRSEQ          2
#define NUMTYPE_ALL             0
//#define UDRLOG_PATH             "/BSDM/UDR"
#define UDRLOG_PATH             "/DSCM/UDR"

#ifndef PASS_EXCEPTION
    #define PASS_EXCEPTION(condition, label) if ( (condition) ) \
							{ goto label; }
#endif

#ifndef PASS_CATCH
	#define PASS_CATCH(label)	goto PASS_CATCH_END_LABEL; label:
#endif

#ifndef PASS_CATCH_END
	#define PASS_CATCH_END 	PASS_CATCH_END_LABEL:
#endif




typedef struct {
    char        ucStartF;
    char        ucEndF;
    char        ucSearchFileF;
    char        ucImsiF;
    char        ucViewF;
    char        ucOutfileF;
    char        ucUdrSeqF;
    int         viewtype;       /* NORM, LINE */
    int         msgtype;        /* NAS, QUD, AAA, DSCP, IDR */
    unsigned int uiUdrSeq;
    time_t      starttime;
    time_t      endtime;
    char        filename[MAX_FILENAME_LEN+1];
    char        imsi[MAX_IMSI_LEN+1];
    char        msisdn[MAX_MSISDN_LEN+1];
    char        outfile[MAX_FILENAME_LEN+1];
    char        errfile[MAX_FILENAME_LEN+1];
    int         srchtype;       /* TIME, FILE */
    int         numtype;        /* IMSI, MSISDN, all */
    FILE        *fp;
    FILE        *err_fp;
    int         tot_file;       /* open files */
    int         tot_msg;        /* total messages in open files */
    int         tot_found;      /* condition matched messages */
    int         tot_written;    /* logged messages */
} SRCH_INFO;

typedef struct _strUdrFileName{

#define SWITCH_NAME_LEN         5
    char        sw[SWITCH_NAME_LEN];
    char        udrscore1;
    char        side;
    char        udrscore2;
    union{
        struct{
            char    year[4];
            char    mon[2];
            char    day[2];
            char    hour[2];
            char    min[2];
            char    sec[2];
        } ymdhms;
        char    time14s[14];
    }createtime;
    char        udrscore3;
#define ID_NAME_LEN             2
    char        id[ID_NAME_LEN];
#define SEQUENCE_LEN            7
    char        seq[SEQUENCE_LEN];
    char        dot;
#define EXTENSION_LEN           3
    char        ext[EXTENSION_LEN];
}UDRFilename;



//#define RESULT_PATH             "/BSDM/RESULT"
#define RESULT_PATH             "/DSCM/RESULT"

#endif  /* _SRCHMSG_H */
