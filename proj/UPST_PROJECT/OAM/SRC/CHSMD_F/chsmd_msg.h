#ifndef __CHSMD_MSG_H__
#define __CHSMD_MSG_H__

#define IDX_CPU			0
#define IDX_MEM			1
#define IDX_QUE			2
#define IDX_NIFO		3
#define IDX_DISKAR		4
#define IDX_LINK		16
#define IDX_FAN			20	/*	6	*/
#define IDX_PWR			28	/*	2	*/
#define IDX_DISK		30	/*	2	*/
#define IDX_PORT		32	/*	8	*/

extern void set_time(unsigned char loctype, unsigned char invtype, unsigned char invno, time_t tWhen );
extern void SetFIDBValue( unsigned char *ucFIDB, unsigned char ucNEW );
extern void Send_AlmMsg( char loctype, char invtype, short invno, char almstatus, char oldalm);
extern int Send_ALMD(void);

#endif /* __CHSMD_MSG_H__ */
