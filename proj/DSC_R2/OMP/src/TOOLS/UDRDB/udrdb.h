#include <stdio.h>
#include <dirent.h>
#include <mysql.h>

#include <ipaf_svc.h>
#include <udrgen_define.h>

#define		MAX_UDR_DATA		1024

#define		COMPRESS_FILE		1
#define		UNCOMPRESS_FILE		2

#define		REF_DIR				"/BSDM/UDR"
#define		UDRDB_ACCOUNT		"udrdb"
#define		UDRDB_PASSWORD		"udrdb"
#define		UDR_DATABASE		"UDRINFO"

#define		UDR_TABLE_NAME		"udr_info"
#define		ACCT_TABLE_NAME		"acct_info"
