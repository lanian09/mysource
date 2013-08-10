#ifndef _MSRP_H_
#define _MSRP_H_

#define MSRP_ENDFLAG_NOTHING	0
#define MSRP_ENDFLAG_END		1
#define MSRP_ENDFLAG_CONTINUE	2
#define MSRP_ENDFLAG_ABORT		3

extern int msrpheader(char *sp, int slen, unsigned short *usMethod, char *szTID);
extern int msrp(char *sp, int slen, unsigned short *usMethod, char *szContentType, unsigned short *usSuccessReport, unsigned short *usFailureReport, char *szToPath, char *szFromPath, char *szMSGID, unsigned short *usResCode, unsigned short *usEndFlag, char *szTID, char *suri);
extern int msrp_min(char *sp, int slen, char *min, int *vendor);

#endif	/* _MSRP_H_ */
