#ifndef _ERROR_CODE_TABLE
#define _ERROR_CODE_TABLE

/********************************************
ECODE_7XX, ELOG_7XX Reserved By KIW in 2004.2.14 
********************************************/

/* Etc Error... */
#define	ERR_Invalid_Length		-11
#define ERR_Wrong_Size			-12
#define ERR_000101				-101

/* non protocol */
#define ERR_000201              -201

/* general */
#define E_ParameterFormat		-1001
#define E_HTTPParcing			-1002

/* PPP */
#define ERR_010101              -10101
#define ERR_010102              -10102
#define ERR_010103              -10103
#define ERR_010104              -10104
#define ERR_010105              -10105
#define ERR_010106              -10106
#define ERR_010107              -10107
#define ERR_010108              -10108
#define ERR_010109              -10109
#define ERR_010110              -10110
#define ERR_010111              -10111
#define ERR_010112              -10112

/* CCP */
#define ERR_020101              -20101

/* CHAP */
#define ERR_030101              -30101

/* IPCP */
#define ERR_040101              -40101
#define ERR_040102              -40102
#define ERR_040103              -40103

/* LCP */
#define ERR_050101              -50101

/* PAP */
#define ERR_060101              -60101

/* MAC */
#define	ERR_200101				-200101
#define	ERR_200102				-200102

/* IP */
#define ERR_300101              -300101
#define ERR_300102              -300102
#define ERR_300103              -300103
#define ERR_300201              -300201
#define ERR_300301              -300301
#define ERR_300302				-300302

/* ICMP */
#define ERR_310101              -310101

/* OSPFIGP */
#define ERR_320101              -320101

/* IGMP */
#define ERR_330101              -330101
#define ERR_330102              -330102

/* EGP */
#define ERR_340101              -340101

/* GRE */
#define ERR_GRE_HeaderLenError	-350101
#define ERR_GRE_DataLenError	-350102

/* UDP */
#define ERR_360101              -360101
#define ERR_360201              -360201

/* TCP */
#define ERR_370101              -370101
#define ERR_370102              -370102

/* A11 */
#define ERR_450101              -450101
#define ERR_450102              -450102
#define ERR_450103              -450103
#define ERR_450104              -450104
#define ERR_450105              -450105
#define ERR_450106              -450106
#define ERR_450107              -450107
#define ERR_450108              -450108
#define ERR_450109              -450109
#define ERR_450110              -450110
#define ERR_450111              -450111
#define ERR_450112              -450112
#define ERR_450113              -450113
#define ERR_450114              -450114
#define ERR_450115              -450115

/* RADIUS */
#define ERR_480101              -480101
#define ERR_480102              -480102

/* RIP */
#define ERR_510101              -510101


/* ARP/RARP */
#define ERR_830101              -830101
#define ERR_830102              -830102


#define COMMENT_000101          "Incorrect Version"

#define COMMENT_000201          "Unknown Packet : size < 5"

#define COMMENT_010101          "[PPP] Size Error : size < 1"
#define COMMENT_010102          "[PPP] sl_uncompress_tcp Error : NULL(VJUCTCPIP)"
#define COMMENT_010103          "[PPP] Size Error : size > 1600"
#define COMMENT_010104          "[PPP] Size Error(PPP_LCP)"
#define COMMENT_010105          "[PPP] sl_uncompress_tcp Error : NULL(VJCTCPIP)"
#define COMMENT_010106          "[PPP] Size Error(PPP_PAP)"
#define COMMENT_010107          "[PPP] Size Error(PPP_IPCP)"
#define COMMENT_010108          "[PPP] Size Error(PPP_CHAP)"
#define COMMENT_010109          "[PPP] Size Error(PPP_CCP)"
#define COMMENT_010110          "[PPP] Size Error(PPP_IP)"
#define COMMENT_010111          "[PPP] Size Error(PPP_VJCUTCPIP)"
#define COMMENT_010112          "[PPP] Size Error(PPP_VJCTCPIP)"

#define COMMENT_020101          "[CCP] Header Length Error"
#define COMMENT_030101          "[CHAP] Header Length Error"

#define COMMENT_040101          "[IPCP] Header Length Error"
#define COMMENT_040102          "[IPCP] Length Error" 
#define COMMENT_040103          "[IPCP] Length Error" 

#define COMMENT_050101          "[LCP] Header Length Error"
#define COMMENT_060101          "[PAP] Header Length Error"

#define COMMENT_300101          "[IP] Total Length Error"
#define COMMENT_300102          "[IP] Header Length Error"
#define COMMENT_300103          "[IP] Unknown Version"
#define COMMENT_300302			"[IP] Flagments"

#define COMMENT_310101          "[ICMP] Header Length Error"
#define COMMENT_320101          "[OSPF] Header Length Error"

#define COMMENT_330101          "[IGMPv1] Total Length Error"
#define COMMENT_330102          "[IGMPv2] Total Length Error"

#define COMMENT_340101          "[EGP] Header Length Error"

#define COMMENT_350101          "[GRE] Header Length Error"
#define COMMENT_350102          "[GRE] Total Length Error"

#define COMMENT_360101          "[UDP] Header Length Error"
#define COMMENT_370101          "[TCP] Header Length Error"
#define COMMENT_370102          "[TCP] Packet Size Error"

#define COMMENT_450101          "[A11] Unknown message"
#define COMMENT_450102          "[A11] Total Length Error : Registration Request Message"
#define COMMENT_450103          "[A11] Total Length Error : Registration Reply Message"
#define COMMENT_450104          "[A11] Total Length Error : Registration Update Message"
#define COMMENT_450105          "[A11] Total Length Error : Registration Acknowledge Message"
#define COMMENT_450106          "[A11] Extension Length Error "
#define COMMENT_450107          "[A11] Extension Length Error - 2 "
#define COMMENT_450108          "[A11] Extension Length Error - 2 "
#define COMMENT_450109          "[A11] Extension Length Error - 2 "
#define COMMENT_450110          "[A11] Extension Length Error - 2 "
#define COMMENT_450111          "[A11] Extension Length Error - 2 "
#define COMMENT_450112          "[A11] Extension Length Error - 2 "
#define COMMENT_450113          "[A11] Extension Length Error - 2 "
#define COMMENT_450114          "[A11] Extension Length Error - 2 "
#define COMMENT_450115          "[A11] Extension Length Error - 2 "

#define COMMENT_480101          "[RADIUS] Total Length Error"
#define COMMENT_480102          "[RADIUS] Length Error"

#define COMMENT_510101          "[RIP] Total Length Error"

#define COMMENT_830101          "[ARP] Total Length Error"
#define COMMENT_830102          "[RARP] Total Length Error"

// DTAF Error Code Define

#define ELOG_LENGTH				128

#define ECODE_101				101
#define ECODE_102				102
#define ECODE_103				103

#define ECODE_201				201
#define ECODE_202				202
#define ECODE_203				203
#define ECODE_204				204

#define ECODE_301				301
#define ECODE_302				302
#define ECODE_303				303
#define ECODE_304				304

#define ECODE_401				401
#define ECODE_402				402

#define ECODE_501				501
#define ECODE_502				502
#define ECODE_503				503
#define ECODE_504				504
#define ECODE_505				505
#define ECODE_506				506
#define ECODE_507				507
#define ECODE_508				508
#define ECODE_509				509
#define ECODE_510				510

#define ECODE_511				511
#define ECODE_512				512
#define ECODE_513				513
#define ECODE_514				514
#define ECODE_515				515
#define ECODE_516				516
#define ECODE_517				517
#define ECODE_518				518
#define ECODE_519				519
#define ECODE_520				520
#define ECODE_521				521
#define ECODE_524				524

/*** dark264 20040211 Add ***/
#define ECODE_530				530
#define	ECODE_600				600
#define ECODE_601				601


/******* Defined By KIW in 2004.2.14 , 7XX Err Num is reserved by KIW!!!!! ***********/
#define ECODE_700				700
#define ECODE_799				799


#define ELOG_101 	"Analyze Error : AnalyzeEth_Info()"
#define ELOG_102 	"Analyze Error : AnalyzePPP_Info()"
#define ELOG_103 	"Analyze Error : AnalyzeSerial_Info()"

#define ELOG_201 	"Can not PPP Merge, Length or Buf Pointer Error : dMergePPPData()"
#define ELOG_202 	"Insert PPP Session Buffer Overlaped : dInsertPSESS()"
#define ELOG_203 	"PPP State Error : dGetPPP7EState()"
#define ELOG_204 	"Can not PPP Merge, Length or Buf Pointer Error2 : dMergePPPData()"

#define ELOG_301 	"Get LDGS IP Address Failed : dGetIPAddr_TAS()"
#define ELOG_302 	"TCP/IP Connection Failed : Init_Tcpip()"
#define ELOG_303 	"TCP/IP Connection Retry Failed : Init_Tcpip()"
#define ELOG_304 	"Send Message Failed : send_message()"

#define ELOG_401 	"Incorrect Service Number : dSelectService()"
#define ELOG_402 	"Unknown Service Number : dSelectService()"

#define ELOG_501 	"Initialize Share Memory Failed : Init_shm()"

#define ELOG_502 	"Insert MMDB Failed : Insert_PCTRLA()"
#define ELOG_503 	"Insert MMDB Failed : Insert_PCTRLB()"
#define ELOG_504 	"Insert MMDB Failed : Insert_STATFAIL()"
#define ELOG_505 	"Insert MMDB Failed : Insert_STATTCP()"
#define ELOG_506 	"Insert MMDB Failed : Insert_STATURL()"
#define ELOG_507 	"Insert MMDB Failed : Insert_MAGICN()"
#define ELOG_508 	"Insert MMDB Failed : Insert_PSESS()"
#define ELOG_509 	"Insert MMDB Failed : Insert_STATAPP()"
#define ELOG_510 	"Insert MMDB Failed : Insert_STATDEST()"
#define ELOG_511 	"Insert MMDB Failed : Insert_STATDESTAPP()"
#define ELOG_512 	"Insert MMDB Failed : Insert_STATSVC()"
#define ELOG_513 	"Insert MMDB Failed : Insert_STATURL()"
#define ELOG_514 	"Insert MMDB Failed : Insert_TSESS()"

#define ELOG_515 	"Read Buffer Critical Error : Read_m_shm()"
#define ELOG_516 	"Read Buffer Critical Error : Read_mp_shm()"
#define ELOG_517 	"Read Buffer Critical Error : Read_Trace_shm()"

#define ELOG_518 	"Write Buffer Full : Write_mp_shm()"
#define ELOG_519 	"Write Buffer Full : Write_Trace_shm()"

#define ELOG_520 	"Insert MMDB Failed : Insert_STATRADIUS()"
#define ELOG_521 	"Insert MMDB Failed : Insert_PCTRLB_TOTAL()"

#define ELOG_522	"Current Session Mismatch : PPP"
#define ELOG_523	"Current Session Mismatch : TCP"

#define ELOG_524	"Insert MMDB Failed : Insert_PCTRLC()"
#define ELOG_525	"Insert MMDB Failed : Insert_STATSYS()"

/*** dark264 20040211 Add ***/
#define ELOG_530	"Insert MMDB Failed : Insert_HTTPSESS()"

#define	ELOG_600	"Init_msgq Failed : Init_msgq()"
#define	ELOG_601	"Failed in msgsnd()"


/*********** Defined By KIW in 2004.2.14 , 7XX error msg is reserved by KIW !!! **********/

#define	ELOG_700	"Input Parameter is Illegal"
#define	ELOG_799	"Debugging out"



#define	E_A_APP_STATD		101000
#define	E_A_CALL_TRACED		102000
#define	E_A_DESTAPP_STATD	103000
#define	E_A_DEST_STATD		104000
#define	E_A_LAYER_STATD		105000
#define	E_A_PPP_TRACED		106000
#define	E_A_RPLAYER_STATD	107000
#define	E_A_SVC_STATD		108000
#define	E_A_TCP_STATD		109000
#define	E_A_URL_STATD		110000
#define	E_PPP_MRGD			111000
#define	E_A_RADIUS_STATD	112000
#define	E_A_SYS_STATD		113000
/*** added by Mihee 2002-08-03 */
#define E_A_PKT_TRACED		114000
/***/

#define	E_S_APP_STATD		201000
#define	E_S_CALL_TRACED		202000
#define	E_S_DESTAPP_STATD	203000
#define	E_S_DEST_STATD		204000
#define	E_DLOG_CTRL			205000
#define	E_S_LAYER_STATD		206000
#define	E_S_SYS_STATD		207000
#define	E_S_FAIL_STATD		208000
#define	E_S_PPP_STATD		209000
#define	E_S_PPP_TRACED		210000
#define	E_S_RADIUS_STATD	211000
#define	E_S_SVC_STATD		212000
#define	E_S_TCP_STATD		213000
#define	E_S_URL_STATD		214000
/*** added by Mihee 2002-08-03 */
#define E_S_PACKET_LOG		215000
#define	E_S_FAULT_LOG		216000
/***/

#define	E_ETH_CAPD			301000

#endif
