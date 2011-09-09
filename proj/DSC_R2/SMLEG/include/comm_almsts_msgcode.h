#ifndef __COMM_ALMSTS_MSGCODE_H__LMCODE_SFM
#define __COMM_ALMSTS_MSGCODE_H__

#pragma pack(1)

//------------------------------------------------------------------------------
// ��� �޽��� �ڵ� ���� �����Ѵ�.
//------------------------------------------------------------------------------

/* common alm */
#define ALMCODE_SFM_CPU_USAGE						1000 // CPU ���� �˶�
#define ALMCODE_SFM_MEMORY_USAGE					1001 // �޸� ���� �˶�
#define ALMCODE_SFM_DISK_USAGE						1002 // ��ũ ���� �˶�
#define ALMCODE_SFM_LAN								1003 // LAN connection ���� �˶�
#define ALMCODE_SFM_PROCESS							1004 // ���μ��� ���� �˶�


/* spec alm */
#define ALMCODE_SFM_LINK							1005 // LINK connection ���� �˶�
#define ALMCODE_SFM_DB_REP    						1006// DB Replication state
#define ALMCODE_SFM_DB_REP_GAP						1007 // DB Replication Gap state
#define ALMCODE_SFM_BSD								1009// hardware �� �˶� �߻���
#define ALMCODE_SFM_SERVER_CONN						1010// hardware �� �˶� �߻���
#define ALMCODE_SFM_CALL_INFO						1011// hardware �� �˶� �߻���

/* by helca */
/* duplication alm*/
#define ALMCODE_SFM_DUPLICATION						1012 // Duplication ���� �˶�
#define ALMCODE_SFM_HEARTBEAT						1013 // HEARTBEAT ���� �˶�
#define ALMCODE_SFM_OOS								1014 // OOS ���� �˶�
#define ALMCODE_SFM_SUCCESS_RATE					1015 // SuccessRate ���� �˶�
#define ALMCODE_SFM_SESS_LOAD						1016 // Session�� ���� Load���� �˶�
#define ALMCODE_SFM_RMT_DB_STS						1017 // Remote DB ���� �˶�
#define ALMCODE_SFM_RMT_LAN							1018 // Remote LAN Connection ���� �˶�
#define ALMCODE_SFM_OPT_LAN							1019 // Optical LAN Connection ���� �˶�
#define ALMCODE_SFM_HWNTP							1020 // hwNTP ���� �˶�

/* probe device */
#define ALMCODE_SFM_PD_CPU_USAGE					1021
#define ALMCODE_SFM_PD_MEMORY_USAGE					1022
#define ALMCODE_SFM_PD_FAN_USAGE					1023
#define ALMCODE_SFM_PD_GIGA_USAGE					1024
#define ALMCODE_SFM_PD_RSRC_LOAD					1025

// HPUX_HW
#define ALMCODE_SFM_HPUX_HW							1026

// by helca 08.07
#define ALMCODE_SFM_QUEUE_LOAD						1027 // Queue load ���� �˶� 

/* SYSTEM ACTIVE/STANBY alm */
// �� �ý����� standby/active ���°� �������� �˶�
#define ALMCODE_SFM_ACTIVE							1100

/* NMS */
#define ALMCODE_NMS_CONNECT							3500

//------------------------------------------------------------------------------
// ���� �޽��� �ڵ� ���� �����Ѵ�.
//------------------------------------------------------------------------------

#define STSCODE_SFM_SYS_LEVEL_CHANGE				2000 // �ý��� ��ü ��� ����� ����� ����� �˶�

//process �� keepalive ���� ���Ҷ��� �˶�
#define	STSCODE_SFM_WATCHDOG_REPORT					2001

// hardware ���� �˶� ������
#define STSCODE_SFM_HPUX_HW							2002 

//1. altibase log report �޼����� �ѷ��ٶ�
//2. DB Replication �� clear �Ǿ�����
#define STSCODE_SFM_PROCESS							2003

// Queue cnt�� 200,000 �̻��� �׿� clear��
#define STSCODE_SFM_QUEUE_CLEAR						2004

//------------------------------------------------------------------------------
// OMP�� ��� �޽��� �ڵ� ���� �����Ѵ�.
//------------------------------------------------------------------------------

// �ֱ��� ��� �޽����� ���� �޽��� �ڵ�� comm_msgtypes.h�� ���ǵǾ� �ִ�
// �޽��� ID�� 2000�� ���ϸ� �ȴ�.
#define STSCODE_STM_PERIODIC_FAULT_MINUTE           3000 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_FAULT_HOUR             3001
#define STSCODE_STM_PERIODIC_FAULT_DAY              3002
#define STSCODE_STM_PERIODIC_FAULT_WEEK             3003
#define STSCODE_STM_PERIODIC_FAULT_MONTH            3004
#define STSCODE_STM_SCHEDULE_FAULT                  3005

#define STSCODE_STM_PERIODIC_LOAD_MINUTE            3010 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_LOAD_HOUR              3011
#define STSCODE_STM_PERIODIC_LOAD_DAY               3012
#define STSCODE_STM_PERIODIC_LOAD_WEEK              3013
#define STSCODE_STM_PERIODIC_LOAD_MONTH             3014
#define STSCODE_STM_SCHEDULE_LOAD                   3015

#define STSCODE_STM_PERIODIC_SCIB_MINUTE            3020 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_SCIB_HOUR              3021
#define STSCODE_STM_PERIODIC_SCIB_DAY               3022
#define STSCODE_STM_PERIODIC_SCIB_WEEK              3023
#define STSCODE_STM_PERIODIC_SCIB_MONTH             3024
#define STSCODE_STM_SCHEDULE_SCIB                   3025

#define STSCODE_STM_PERIODIC_SCIB_MINUTE            3020 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_SCIB_HOUR              3021
#define STSCODE_STM_PERIODIC_SCIB_DAY               3022
#define STSCODE_STM_PERIODIC_SCIB_WEEK              3023
#define STSCODE_STM_PERIODIC_SCIB_MONTH             3024
#define STSCODE_STM_SCHEDULE_SCIB                   3025

#define STSCODE_STM_PERIODIC_RCIF_MINUTE            3030 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_RCIF_HOUR              3031
#define STSCODE_STM_PERIODIC_RCIF_DAY               3032
#define STSCODE_STM_PERIODIC_RCIF_WEEK              3033
#define STSCODE_STM_PERIODIC_RCIF_MONTH             3034
#define STSCODE_STM_SCHEDULE_RCIF                   3035

#define STSCODE_STM_PERIODIC_SCPIF_MINUTE            3040 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_SCPIF_HOUR              3041
#define STSCODE_STM_PERIODIC_SCPIF_DAY               3042
#define STSCODE_STM_PERIODIC_SCPIF_WEEK              3043
#define STSCODE_STM_PERIODIC_SCPIF_MONTH             3044
#define STSCODE_STM_SCHEDULE_SCPIF                   3045

#define STSCODE_STM_PERIODIC_WISE_MINUTE            3050 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_WISE_HOUR              3051
#define STSCODE_STM_PERIODIC_WISE_DAY               3052
#define STSCODE_STM_PERIODIC_WISE_WEEK              3053
#define STSCODE_STM_PERIODIC_WISE_MONTH             3054
#define STSCODE_STM_SCHEDULE_WISE                   3055

#define STSCODE_STM_PERIODIC_DB_MINUTE            	3060 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_DB_HOUR              	3061
#define STSCODE_STM_PERIODIC_DB_DAY               	3062
#define STSCODE_STM_PERIODIC_DB_WEEK              	3063
#define STSCODE_STM_PERIODIC_DB_MONTH             	3064
#define STSCODE_STM_SCHEDULE_DB                   	3065

#define STSCODE_STM_PERIODIC_OB_MINUTE            	3070 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_OB_HOUR              	3071
#define STSCODE_STM_PERIODIC_OB_DAY               	3072
#define STSCODE_STM_PERIODIC_OB_WEEK              	3073
#define STSCODE_STM_PERIODIC_OB_MONTH             	3074
#define STSCODE_STM_SCHEDULE_OB                   	3075

#define STSCODE_STM_PERIODIC_OBC_MINUTE           	3080 //�Ⱦ��µ�
#define STSCODE_STM_PERIODIC_OBC_HOUR             	3081
#define STSCODE_STM_PERIODIC_OBC_DAY              	3082
#define STSCODE_STM_PERIODIC_OBC_WEEK             	3083
#define STSCODE_STM_PERIODIC_OBC_MONTH            	3084
#define STSCODE_STM_SCHEDULE_OBC                  	3085

#define STSCODE_STM_PERIODIC_IPAF_MINUTE     		3790
#define STSCODE_STM_PERIODIC_IPAF_HOUR    			3791
#define STSCODE_STM_PERIODIC_IPAF_DAY     			3792
#define STSCODE_STM_PERIODIC_IPAF_WEEK    			3793
#define STSCODE_STM_PERIODIC_IPAF_MONTH      		3794
#define STSCODE_STM_SCHEDULE_IPAF         			3795

#define STSCODE_STM_PERIODIC_AAA_MINUTE     		3800
#define STSCODE_STM_PERIODIC_AAA_HOUR    			3801
#define STSCODE_STM_PERIODIC_AAA_DAY     			3802
#define STSCODE_STM_PERIODIC_AAA_WEEK    			3803
#define STSCODE_STM_PERIODIC_AAA_MONTH      		3804
#define STSCODE_STM_SCHEDULE_AAA        		 	3805

#if 0 
#define STSCODE_STM_PERIODIC_MEAN_MINUTE    3810 
#define STSCODE_STM_PERIODIC_MEAN_HOUR    	3811
#define STSCODE_STM_PERIODIC_MEAN_DAY     	3812
#define STSCODE_STM_PERIODIC_MEAN_WEEK    	3813
#define STSCODE_STM_PERIODIC_MEAN_MONTH     3814
#define STSCODE_STM_SCHEDULE_MEAN         	3815
#endif

#define STSCODE_STM_PERIODIC_UAWAP_MINUTE			3820 
#define STSCODE_STM_PERIODIC_UAWAP_HOUR				3821
#define STSCODE_STM_PERIODIC_UAWAP_DAY				3822
#define STSCODE_STM_PERIODIC_UAWAP_WEEK				3823
#define STSCODE_STM_PERIODIC_UAWAP_MONTH			3824
#define STSCODE_STM_SCHEDULE_UAWAP					3825

#define STSCODE_STM_PERIODIC_SVC_TR_MINUTE			3830 
#define STSCODE_STM_PERIODIC_SVC_TR_HOUR			3831
#define STSCODE_STM_PERIODIC_SVC_TR_DAY				3832
#define STSCODE_STM_PERIODIC_SVC_TR_WEEK			3833
#define STSCODE_STM_PERIODIC_SVC_TR_MONTH			3834
#define STSCODE_STM_SCHEDULE_SVC_TR					3835

#define STSCODE_STM_PERIODIC_RADIUS_MINUTE			3840 
#define STSCODE_STM_PERIODIC_RADIUS_HOUR			3841
#define STSCODE_STM_PERIODIC_RADIUS_DAY				3842
#define STSCODE_STM_PERIODIC_RADIUS_WEEK			3843
#define STSCODE_STM_PERIODIC_RADIUS_MONTH			3844
#define STSCODE_STM_SCHEDULE_RADIUS					3845

#define STSCODE_STM_PERIODIC_SVC_TTR_MINUTE			3850 
#define STSCODE_STM_PERIODIC_SVC_TTR_HOUR			3851
#define STSCODE_STM_PERIODIC_SVC_TTR_DAY			3852
#define STSCODE_STM_PERIODIC_SVC_TTR_WEEK			3853
#define STSCODE_STM_PERIODIC_SVC_TTR_MONTH			3854
#define STSCODE_STM_SCHEDULE_SVC_TTR				3855

/* NEW Added For UAWAP TRANSLOG (061219) */
#define STSCODE_SFM_SYS_UAWAP_TRANSLOG_FILE_STS		8000

#pragma pack(0)
#endif
