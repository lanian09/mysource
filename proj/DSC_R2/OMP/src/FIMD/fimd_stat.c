#include "fimd_proto.h"

extern int		ixpcQid, eqSysCnt;
extern char		trcBuf[4096], trcTmp[1024];
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern SFM_sfdb		*sfdb;
extern STM_SysFltStat	almStat[SYSCONF_MAX_ASSO_SYS_NUM];
extern int	trcFlag, trcLogFlag;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void fimd_initSysAlmStat (void)
{
	int		i;

	memset ((void*)almStat, 0, sizeof(almStat));

	for (i=0; i<SYSCONF_MAX_ASSO_SYS_NUM; i++) {
		if (!strcasecmp (sfdb->sys[i].commInfo.name, ""))
			continue;
		strcpy (almStat[i].sysType,  sfdb->sys[i].commInfo.type);
		strcpy (almStat[i].sysGroup, sfdb->sys[i].commInfo.group);
		strcpy (almStat[i].sysName,  sfdb->sys[i].commInfo.name);
	}
	
	return;

} //----- End of fimd_initSysAlmStat -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void fimd_increaseAlmStat (int sysIndex, int almType, int almLevel)
{
	almLevel--; // index로 사용하기 위해 1을 뺀다.

	switch (almType) {
		case SFM_ALM_TYPE_CPU_USAGE:
			almStat[sysIndex].comm.cpu[almLevel]++;
			break;

		case SFM_ALM_TYPE_MEMORY_USAGE:
			almStat[sysIndex].comm.mem[almLevel]++;
#if 0
			sprintf(trcBuf,"[fimd_increaseAlmStat] MEM SYS[%d] LEVEL[%d] ALM[%d]\n"
					, sysIndex, almLevel, almStat[sysIndex].comm.mem[almLevel]);
			trclib_writeLogErr (FL,trcBuf);
#endif
			break;

		case SFM_ALM_TYPE_DISK_USAGE:
			almStat[sysIndex].comm.disk[almLevel]++;
			break;

		case SFM_ALM_TYPE_LAN:
			almStat[sysIndex].comm.lan[almLevel]++;
			break;
		
		case SFM_ALM_TYPE_PROC:
			almStat[sysIndex].comm.proc[almLevel]++;
			break;

		case SFM_ALM_TYPE_OPT_LAN: // by helca 09.13
			almStat[sysIndex].comm.optlan[almLevel]++;
			break;

		case SFM_ALM_TYPE_MP_HW:   // by helca 09.13
			almStat[sysIndex].comm.mp_hw[almLevel]++;
			break;
		
		case SFM_ALM_TYPE_DUP_HEARTBEAT:
		case SFM_ALM_TYPE_DUAL_ACT:
		case SFM_ALM_TYPE_DUAL_STD:
		case SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT:
			almStat[sysIndex].spec.u.bsd.dup_hb[almLevel]++;
			break;

		case SFM_ALM_TYPE_DUP_OOS:
			almStat[sysIndex].spec.u.bsd.dup_oos[almLevel]++;
			break;

		#if 0
		case SFM_ALM_TYPE_DUAL_ACT:
			almStat[sysIndex].spec.u.bsd.dualAct[almLevel]++;
			break;

		case SFM_ALM_TYPE_DUAL_STD:
			almStat[sysIndex].spec.u.bsd.dualStd[almLevel]++;
			break;

		case SFM_ALM_TYPE_DUAL_STS_QRY_TIME_OUT:
			almStat[sysIndex].spec.u.bsd.dualTimeOut[almLevel]++;
			break;
		#endif
	
		// by helca 08.11
		case SFM_ALM_TYPE_DBCON_STST:
			almStat[sysIndex].spec.u.bsd.sess_uawap[almLevel]++;
			break;
			
		// by helca 11.7
       	case SFM_ALM_TYPE_NMSIF_CONNECT:
           	if(sysIndex == 0)
               	almStat[sysIndex].comm.net_nms[almLevel]++;
           	break;
        // 20100915 by dcham
		case SFM_ALM_TYPE_SCM_FAULTED:
			almStat[sysIndex].spec.u.bsd.scm_fault[almLevel]++;
			break;

		/*
		case SFM_ALM_TYPE_QUEUE_LOAD:
			almStat[sysIndex].comm.queue[almLevel]++;
			break;
        */

		/* hjjung_20100823 */
		case SFM_ALM_TYPE_LEG_SESSION:
			almStat[sysIndex].comm.legSession[almLevel]++;
			break;
		case SFM_ALM_TYPE_TPS: // TPS Alarm added by dcham 2011.05.25
			almStat[sysIndex].comm.tpsStatus[almLevel]++;
			break;

	}

	return;

} //----- End of fimd_increaseAlmStat -----//

// by helca 07.31
void fimd_increaseAlmStatIndex (int sysIndex, int subIndex, int almType, int almLevel)
{
	almLevel--; // index로 사용하기 위해 1을 뺀다.
	if (almLevel >= SFM_ALM_CRITICAL) return;

	switch (almType) {
/*
		case SFM_ALM_TYPE_SUCC_RATE:
			if(subIndex ==0) {
				almStat[sysIndex].spec.u.bsd.succ_uawap[almLevel]++;
				break;
			}else if (subIndex == 1) {
				almStat[sysIndex].spec.u.bsd.succ_aaa[almLevel]++;
				break;
			}else if (subIndex == 2) {
				almStat[sysIndex].spec.u.bsd.succ_wap1ana[almLevel]++;
				break;
			}else if (subIndex == 3) {
				almStat[sysIndex].spec.u.bsd.succ_wap2ana[almLevel]++;
				break;
			}else if (subIndex == 4) {
				almStat[sysIndex].spec.u.bsd.succ_httpana[almLevel]++;
				break;
		   	}else if (subIndex == 5) {
				almStat[sysIndex].spec.u.bsd.succ_vods[almLevel]++;
				break;
           	}else if (subIndex == 6) {
				almStat[sysIndex].spec.u.bsd.succ_anaaa[almLevel]++;
				break;
           	}else if (subIndex == 7) {
				almStat[sysIndex].spec.u.bsd.succ_vt[almLevel]++;
				break;
			}else if (subIndex == 8) {
				almStat[sysIndex].spec.u.bsd.succ_radius[almLevel]++;
				break;
			}
			else
				break;	
*/		
		// by helca 08.10 ////
		//case SFM_ALM_TYPE_PD_CPU_USAGE:
		case SFM_ALM_TYPE_TAP_CPU_USAGE:
			almStat[sysIndex].comm.pdcpu[subIndex][almLevel]++;
			break;
	
		//case SFM_ALM_TYPE_PD_MEMORY_USAGE:
		case SFM_ALM_TYPE_TAP_MEMORY_USAGE:
			almStat[sysIndex].comm.pdmem[subIndex][almLevel]++;
			break;

		//case SFM_ALM_TYPE_PD_FAN_STS:
		case SFM_ALM_TYPE_TAP_FAN_STS:
			almStat[sysIndex].comm.pdfan[subIndex][almLevel]++;
			break;

			//case SFM_ALM_TYPE_PD_GIGA_LAN:
		case SFM_ALM_TYPE_TAP_PORT_STS:
			almStat[sysIndex].comm.pdgiga[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_TAP_POWER_STS: // 20110424 by dcham
			almStat[sysIndex].comm.pdpower[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_RSRC_LOAD:
			almStat[sysIndex].spec.u.bsd.rsrc[almLevel]++;
			break;

		case SFM_ALM_TYPE_HWNTP:
			if(sysIndex == 0) almStat[sysIndex].comm.sess_ntp[subIndex][almLevel]++;
			else {
				almStat[sysIndex].spec.u.bsd.hwntp[subIndex][almLevel]++;
				almStat[sysIndex].spec.u.bsd.sess_ntp[almLevel]++;
			}
			break;
		
		// by helca 08.11
		case SFM_ALM_TYPE_RMT_LAN:
			if(!strncmp(sfdb->sys[sysIndex].commInfo.rmtLanInfo[subIndex].name, "SCEA", 4))	{	// U -> SCEA
				almStat[sysIndex].spec.u.bsd.net_uawap[almLevel]++;
/* DEBUG: by june, 2010-10-07
 * DESC : LOG ADD
 */
#if 0
				sprintf(trcBuf,"[fimd_increaseAlmStatIndex] SCEA SYS[%d] ALM_LEVEL[%d] ALM_CNT[%d]\n"
						, sysIndex, almLevel, almStat[sysIndex].spec.u.bsd.net_uawap[almLevel]);
				trclib_writeLogErr (FL,trcBuf);
#endif
			}
			else if(!strncmp(sfdb->sys[sysIndex].commInfo.rmtLanInfo[subIndex].name, "SCEB", 4)) {	// A -> SCEB
				almStat[sysIndex].spec.u.bsd.net_aaa[almLevel]++;
#if 0
				sprintf(trcBuf,"[fimd_increaseAlmStatIndex] SCEB SYS[%d] ALM_LEVEL[%d] ALM_CNT[%d]\n"
						, sysIndex, almLevel, almStat[sysIndex].spec.u.bsd.net_aaa[almLevel]);
				trclib_writeLogErr (FL,trcBuf);
#endif
			}
			break;

		case SFM_ALM_TYPE_SCE_CPU:
			almStat[sysIndex].comm.sceCpu[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_MEM:
			almStat[sysIndex].comm.sceMem[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_DISK:
			almStat[sysIndex].comm.sceDisk[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_PWR:
			almStat[sysIndex].comm.scePwr[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_FAN:
			almStat[sysIndex].comm.sceFan[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_TEMP:
			almStat[sysIndex].comm.sceTemp[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_VOLT:
			almStat[sysIndex].comm.sceVolt[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_PORT_MGMT:
		case SFM_ALM_TYPE_SCE_PORT_LINK:
			almStat[sysIndex].comm.scePort[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_RDR:
			almStat[sysIndex].comm.sceRdr[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_RDR_CONN:
			almStat[sysIndex].comm.sceRdrConn[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_SCE_STATUS:
			almStat[sysIndex].comm.sceStatus[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_L2_CPU:
			almStat[sysIndex].comm.l2Cpu[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_L2_MEM:
			almStat[sysIndex].comm.l2Mem[subIndex][almLevel]++;
			break;
		case SFM_ALM_TYPE_L2_LAN:
			almStat[sysIndex].comm.l2Port[subIndex][almLevel]++;
			break;
		/* hjjung_20100823 */
		case SFM_ALM_TYPE_SCE_USER:
			almStat[sysIndex].comm.sceUser[subIndex][almLevel]++;
			break;

		/* added by uamyd 20110209. LOGON 성공율 감시를 위한 */
		case SFM_ALM_TYPE_LOGON_SUCCESS_RATE:
			almStat[sysIndex].comm.logonSuccRate[almLevel]++;
			break;

		case SFM_ALM_TYPE_LOGOUT_SUCCESS_RATE:
			almStat[sysIndex].comm.logoutSuccRate[almLevel]++;
			break;

		case SFM_ALM_TYPE_SM_CONN_STS:
			almStat[sysIndex].comm.smCh[subIndex][almLevel]++;
			break;
	}

	return;

} //----- End of fimd_increaseAlmStatIndex -----//


//------------------------------------------------------------------------------
// STMD로 수집된 System 장애 통계 데이터를 전송한다.
//------------------------------------------------------------------------------
void fimd_reportSysStatData2OMP  (IxpcQMsgType *rxIxpcMsg)
{
	int				txLen;
	GeneralQMsgType	txGenQMsg;
	IxpcQMsgType	*txIxpcMsg;
	STM_AlarmStatisticMsgType	*txAlmStatMsg;

	txGenQMsg.mtype = MTYPE_STATISTICS_REPORT;

	txIxpcMsg = (IxpcQMsgType*)txGenQMsg.body;
	memset ((void*)&txIxpcMsg->head, 0, sizeof(txIxpcMsg->head));

	txIxpcMsg->head.msgId = MSGID_FAULT_STATISTICS_REPORT;

	strcpy (txIxpcMsg->head.srcSysName, mySysName);
	strcpy (txIxpcMsg->head.srcAppName, myAppName);
	strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
	strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);

	// 수집된 통계 데이터를 복사한다.
	//
	txAlmStatMsg = (STM_AlarmStatisticMsgType*)txIxpcMsg->body;
	txAlmStatMsg->eqSysCnt = eqSysCnt;
//fprintf(stdout, "[%s:%d] sysCnt: %d\n", __FUNCTION__, __LINE__,  eqSysCnt);


	 // 유효한 영역만 복사한다.
	memcpy ((void*)txAlmStatMsg->sys, almStat, sizeof(STM_SysFltStat)*eqSysCnt);


	txIxpcMsg->head.bodyLen = sizeof(STM_AlarmStatisticMsgType)
								- sizeof(txAlmStatMsg->sys)
								+ (sizeof(STM_SysFltStat)*eqSysCnt);

	txLen = sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen;
	if (msgsnd (ixpcQid, (void*)&txGenQMsg, txLen, IPC_NOWAIT) < 0) {
		sprintf(trcBuf,"[fimd_reportSysStatData2OMP ] msgsnd fail; err=%d(%s)\n", errno, strerror(errno));
		trclib_writeLogErr (FL,trcBuf);
		return;
	} else {
		if (trcFlag || trcLogFlag) {
			sprintf(trcBuf,"[fimd_reportSysStatData2OMP ] send AlmStat data\n");
			trclib_writeLog (FL,trcBuf);
		}
	}

#if 0
	int i;
	for(i=0;i<eqSysCnt;i++)
		fprintf(stdout,"[%s] sysType(%d) : %s\n", __FUNCTION__,i, almStat[i].sysType);
	fprintf(stdout,"total snd : %d, body len : %d\n", txIxpcMsg->head.bodyLen, txLen);
	
#endif	


	fimd_initSysAlmStat (); // clear

	return;

} //----- End of fimd_reportSysStatData2OMP  -----//

