#ifndef __OMP_FILEPATH_H__
#define __OMP_FILEPATH_H__

#define STATD_MAIN_PATH         "LOG/STAT/"

#define MMCD_TRCLOG_FILE		"LOG/OAM/mmcd_log"
#define MMCD_ERRLOG_FILE		"LOG/OAM/mmcd_err"
#define MMCD_CMDHIS_FILE		"LOG/COMMAND_HIS"  // command history file
#define MMCD_MMCLOG_FILE		"LOG/MMC_MSG_LOG"  // mmc 입/출력 메시지 log file
#define FIMD_TRCLOG_FILE		"LOG/OAM/fimd_log"
#define FIMD_ERRLOG_FILE		"LOG/OAM/fimd_err"
#define STMD_TRCLOG_FILE		"LOG/OAM/stmd_log"
#define STMD_ERRLOG_FILE		"LOG/OAM/stmd_err"
#define COND_TRCLOG_FILE		"LOG/OAM/cond_log"
#define COND_ERRLOG_FILE		"LOG/OAM/cond_err"
#define COND_TRACE_FILE         "LOG/TRC_MSG_LOG" // TRACE 파일 저장 
#define COND_ALMLOG_FILE		"LOG/ALM_MSG_LOG"
#define COND_STSLOG_FILE		"LOG/STS_MSG_LOG"
/** TRACE 메시지 */
#define COND_TRACE_FILE			"LOG/TRC_MSG_LOG" // TRACE 메시지를 저장. 
#define NMSIB_TRCLOG_FILE		"LOG/OAM/nmsib_log"
#define NMSIB_ERRLOG_FILE		"LOG/OAM/nmsib_err"
#define MCDM_TRCLOG_FILE		"LOG/OAM/mcdm_log"
#define MCDM_ERRLOG_FILE		"LOG/OAM/mcdm_err"
#define NTPB_TRCLOG_FILE		"LOG/OAM/ntpb_log"
#define NTPB_ERRLOG_FILE		"LOG/OAM/ntpb_err"
#define UDRCOL_TRCLOG_FILE		"LOG/OAM/udrcol_err"
#define UDRCOL_ERRLOG_FILE		"LOG/OAM/udrcol_err"
#define NMSIF_TRCLOG_FILE		"LOG/OAM/nmsif_log"
#define NMSIF_ERRLOG_FILE		"LOG/OAM/nmsif_err"
#define CDELAY_TRCLOG_FILE		"LOG/OAM/cdelay_log"
#define CDELAY_ERRLOG_FILE		"LOG/OAM/cdelay_err"

#define MCDM_DISTRIB_MMC_FILE	"DATA/mcdm_distr_mmc"

// 보관주기 설정 파일 
#define STMD_DEL_TIME_FILE	"DATA/stmd_del_time"

// 주기적 통계를 위한 정보
#define STMD_PRINT_TIME_FILE	"DATA/stmd_print_time"
#define STMD_PRINT_MASK_FILE	"DATA/stmd_print_mask"
#define STMD_CRONJOB_FILE		"DATA/stmd_cronjob"

// NMS parsing을 위한 정보
#define NMSIB_MSG_DATA_FILE     "DATA/nmsib_msg_data"


#endif //__OMP_FILEPATH_H__
