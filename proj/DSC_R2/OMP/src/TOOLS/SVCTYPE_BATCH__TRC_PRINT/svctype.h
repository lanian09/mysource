#ifndef SVCTYPE_H
#define SVCTYPE_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <comm_msgtypes.h>
#include <sysconf.h>
#include <socklib.h>
#include <conflib.h>

#define LOOPBACK_IP             "127.0.0.1"
#define ADD			"A"
#define DEL			"D"
#define CHG			"C"

#define SYSNAME_SIZE		4
#define FILE_COLUMNS_NUM	11
#define FILE_COLUMN_TOKEN	","
#define MAX_COMMAND_NUM		500

#define SYSTEM		"SYSTEM"
#define FAIL		'F'
#define SUCCESS		'S'
#define RESULT		"RESULT"
#define REASON		"REASON"

/* --------------- MMC COMMAND ---------------- */
#define CMD_ADD		"add-svc-type"
#define CMD_DEL		"del-svc-type"
#define CMD_CHG		"chg-svc-type"
#define CMD_DIS		"dis-svc-type"

//add-svc-type syntax --------> ADD-SVC-TYPE SYS, SVCTYPE, LAYER, FOUT, IP, NETMASK, PORT, BLOCK, URL_CHA, FIRST_UDR, [ALIAS]
#define CMD_STRING_ADD		CMD_ADD" %s,%s,%s,%s,%s,%d,%d,%s,%s,%s,%s"

//del-svc-type syntax --------> DEL-SVC-TYPE SYS, RECORDID
#define CMD_STRING_DEL		CMD_DEL" %s,%s"

//chg-svc-type syntax --------> CHG-SVC-TYPE SYS, RECORDID, [LAYER], [FOUT], [IP], [NETMASK], [PORT], [BLOCK], [URL_CHA], [FIRST_UDR],[ALIAS]
#define CMD_STRING_CHG		CMD_CHG" %s,%s,%s,%s,%s,%d,%d,%s,%s,%s,%s"

//dis-svc-type syntax --------> DIS-SVC-TYPE SYS, RECORDID
#define CMD_STRING_DIS		CMD_DIS" %s"	

/* -------------- MMC COMMAND --------------- */
#define BSDA		"BSDA"
#define BSDB		"BSDB"
#define ALL		"ALL"

typedef struct st_rec_svctype_key{
	unsigned int		ipaddr;
	char	netmask;
#define		TCP_LAYER	'T'
#define		TCP		"TCP"
#define		UDP_LAYER	'U'
#define		UDP		"UDP"
#define		IP_LAYER	'I'
#define		IP		"IP"
	char	layer;
	unsigned short	port;
}REC_SVCTYPE_KEY;

typedef struct st_rec_svctype{
	REC_SVCTYPE_KEY key;
	char	svctype[10];
	char	fout[4];
	char	block[20];
	char	url_cha[4];
	char	first_udr[4];	
	char	alias[50];
	char	recordid[10];
#define		ADD_ACTION	'A'
#define		DEL_ACTION	'D'
#define		CHG_ACTION	'C'
	char	action;
}REC_SVCTYPE;

typedef struct st_mmc_proc_res{
	char	sysname[10];
	char	SFflag;
	char	FailReason[256];
}MMC_PROC_RES;


/* --------------- ERROR DESC. INDEX ---------------- */
// File Parsing error
#define FPERR_WRONG_DELIMITER		0
#define FPERR_UNDEF_COMMANDTYPE		1
#define FPERR_UNKNOWN_LAYER		2
#define FPERR_WRONG_IPV4ADDR		3
#define FPERR_INVALID_NETMASK		4


extern char *ERRTABLE[];

void usage(void);
int get_cond_port_number(void);
int batch_main(char *hostIp, int portNum, char *batchfile);
int login2mmcd (int sockfd);
int sendmsg2mmcd (int sockfd, char *msgbody, char confirm, int batchFlag);
int file_exist(char *file);
char *current_time(void);
int load_batch_file(char *batchFile, REC_SVCTYPE *svcType[], int psize);
void print_struct_all(REC_SVCTYPE *svcType[], int maxsize);
void print_struct(REC_SVCTYPE *);
void fill_struct(REC_SVCTYPE *svcType, char values[][50]);
int conf_msg_parese(char *msgbody, REC_SVCTYPE *svcType[], int startpos, int maxsize);
int get_conf_from_mmcr(int sockfd, REC_SVCTYPE *svcType[], int psize);
int search_svctype(REC_SVCTYPE *svcType, REC_SVCTYPE *svcTypeTB[], int tbsize);
void processing_mmc(int sockfd, REC_SVCTYPE *cmdSvcType[], int size1, REC_SVCTYPE *confSvcType[], int size2);
void copyFeq2CR(char *s1, char *s2);
char *get_layer_name(char layer);
char *get_str_ipaddr(int nipaddr);
char *get_action_name(char action);
void removeCRLFofLine(char *s1);

#endif /* SVCTYPE_H */
