#ifndef __SYSCONF_H__
#define __SYSCONF_H__

//------------------------------------------------------------------------------
// ��� �ý��ۿ��� �������� ���Ǵ� Configuration�� ���õ� ������ �����Ѵ�.
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

#define SYSCONF_MAX_APPL_NUM			32	// �ý��۴� application process �ִ� ����
#define SYSCONF_MAX_ASSO_SYS_NUM		3	// SCMA, SCMA, DSCM(OMP)	������� �ý��� �ִ� ����
#define SYSCONF_MAX_SYS_TYPE_NUM		2	// MP, OMP			�ý��� type �ִ� ����
#define SYSCONF_MAX_SYS_TYPE_MEMBER		2	// MP->SCMA, SCMB	type�� �ý��� �ִ� ����
#define SYSCONF_MAX_GROUP_NUM			2	// MP, OMP 			cluster�� ����ȭ�� �ý��� group �ִ� ����
#define SYSCONF_MAX_GROUP_MEMBER		2	// MP->SCMA, SCM: 	�׷�� �ý��� member �ִ� ����

#define SYSCONF_MAX_DAEMON_NUM          20  // �ý��۴� daemon process �ִ� ����
#define SYSCONF_MAX_APPL_MSGQ_NUM       4   // application�� message Queue �ִ� ����
#define SYSCONF_MAX_AUXILARY_MSGQ_NUM   32  // auxilary message Queue �ִ� ����
// sysconfig ���Ͽ� ��ϵǴ� system_type���� �����Ѵ�.
// - FIMD���� ���⿡ ���ǵ� �̸��� �ν��Ѵ�.
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
