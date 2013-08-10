#ifndef __COND_DB_H__
#define __COND_DB_H__

void vErrPrint(char* szAlm, char* szMsg, unsigned char TAMID, unsigned char TAFID);
int  dInsert_CONDResult(char* szAlm, char* szMsg, int dLen, unsigned char ucTAMID, unsigned char ucTAFID);

#endif /* __COND_DB_H__ */
