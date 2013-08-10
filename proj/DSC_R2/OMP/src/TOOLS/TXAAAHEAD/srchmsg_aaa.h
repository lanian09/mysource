#ifndef _SRCHMSG_H
#define _SRCHMSG_H

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
#define NUMTYPE_MSISDN          2
#define NUMTYPE_ALL             0

typedef struct {
    int         viewtype;       /* NORM, LINE */
    int         msgtype;        /* NAS, QUD, AAA, DSCP, IDR */
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



#define RESULT_PATH             "/BSDM/RESULT"

#endif  /* _SRCHMSG_H */
