/* @(#)toto_error.h	1.1 3/1/02 */
#ifndef _TOTO_ERROR_H
#define _TOTO_ERROR_H

// ########## System Internal Error (1-4999)
#define ERROR_SYST_GENE			1000		// System General Error
#define ERROR_SYST_INIT			1100		// System Init Process Error
#define ERROR_SYST_INIT_DATA	3002		// System Init Data Error
#define ERROR_SYST_COMM			2000		// Communication Error
#define ERROR_SYST_MSGQ			2100		// Message Queue Communication Error
#define ERROR_SYST_MSGQ_RECV	2101		// Message Queue Receive Error
#define ERROR_SYST_MSGQ_SEND	2102		// Message Queue Send Error
#define ERROR_SYST_MSGQ_GETI	2103		// Message Queue Get Error
#define ERROR_SYST_MSGQ_TYPE	2104		// Message Type Error
#define ERROR_SYST_SOCK			2200		// Socket Communication Error
#define ERROR_SYST_SCOK_RECV	2201		// Socket Receive Error
#define ERROR_SYST_SOCK_SEND	2202		// Socket Send Error
#define ERROR_SYST_SOCK_BIND	2203		// Scoket Bind Error
#define ERROR_SYST_DATA			3000		// Database Error
#define ERROR_SYST_DATA_CONN	3001		// Database Connection Error
#define ERROR_SYST_DATA_MMDB	3002		// MMDB Databse Error
#define ERROR_SYST_ETCI			4000		// ETC Information Error

#define ERROR_SYST_WARN			4900		// System General Warning
#define ERROR_SYST_WARN_MSGQ	4910		// System Message Queue Warning

#define ERROR_IPAF_LOGOUT		6001
#define ERROR_IPAF_SOCK			6002
#define ERROR_IPAF_IDX			6003
#define	ERROR_IPAF_NOT_LIST		6004

// ########## General Information Error (5000-9999)
#define ERROR_GENE 				5000
#define ERROR_GENE_PACK			5100		// Packet Error
#define ERRPR_GENE_PACK_SRID	5101		// Srvice ID Error
#define ERRPR_GENE_PACK_MSID	5102		// Message ID Error
#define ERROR_GENE_PACK_ENCO	5103		// Packet Message Encoding Error
#define ERROR_GENE_PACK_EXTE	5104		// Packet Extended Message Encoding Error
#define ERROR_GENE_PACK_LENG	5110		// Packet Length Error

#define ERROR_GENE_WARN			9900

// ##########  Agency Information Error (10000 - 19999)
// Agency General Information Error
#define ERROR_AGEN				10000		// Agency Area Inforamtion General Error

// Agency Information Error
#define ERROR_AGEN_GENE			11000		// Agency Information General Error
#define ERROR_AGEN_EXIS			11001		// Agency Information Aleardy Exist Error
#define ERROR_AGEN_NOEX			11002		// Agency Information Not Exist Error
#define ERROR_AGEN_STAT			11010		// Agency Status Information Error
#define ERROR_AGEN_STAT_STOP	11011		// Agency Status Sale Stop Error

#define ERROR_AGEN_INFO			11100		// Information Error
#define ERROR_AGEN_INFO_TOTL	11110		// Total Limmits Error
#define ERROR_AGEN_INFO_SUML	11120		// Summation Limmits Error
#define ERROR_AGEN_INSE			11200		// Insert Error
#define ERROR_AGEN_UPDA			11300		// Update Error
#define ERROR_AGEN_DELE			11400		// Delete Error

#define ERROR_AGEN_WARN			11900		// Warning Information
 
// Shop Information Error
#define ERROR_SHOP_GENE			12000		// Shop Information General Error
#define ERROR_SHOP_EXIS			12001		// Shop Information Aleardy Exist Error
#define ERROR_SHOP_NOEX			12002		// Shop Information Not Exist Error
#define ERROR_SHOP_STAT			12010		// Shop Status Information Error

#define ERROR_SHOP_INFO			12100		// Information Error
#define ERROR_SHOP_INSE			12200		// Insert Error
#define ERROR_SHOP_UPDA			12300		// Update Error
#define ERROR_SHOP_DELE			12400		// Delete Error

#define ERROR_SHOP_WARN			12900		// Warning Information

// Tag Information Error
#define ERROR_TAGI_GENE			13000		// Tag Information General Error
#define ERROR_TAGI_EXIS			13001		// Tag Information Aleardy Exist Error
#define ERROR_TAGI_NOEX			13002		// Tag Information Not Exist Error
#define ERROR_TAGI_STAT			13010		// Tag Status Information Error

#define ERROR_TAGI_INFO			13100		// Information Error
#define ERROR_TAGI_INSE			13200		// Insert Error
#define ERROR_TAGI_UPDA			13300		// Update Error
#define ERROR_TAGI_DELE			13400		// Delete Error

#define ERROR_TAGI_WARN			13900		// Warning Information
#define ERROR_TAGI_ALLG			13901		// ALREADY LOGIN PLEASE LOGOUT FIRST

// Tag Oper Information Error
#define ERROR_OPER_GENE			14000		// Tag Operator Information Error
#define ERROR_OPER_EXIS			14001		// Tag Operator Information Aleardy Exist Error
#define ERROR_OPER_NOEX			14002		// Tag Operator Information Not Exist Error
#define ERROR_OPER_BOUN			14003		// Tag Operator ID Boundary Error(0-10)
#define ERROR_OPER_STAT			14010		// Tag Operator Status Error
#define ERROR_OPER_LOGI			14011		// Tag Operator Login Error
#define ERROR_OPER_LOGO			14012		// Tag Operator LogOut Error

#define ERROR_OPER_INFO			14100		// Information Error
#define ERROR_OPER_INFO_SUPE	14101		// Tag Super Operator Password Error
#define ERROR_OPER_INFO_PASS	14102		// Tag Operator Password Error
#define ERROR_OPER_INSE			14200		// Insert Error
#define ERROR_OPER_UPDA			14300		// Update Error
#define ERROR_OPER_UPDA_PASS	14311		// Update Operator Password 
#define ERROR_OPER_DELE			14400		// Delete Error

#define ERROR_OPER_WARN 		14900		// Warning Information

// ##########  Program Area Information Error(20000)
// Program General Information Error
#define ERROR_PROG				20000		// Program Information General Error
#define ERROR_PROG_GMID			20001		// Game ID Error

// Program Information Error
#define ERROR_PROG_GENE			21000		// Service Program General Error
#define ERROR_PROG_EXIS			21001		// Service Program Information Aleardy Exist Error
#define ERROR_PROG_NOEX			21002		// Service Program Information Not Exist Error
#define ERROR_PROG_CLOS			21003		// Program Not Close But EndTime < CurrTime
#define ERROR_PROG_STAT			21010		// Service Program Status Error

#define ERROR_PROG_INFO			21100		// Information Error
#define ERROR_PROG_INSE			21200		// Insert Error
#define ERROR_PROG_UPDA			21300		// Update Error
#define ERROR_PROG_DELE			21400		// Delete Error

#define ERROR_PROG_WARN			21900		// Warning Information

// Event Information Error
#define ERROR_EVEN_GENE			22000		// Service Event General Error
#define ERROR_EVEN_EXIS			22001		// Service Event Information Aleardy Exist Error
#define ERROR_EVEN_NOEX			22002		// Service Event Information Not Exist Error
#define ERROR_EVEN_STAT			22010		// Service Event Status Error

#define ERROR_EVEN_INFO			22100		// Information Error
#define ERROR_EVEN_INSE			22200		// Insert Error
#define ERROR_EVEN_UPDA			22300		// Update Error
#define ERROR_EVEN_DELE			22400		// Delete Error

#define ERROR_EVEN_WARN			22900		// Event Warning Information

// Winning Information Error
#define ERROR_WING_GENE			23000		// Winning General Error
#define ERROR_WING_EXIS			23001		// Winning Information Aleardy Exist Error
#define ERROR_WING_NOEX			23002		// Winning Information Not Exist Error
#define ERROR_WING_STAT			23010		// Winning Status Error

#define ERROR_WING_INFO			23100		// Information Error
#define ERROR_WING_INSE			23200		// Insert Error
#define ERROR_WING_UPDA			23300		// Update Error
#define ERROR_WING_DELE			23400		// Delete Error

#define ERROR_WING_WARN			23900		// Winning Warning Information

// Winner Information Error
#define ERROR_WINN_GENE			24000		// Winner General Error
#define ERROR_WINN_EXIS			24001		// Winner Information Aleardy Exist Error
#define ERROR_WINN_NOEX			24002		// Winner Information Not Exist Error
#define ERROR_WINN_STAT			24010		// Winner Status Error

#define ERROR_WINN_INFO			24100		// Information Error
#define ERROR_WINN_INSE			24200		// Insert Error
#define ERROR_WINN_UPDA			24300		// Update Error
#define ERROR_WINN_DELE			24400		// Delete Error

#define ERROR_WINN_WARN			24900		// Winner Warning Information

// ##########  Service Information Error(30000 - 50000)
// Service General Information Error
#define ERROR_SRVC				30000		// Service Information General Error
#define ERROR_SRVC_EXIS			30001		// Service Aleardy Exist Error
#define ERROR_SRVC_NOEX			30002		// Service Not Exist Error
#define ERROR_SRVC_STAT			30010		// Service Status Informaton Error
#define ERROR_SRVC_SLID			30020		// Invalid Sale ID
#define ERROR_SRVC_INTE			30040		// InterSale Error
#define ERROR_INTE_NOEX			30041		// InterSale Not Exist Error

// FOOT1X2 Service Information Error
#define ERROR_F1X2_GENE			31000		// FOOT1X2 Information General Error
#define ERROR_F1X2_EXIS_SLID	31001		// FOOT1X2 SaleID Aleardy Exist Error
#define ERROR_F1X2_NOEX_SLID	31002		// FOOT1X2 SaleID Not Exist Error
#define ERROR_F1X2_STAT			31010		// FOOT1X2 Status Information Error
#define ERROR_F1X2_STAT_SLID	31011		// FOOT1x2 SaleID Status Error

#define ERROR_F1X2_INFO			31100		// Information Error
#define ERROR_F1X2_SALE			31110		// Sale Information Error
#define ERROR_F1X2_SALE_PRIC	31111		// Sale Price Error
#define ERROR_F1X2_SALE_BUYT	31112		// Sale Buy Type Error
#define ERROR_F1X2_PAYM			31120		// Payment Information Error
#define ERROR_F1X2_PAYM_PRIC	31121		// Payment Prize Amount Error
#define ERROR_F1X2_REFU			31130		// Refund Information Error
#define ERROR_F1X2_REFU_PRIC	31131		// Refund Prize Amount Error
#define ERROR_F1X2_CNCL			31140		// Cancel Information Error
#define ERROR_F1X2_CNCL_PRIC	31141		// Cancel Prize Amount Error
#define ERROR_F1X2_CNCL_NORE	31142		// Cancel Cash Not Regi Error
#define ERROR_F1X2_CNCL_UPDA	31143		// Cancel Table Status Update Error
#define ERROR_F1X2_INSE			31200		// Insert Error
#define ERROR_F1X2_UPDA			31300		// Update Error
#define ERROR_F1X2_DELE			31400		// Delete Error

// LOTTO649 Service Information Error
#define ERROR_L649_GENE			32000		// LOTTO649 Information General Error
#define ERROR_L649_EXIS_LTID	32001		// LOTTO649 LottoID Aleardy Exist Error
#define ERROR_L649_NOEX_LTID	32002		// LOTTO649 LottoID Not Exist Error
#define ERROR_L649_STAT			32010		// LOTTO649 Status Information Error
#define ERROR_L649_STAT_LTID	32011		// LOTTO649 LottoID Statis Error

#define ERROR_L649_INFO			32100		// Information Error
#define ERROR_L649_SALE			32110		// Sale Information Error
#define ERROR_L649_SALE_PRIC	32111		// Sale Price Information Error
#define ERROR_L649_SALE_BUYT	32112		// Sale Buy Type Information Error
#define ERROR_L649_SALE_PDAY	32113		// Sale Prize Day Type Information Error
#define ERROR_L649_SALE_REPE	32114		// Sale Repeat Time  Day Type Information Error
#define ERROR_L649_PAYM			32120		// Payment Information Error
#define ERROR_L649_PAYM_PRIC	32121		// Payment Prize Amount Information Error
#define ERROR_L649_REFU			32130		// Refund Information Error
#define ERROR_L649_REFU_PRIC	32131		// Refund Prize Information Error
#define ERROR_L649_CNCL			32140		// Cancel Information Error
#define ERROR_L649_CNCL_PRIC	32141		// Cancel Price Information Error
#define ERROR_L649_CNCL_NORE	31142		// Cancel Cash Not Regi Error
#define ERROR_L649_INSE			32200		// Insert Error
#define ERROR_L649_UPDA			32300		// Update Error
#define ERROR_L649_DELE			32400		// Delete Error

//World Cup Lotto Information Error
#define ERROR_WORLD_GENE		33100
#define	ERROR_WORLD_NOEX_LTID	33101		//WORLD CUP Lotto Not Exist Error

// ##########  ETC Information Error(60000 - )
#define ERROR_ETCI_UNKW			50000		// Unknown Error
#define ERROR_SRVC_CARD			60000		// Card Error


#endif
