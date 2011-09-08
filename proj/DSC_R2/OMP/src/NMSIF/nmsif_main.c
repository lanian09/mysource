#include <nmsif_proto.h>
#include <nmsif.h>


int		nmsifQid;

SFM_sfdb	*sfdb;
MYSQL		*conn, sql;

extern		char	traceBuf[];
extern		int	trcFlag, trcLogFlag;
char    trcBuf[4096], trcTmp[1024];



void nmsif_quitSignal (int signo)
{
	int  i;

	for (i=0; i < MAX_NMS_CON; i++) {
		if (sfdb->nmsInfo.fd[i] > 0) {
			close_nms_con (sfdb->nmsInfo.fd[i]);
		}
	}

	trclib_writeLogErr (FL,">>> terminated by user request signo\n");
	exit(1);
} /** End of commlib_quitSignal **/

void nmsif_ignoreSignal (int signo)
{
	signal (signo, nmsif_ignoreSignal);
	return;
} /** End of commlib_quitSignal **/


void nsmif_setupSignals (int *notMaskSig)
{
	int		i, flag, *ptr;

	signal(SIGINT,  nmsif_quitSignal);
	signal(SIGTERM, nmsif_quitSignal);

	signal(SIGHUP,   nmsif_ignoreSignal);
	signal(SIGPIPE,  nmsif_ignoreSignal);
	signal(SIGALRM,  nmsif_ignoreSignal);
#ifdef TRU64
	for (i=16; i<=SIGMAX; i++)
#else
	for (i=16; i<=MAXSIG; i++)
#endif
	{
		// catch하지 말아야 할 놈으로 지정되었는지 확인한다.
		for (ptr = notMaskSig, flag=0; (ptr != NULL && *ptr != 0); ptr++) {
			if (*ptr == i) {
				flag = 1;
			}
		}
		if (flag) continue;

		signal(i, nmsif_ignoreSignal);
	}

	return;
}


main ()
{
	int		ret=0, l_cnt=0;
	int		r_sfd;
	int		k;
	char		rbuf[MAX_NMS_BUF];

	CommonPkt   *rx_pkt;
	Packet1		*rpkt1;
	Packet2		*rpkt2;
	Packet3		*rpkt3;
	GeneralQMsgType	rx_qmsg;

	if (init () < 0)
		exit (1);

	nsmif_setupSignals (NULL);
	//trcLogFlag = 1;

	if (trcFlag || trcLogFlag) {
		sprintf (traceBuf, "nmsif startup ...\n");
		trclib_writeLog (FL, traceBuf);
	}

	while (1) {

		/*******************************************************
		  1. Internal Interface Section (NMSIF/STMD+COND+FIMD) 
		 *******************************************************/
		memset (&rx_qmsg, 0, sizeof (GeneralQMsgType));
		while (msgrcv (nmsifQid, &rx_qmsg, sizeof (rx_qmsg), 0, IPC_NOWAIT) > 0) {

			switch (rx_qmsg.mtype) {


				/*******************************************************
				  1.1 Recv Statistics Notification (<-STMD)
				 *******************************************************/
				case MTYPE_STATISTICS_REPORT :

					/* - transform local format ->NMS format(oid) 
					 * - create File & send notification (->NMS)
					 */
					//					printf("Get MTYPE_STATISTICS_REPORT\n");
					handle_statistic ((GeneralQMsgType *)&rx_qmsg);

					break;

					/*******************************************************
					  1.2 Recv Console & Send Console Data (<-COND)
					 *******************************************************/
				case MTYPE_CONSOLE_REPORT :

					/* make NMS packet(text) & broadcast console
					 */
					//					printf("Get MTYPE_CONSOLE_REPORT\n");
					handle_console ((GeneralQMsgType *)&rx_qmsg);

					break;

					/*******************************************************
					  1.3 Recv Alarm & Send Alarm Data (<-FIMD)
					 *******************************************************/
				case MTYPE_ALARM_REPORT :

					/* make NMS packet(oid) & broadcast console
					 */
					//					printf("Get MTYPE_ALARM_REPORT\n");
					handle_alarm ((GeneralQMsgType *)&rx_qmsg);
					break;

					/*******************************************************
					  1.4 Recv MMC Request & Send Result
					 *******************************************************/
				case MTYPE_MMC_REQUEST :
					//					printf("Get MTYPE_MMC_REPORT\n");
					handle_mmc_request ((GeneralQMsgType *)&rx_qmsg);
					break;


				default :
					break;
			}
		}

		/*******************************************************
		 2. External Interface Section (DSCM/NMS)
		*******************************************************/
		memset (rbuf, 0, MAX_NMS_BUF);	ret=0;
		ret = lookup_tcpaction (rbuf, &r_sfd);

		if (ret == NMS_DATA_EVENT) {

			rx_pkt = (CommonPkt *)rbuf;

			/* packet 1 처리 : control packet ************************************
			*/
			if (rx_pkt->hdr.chd.pktype == 1) {
				rpkt1 = (Packet1 *)rx_pkt;
			}

			/* packet 2 처리 : console packet ************************************
			*/
			else if (rx_pkt->hdr.chd.pktype == 2) {

				rpkt2 = (Packet2 *)rx_pkt;

				/* H/W current_alarm 초기화 요청 
				*/
				if (rpkt2->msgtype == PRMT_INIT_HWALM_REQ) {

					/* send signal => confirm (packet2) 
					*/
					send_primitive (PRMT_INIT_HWALM_CONFIRM, (int *)&r_sfd);

					/* send data => current_alarm (packet3 : data1, 2, ... end) 
					*/
					send_primitive (PRMT_INIT_HWALM_DATA, (int *)&r_sfd);
				}

				/* H/W current_alarm 초기화 수신 응답 
				*/
				else if (rpkt2->msgtype == PRMT_INIT_HWALM_RCV_OK) {
					;
				}

				/* S/W current_alarm 초기화 요청
				*/
				else if (rpkt2->msgtype == PRMT_INIT_SWALM_REQ) {

					/* send signal => confirm (packet2) 
					*/
					send_primitive (PRMT_INIT_SWALM_CONFIRM, (int *)&r_sfd);

					/* send data => current_alarm (packet3 : data1, 2, ... end) 
					*/
					send_primitive (PRMT_INIT_SWALM_DATA, (int *)&r_sfd);
				}

				/* S/W current_alarm 초기화 수신 응답 
				*/
				else if (rpkt2->msgtype == PRMT_INIT_SWALM_RCV_OK) {
					;
				}

				/* H/W 구성정보 초기화 요청
				*/
				else if (rpkt2->msgtype == PRMT_HW_EQUIP_REQ) {

					/* send signal => confirm (packet2) 
					*/
					send_primitive (PRMT_HW_EQUIP_CONFIRM, (int *)&r_sfd);

					/* send data => configuration (packet3 : data1, 2, ... end) 
					*/
					send_primitive (PRMT_HW_EQUIP_DATA, (int *)&r_sfd);
				}

				/* H/W 구성정보 초기화 수신 응답
				*/
				else if (rpkt2->msgtype == PRMT_HW_EQUIP_RCV_OK) {
					;
				}

				/* S/W 구성정보 초기화 요청
				*/
				else if (rpkt2->msgtype == PRMT_SW_EQUIP_REQ) {

					/* send signal => confirm (packet2) 
					*/
					send_primitive (PRMT_SW_EQUIP_CONFIRM, (int *)&r_sfd);

					/* send data => configuration (packet3 : data1, 2, ... end) 
					*/
					send_primitive (PRMT_SW_EQUIP_DATA, (int *)&r_sfd);
				}

				/* S/W 구성정보 초기화 수신 응답
				*/
				else if (rpkt2->msgtype == PRMT_SW_EQUIP_RCV_OK) {
					;
				}

				/* MMC 요청
				*/
				else if (rpkt2->msgtype == PRMT_MMC_REQUEST) {

					/* send signal => confirm (packet2)
					*/
					send_primitive (PRMT_MMC_CONFIRM, (int *)&r_sfd);
				}
			}

			/* packet 3 처리 : current_alm packet ************************************
			*/
			else if (rx_pkt->hdr.chd.pktype == 3) {
				rpkt3 = (Packet3 *)rx_pkt;
			}

		}

		keepalivelib_increase ();

		sleep (1);

		/* 1분 간격 periodic function */
		if (!(l_cnt++% 60)) {

			/* 통계 전송화일 버퍼(메모리) 검색 & expired 전송화일 정보 삭제
			*/
			refresh_flist ();

			/* 1시간 간격 periodic function */
			if (l_cnt >= 3600) {

				/* 저장 주기 만료된 통계 전송 화일 삭제 
				 * - 5분 통계   = 48시간
				 * - 1시간 통계 = 1주일
				 */
				expired_stat_file ();
				l_cnt = 0;
			}
		}
	}

} /* End of main () */
