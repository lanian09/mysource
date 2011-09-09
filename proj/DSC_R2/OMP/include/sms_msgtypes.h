#ifndef __SMS_MSGTYPES_H__
#define __SMS_MSGTYPES_H__

/* 
 * KTF ���ɸ� SMS ������ �˶� ����
 * alarm�� report�ϱ� ���� mtype�� MTYPE_SMS_ALARM_REPORT ���
 * 1. �˶� �߻�/������ �ʿ��� ��� : almTime�� SMS_MANUAL_CLEAR_ALARM(0) ����
 * 2. �˶� �߻� �� �ڵ� �����Ǵ� ��� : almTime�� ��û �溸�� ������ �ð�(��) ����
 * almInfo�� �� alarm�� ������ �� �ִ� ������ Text�� MP���� ���ؼ� ��������
 * ==> OMP�� ������ current alarm list���� sysname, appname, alminfo�� ����Ͽ� ������
 */
#define SMS_MANUAL_CLEAR_ALARM  	0

/* event�� �˶��� ���� �Ⱓ */
#define SMS_DEFAULT_ALARM_PERIOD  	5		/* �������� default �˶� �Ⱓ */
#define SMS_MIN_ALARM_PERIOD  		0		/* event �˶��� �ּ� �溸 �Ⱓ (0�� �˶��� �ȿ︲) */
#define SMS_MAX_ALARM_PERIOD  		3600	/* event �˶��� �ִ� �溸 �Ⱓ */

typedef struct {
    int  almTime;           /* �˶��� �︮�� �ð� , 0 �̸� �ݵ�� �����ϰ�, > 0 �̸� �ش� �Ⱓ(��) �Ŀ� �ڵ� ���� */
    int  almLevel;          /* �˶� ���, normal(0), minor(1), major(2), critical(3) */
    char almSysName[32];    /* �˶� �߻� �ý��� */
    char almProcName[32];   /* �˶� �߻� ���μ��� */
    char almInfo[32];       /* �˶� �߻� ���� ����(text) */
    char almMsg[512];       /* �˶� �� ����, console�� �ѷ����� �޽���  */
} SMS_AlmMsgType;

#endif /*__SMS_MSGTYPES_H__*/

