#ifndef __CHSMD_LINK_H__
#define __CHSMD_LINK_H__

typedef struct _st_linkdev
{
	char          szDevName[7];
	unsigned char ucFlag;
}st_linkdev, *pst_linkdev;

extern void	Send_CondMess(int, int , char , short , char , char );
extern int  dEthCheck(char *szNetStr);
extern void SetFIDBValue( unsigned char*, unsigned char);
extern int  dReadLinkDevInfo(void);
extern void CheckDirector(int dMyID);
extern void CheckSwitch(int dMyID);
extern int  Link_Check(void);
extern int  dChnl_Chk(void);

#endif /* __CHSMD_LINK_H__ */
