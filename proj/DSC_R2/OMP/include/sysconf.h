#ifndef __SYSCONF_H__
#define __SYSCONF_H__

//------------------------------------------------------------------------------
// ��� �ý��ۿ��� �������� ���Ǵ� Configuration�� ���õ� ������ �����Ѵ�.
//------------------------------------------------------------------------------

#define	MY_SYS_NAME			"MY_SYS_NAME"
#define IV_HOME				"IV_HOME"
#define SYSCONF_FILE			"DATA/sysconfig"
#define REMOTE_SYSCONF_FILE		"DATA/rmt_sysconfig" // by helca
#define MML_COMMANDS_FILE		"DATA/mml_commands"
#define MML_PASSWD_FILE			"DATA/mml_passwd"
#define MML_NMSIB_USER_FILE     	"DATA/mml_nmsib_user"

#define MML_IP_FILE         "DATA/mml_iptbl"    // 2009.07.17 by sjs

#define OMDSYS_USER_NAME        	"OMDSYS"

#define INH_MSG_INFO     		"DATA/InhMsgInfo" 
#define	MY_SYS_ENTRY			"MY_SYS_ENTRY" 

#define HW_CONF_G3			"DATA/HW_CONF_G3"
#define HW_CONF_G5			"DATA/HW_CONF_G5"


#define SYSCONF_MAX_APPL_NUM			32	// �ý��۴� application process �ִ� ����
#define SYSCONF_MAX_ASSO_SYS_NUM		3	// ����!!! ���� �����Ұ�..SCMA, SCMA, DSCM(OMP) ������� �ý��� �ִ� ����
#define SYSCONF_MAX_SYS_TYPE_NUM		2	// MP, OMP			�ý��� type �ִ� ����
#define SYSCONF_MAX_SYS_TYPE_MEMBER		2	// MP->SCMA, SCMB	type�� �ý��� �ִ� ����
#define SYSCONF_MAX_GROUP_NUM			2	// MP, OMP 			cluster�� ����ȭ�� �ý��� group �ִ� ����
#define SYSCONF_MAX_GROUP_MEMBER		2	// MP->SCMA, SCMB 	�׷�� �ý��� member �ִ� ����

#define SYSCONF_MAX_DAEMON_NUM          20  // �ý��۴� daemon process �ִ� ����
#define SYSCONF_MAX_APPL_MSGQ_NUM       4   // application�� message Queue �ִ� ����
#define SYSCONF_MAX_AUXILARY_MSGQ_NUM   32  // auxilary message Queue �ִ� ����
// sysconfig ���Ͽ� ��ϵǴ� system_type���� �����Ѵ�.
// - FIMD���� ���⿡ ���ǵ� �̸��� �ν��Ѵ�.
//
#define SYSCONF_SYSTYPE_OMP			"OMP"
#define SYSCONF_SYSTYPE_BSD			"MP"

#define	SYSCONF_SYSTYPE_MPA			"SCMA"
#define	SYSCONF_SYSTYPE_MPB			"SCMB"

#define	SYSCONF_GROUPTYPE_OMP		0
#define	SYSCONF_GROUPTYPE_MP		1

#define S_SHM_INH_MSG_INFO		0x9203	
#endif //__SYSCONF_H__
