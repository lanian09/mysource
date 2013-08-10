#ifndef __SYSCONF_H__
#define __SYSCONF_H__

//------------------------------------------------------------------------------
// 모든 시스템에서 공통으로 사용되는 Configuration에 관련된 정보를 선언한다.
//------------------------------------------------------------------------------

#define	MY_SYS_NAME				"MY_SYS_NAME"
#define IV_HOME					"IV_HOME"
#define SYSCONF_FILE			"NEW/DATA/sysconfig"
#define REMOTE_SYSCONF_FILE		"NEW/DATA/rmt_sysconfig" // by helca
#define MML_COMMANDS_FILE		"NEW/DATA/mml_commands"
#define MML_PASSWD_FILE			"NEW/DATA/mml_passwd"
#define MML_NMSIB_USER_FILE     "NEW/DATA/mml_nmsib_user"
#define TRACE_INFO_FILE			"NEW/DATA/TRACE_INFO.conf" // by helca 2007.05.22
#define DUIA_FILE				"NEW/DATA/DUIA.conf"  // by helca 2007.05.17

#define SYSCONF_MAX_APPL_NUM			32	// 시스템당 application process 최대 갯수
#define SYSCONF_MAX_ASSO_SYS_NUM		3	// SCMA, SCMA, DSCM(OMP)	관리대상 시스템 최대 갯수
#define SYSCONF_MAX_SYS_TYPE_NUM		2	// MP, OMP			시스템 type 최대 갯수
#define SYSCONF_MAX_SYS_TYPE_MEMBER		2	// MP->SCMA, SCMB	type당 시스템 최대 갯수
#define SYSCONF_MAX_GROUP_NUM			2	// MP, OMP 			cluster등 다중화된 시스템 group 최대 갯수
#define SYSCONF_MAX_GROUP_MEMBER		2	// MP->SCMA, SCM: 	그룹당 시스템 member 최대 갯수

#define SYSCONF_MAX_DAEMON_NUM          20  // 시스템당 daemon process 최대 갯수
#define SYSCONF_MAX_APPL_MSGQ_NUM       4   // application당 message Queue 최대 갯수
#define SYSCONF_MAX_AUXILARY_MSGQ_NUM   32  // auxilary message Queue 최대 갯수
// sysconfig 파일에 등록되는 system_type들을 정의한다.
// - FIMD에서 여기에 정의된 이름만 인식한다.
//
#define SYSCONF_SYSTYPE_OMP			"OMP"
#define SYSCONF_SYSTYPE_BSD			"MP"

//sjjeon
//#define	SYSCONF_SYSTYPE_MPA			"DSCA"
//#define	SYSCONF_SYSTYPE_MPB			"DSCB"
#define	SYSCONF_SYSTYPE_MPA			"SCMA"
#define	SYSCONF_SYSTYPE_MPB			"SCMB"

#define	SYSCONF_GROUPTYPE_OMP		0
#define	SYSCONF_GROUPTYPE_MP		1


#endif //__SYSCONF_H__
