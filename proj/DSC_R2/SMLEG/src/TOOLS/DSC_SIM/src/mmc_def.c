
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "mmc.h"

extern void cmd_quit(char*,...);

#if 0
extern void set_radius_ip (char*,...);
extern void set_radius_domain (char*,...);
#endif
extern void set_radius_SubsID (char*,...);
extern void set_radius_SubsIP (char*,...);
extern void set_radius_CBIT (char*,...);
extern void set_radius_PBIT (char*,...);
extern void set_radius_HBIT (char*,...);

extern void set_http (char*,...);
extern void set_client_cnt (char*,...);
extern void set_loop_num (char*,...);
extern void set_loglv_num (char*,...);

extern void reload_conf (char*,...);

extern void send_radius (char*,...);
extern void send_http_num (char*,...);

extern void send_http_num_only (int sockfd);

/* [CONFIG CONTROL]
 * SET-SEND-MGS		: 1=ACCOUNT_REQ_START, 2=ACCOUNT_REQ_STOP, 3=BOTH
 * SET-SEND-CNT		: SEND-MGS 를 보내는 갯수
 * [SUBSCRIBER CONTROL]
 * SET-SUBS-ID 		: subscriber id
 * SET-SUBS-IP		: subscriber ip
 * SET-SUBS-DOMAIN	: subscriber domain
 * SET-STATE 		: subscriber state
 */
mmc_t _mmc[]= {
/*  cmd str    depth/continueornot/token-type   parse-format            cmd-for-help                    callback-function   */
    {"help",        0,      1,      'C',        "help",                 "mmi help menual",              mmchelp},
    {"proc",        0,      1,      'C',        "proc",                 "process",                      NULL},
    {"show",        0,      1,      'C',        "show",                 "show info",                    NULL},
    {"set",         0,      1,      'C',        "set",                  "set mode",                     NULL},
    {"reload",      0,      1,      'C',        "reload",               "reload",                       NULL},
    {"send",      	0,      1,      'C',        "send",               	"send",                         NULL},
    {"hist",        0,      0,      'C',        "hist",                 "history",                      NULL},
    {"quit",        0,      0,      'C',        "quit",                 "quit program",                 cmd_quit},

/**< set **/
	{"radius", 		1,		1,		'C',    	"set-radius",      				"set radius",     						NULL},
/** set start  */
	{"start", 		2,		1,		'C',    	"set-radius-start",  			"set radius start", 					NULL},
	/* set radius subscriber id feild */
	{"subs_id", 	3,		1,		'C',    	"set-radius-start-subs_id",  	"set radius start subs_id", 			NULL},
	{"{value}",   	4,		0,		'N',  		"set-radius-start-subs_id-N", 	"set radius start subs_id-N", 			set_radius_SubsID},
	/* set radius subscriber ip feild */
	{"subs_ip", 	3,		1,		'C',    	"set-radius-start-subs_ip",  	"set radius start subs_ip", 			NULL},
	{"{value}",   	4,		0,		'I',  		"set-radius-start-subs_ip-I", 	"set radius start subs_ip I",			set_radius_SubsIP},
	/* set radius cbit feild */
	{"cbit", 		3,		1,		'C',    	"set-radius-start-cbit",  		"set radius start cbit"		, 			NULL},
	{"{value}",   	4,		0,		'N',  		"set-radius-start-cbit-N", 		"set radius start cbit Number", 		set_radius_CBIT},
	/* set radius pbit feild */
	{"pbit", 		3,		1,		'C',    	"set-radius-start-pbit",  		"set radius start pbit"		, 			NULL},
	{"{value}",   	4,		0,		'N',  		"set-radius-start-pbit-N", 		"set radius start pbit Number", 		set_radius_PBIT},
	/* set radius hbit feild */
	{"hbit", 		3,		1,		'C',    	"set-radius-start-hbit",  		"set radius start hbit"		, 			NULL},
	{"{value}",   	4,		0,		'N',  		"set-radius-start-hbit-N", 		"set radius start hbit Number", 		set_radius_HBIT},

/** set stop */
	{"stop", 		2,		1,		'C',    	"set-radius-stop",  			"set radius stop",	 					NULL},
	/* set radius subscriber id feild */
	{"subs_id", 	3,		1,		'C',    	"set-radius-stop-subs_id",  	"set radius stop subs_id", 				NULL},
	{"{value}",   	4,		0,		'N',  		"set-radius-stop-subs_id-N", 	"set radius stop subs_id-N", 			set_radius_SubsID},
	/* set radius subscriber ip feild */
	{"subs_ip", 	3,		1,		'C',    	"set-radius-stop-subs_ip",  	"set radius stop subs_ip", 				NULL},
	{"{value}",   	4,		0,		'I',  		"set-radius-stop-subs_ip-I", 	"set radius stop subs_ip I", 			set_radius_SubsIP},


	/* set http msg */
	{"http", 		2,		1,		'C',    	"set-send-http",      			"set send http", 				NULL},
	{"{value}",   	3,		0,		'N',  		"set-send-http-N",   			"set sned http type", 			set_http},

	{"loop", 		1,		1,		'C',    	"set-loop",      				"set loop send",     			NULL},
	{"{client_cnt}",2,		1,		'N',  		"set-loop-N",    				"set loop count", 				NULL},
	{"{loop_sec}",  3,		0,		'N',  		"set-loop-N-N",    				"set loop clicnt loop", 		set_loop_num},

	{"loglv", 		1,		1,		'C',    	"set-loglv",      				"set loglv",     				NULL},
	{"{1-4}",   	2,		0,		'N',  		"set-loglv-N",    				"set loglv value", 				set_loglv_num},

/**< reload **/
	{"conf", 		1,		1,		'C',    	"reload-conf",      			"reload conf",     				NULL},
	{"{value}",   	2,		0,		'S',  		"reload-conf-S",    			"reload conf file", 			reload_conf},

/**< send **/
	/* send radius msg */
	{"radius", 		1,		1,		'C',    	"send-radius",     				"send radius",     				NULL},
	{"start",   	2,		1,		'C',  		"send-radius-start", 			"send radius start", 			NULL},
	{"{num}",   	3,		0,		'N',  		"send-radius-start-N",			"send radius start send-count", send_radius},
	{"stop",   		2,		1,		'C',  		"send-radius-stop", 			"send radius stop", 			NULL},
	{"{num}",   	3,		0,		'N',  		"send-radius-stop-N",			"send radius stop send-count",  send_radius},
	/* send http msg */
	{"http", 		1,		1,		'C',    	"send-http",      				"send http",     				NULL},
	{"{num}",   	2,		0,		'N',  		"send-http-N",    				"send http GET ", 				send_http_num},

    {"",-1,0,0,"","",NULL}
};

