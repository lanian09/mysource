#include <stdio.h>
#include <netinet/in.h>
#include "radius.h"


int makeRadius_startMsg (PACCT_START pStart, unsigned char *buf, int mode)
{
    unsigned char   ucCode;
    unsigned char	ucLen;
    unsigned char   ucParaLen;
	int 			dVendorID;
    int				dTmpMsgLen;
    int           	dCurPos;
	int				dTmpVal=0;
	long long		llTmpVal=0;
	
    dCurPos= 0;


	/* CODE */
	memcpy(&buf[dCurPos], &pStart->ucCode, RADIUS_CODE_LEN);
	dCurPos += RADIUS_CODE_LEN;

	/* Identifier */
	memcpy(&buf[dCurPos], &pStart->ucID, RADIUS_IDENTIFIER_LEN);
	dCurPos += RADIUS_IDENTIFIER_LEN;

	/* Length */
	dCurPos += RADIUS_LENGTH_LEN;

	/* Authenticator */
	memcpy(&buf[dCurPos], &pStart->szAuthen[0], MAX_AUTH_SIZE);
	dCurPos += MAX_AUTH_SIZE;

	/* USERNAME ****************************************************/
	/* TYPE */
	ucCode = 1;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = strlen(pStart->szUserName);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
#if 0
	{
		int subsID=0;
		char tmpBuf[20];
		subsID = atoi(pStart->szUserName+3);
		subsID++;
		sprintf(tmpBuf, "%d@lgt.co.kr", subsID);
		strncpy(pStart->szUserName+3, tmpBuf, ucParaLen-3);
	}
#endif
	memcpy(&buf[dCurPos], pStart->szUserName, ucParaLen);
	dCurPos += ucParaLen;


	/* Framed IP ***************************************************/
	/* TYPE, offset 45 */
	ucCode = 8;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], (unsigned char *)&pStart->uiFramedIP, ucParaLen);
	dCurPos += ucParaLen;


	/* ACCT-STATUS-TYPE *********************************************/
	/* TYPE */
	ucCode = 40;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStart->dAStatType);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;

	/* NAS-TYPE ***********************************************/
	/* TYPE */
	ucCode = 23; /* wireless UMTS */
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(char);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], &pStart->ucNASType, ucParaLen);
	dCurPos += ucParaLen;


	/* Calling-Station-ID ***********************************/
	/* TYPE, offset 60 */
	ucCode = 31;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = MAX_IMSI_LEN;
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], &pStart->szMIN[0], ucParaLen);
	dCurPos += ucParaLen;


	/* NAS-IP ADDRESS *****************************************/
	/* TYPE */
	ucCode = 4;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], &pStart->uiNASIP, ucParaLen);
	dCurPos += ucParaLen;

#if 0
	/* STOP */
	/* Acct-Session-Time ***************************************/
	/* TYPE */
	ucCode = 46;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = 4;
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAcctSessTime);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;


	/* Acct-Input-Octets ***************************************/
	/* TYPE */
	ucCode = 42;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAcctInOctes);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;

	/* Acct-Output-Octets **************************************/
	/* TYPE */
	ucCode = 43;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAcctOutOctes);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;

#endif

	/* EVENT TIME *********************************************/
	/* TYPE */
	ucCode = 55;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStart->dEventTime);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;


	/* Accounting Session ID **************************************/
	/* TYPE */
	ucCode = 44;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(long long);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	llTmpVal = htonl(pStart->llAcctID);
	memcpy(&buf[dCurPos], &llTmpVal, ucParaLen);
	dCurPos += ucParaLen;

#if 1
	/* CBIT **************************************/
	/* TYPE */
	ucCode = DEF_VENDOR_ATTR;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = strlen(pStart->szCBit);
	ucLen = ucParaLen + RADIUS_VENDOR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* VENDOR-ID */
	dVendorID = htonl(DEF_RADIUS_VENDOR_ID_LGT);
	memcpy(&buf[dCurPos], &dVendorID, RADIUS_VENDORID_LEN);
	dCurPos += RADIUS_VENDORID_LEN;

	/* VENDOR-TYPE */
	ucCode = 1;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_VENDOR_T_LEN);
	dCurPos += RADIUS_VENDOR_T_LEN;

	/* VENDOR-LENGTH */
	ucLen = ucParaLen + RADIUS_VENDORATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_VENDOR_L_LEN);
	dCurPos += RADIUS_VENDOR_L_LEN;

	/* VENDOR-VALUE */
	memcpy(&buf[dCurPos], &pStart->szCBit[0], ucParaLen);
	dCurPos += ucParaLen;

	/* PBIT **************************************/
	/* TYPE */
	ucCode = DEF_VENDOR_ATTR;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = strlen(pStart->szPBit);
	ucLen = ucParaLen + RADIUS_VENDOR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* VENDOR-ID */
	dVendorID = htonl(DEF_RADIUS_VENDOR_ID_LGT);
	memcpy(&buf[dCurPos], &dVendorID, RADIUS_VENDORID_LEN);
	dCurPos += RADIUS_VENDORID_LEN;

	/* VENDOR-TYPE */
	ucCode = 1;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_VENDOR_T_LEN);
	dCurPos += RADIUS_VENDOR_T_LEN;

	/* VENDOR-LENGTH */
	ucLen = ucParaLen + RADIUS_VENDORATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_VENDOR_L_LEN);
	dCurPos += RADIUS_VENDOR_L_LEN;

	/* VENDOR-VALUE */
	memcpy(&buf[dCurPos], &pStart->szPBit[0], ucParaLen);
	dCurPos += ucParaLen;

	/* HBIT **************************************/
	/* TYPE */
	ucCode = DEF_VENDOR_ATTR;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = strlen(pStart->szHBit);
	ucLen = ucParaLen + RADIUS_VENDOR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* VENDOR-ID */
	dVendorID = htonl(DEF_RADIUS_VENDOR_ID_LGT);
	memcpy(&buf[dCurPos], &dVendorID, RADIUS_VENDORID_LEN);
	dCurPos += RADIUS_VENDORID_LEN;

	/* VENDOR-TYPE */
	ucCode = 1;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_VENDOR_T_LEN);
	dCurPos += RADIUS_VENDOR_T_LEN;

	/* VENDOR-LENGTH */
	ucLen = ucParaLen + RADIUS_VENDORATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_VENDOR_L_LEN);
	dCurPos += RADIUS_VENDOR_L_LEN;

	/* VENDOR-VALUE */
	memcpy(&buf[dCurPos], &pStart->szHBit[0], ucParaLen);
	dCurPos += ucParaLen;
#endif

    dTmpMsgLen = htons(dCurPos);
    memcpy(&buf[RADIUS_LENGTH_POS], &dTmpMsgLen, RADIUS_LENGTH_LEN);

    return dCurPos;
}


int makeRadius_stopMsg (PACCT_STOP pStop, unsigned char *buf, int mode)
{
    unsigned char   ucCode;
    unsigned char	ucLen;
    unsigned char   ucParaLen;
	int 			dVendorID;
    int				dTmpMsgLen;
    int           	dCurPos;
	int				dTmpVal=0;
	long long		llTmpVal=0;
	
    dCurPos= 0;


	/* CODE */
	memcpy(&buf[dCurPos], &pStop->ucCode, RADIUS_CODE_LEN);
	dCurPos += RADIUS_CODE_LEN;

	/* Identifier */
	memcpy(&buf[dCurPos], &pStop->ucID, RADIUS_IDENTIFIER_LEN);
	dCurPos += RADIUS_IDENTIFIER_LEN;

	/* Length */
	dCurPos += RADIUS_LENGTH_LEN;

	/* Authenticator */
	memcpy(&buf[dCurPos], &pStop->szAuthen[0], MAX_AUTH_SIZE);
	dCurPos += MAX_AUTH_SIZE;

	/* USERNAME ****************************************************/
	/* TYPE */
	ucCode = 1;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = strlen(pStop->szUserName);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], pStop->szUserName, ucParaLen);
	dCurPos += ucParaLen;


	/* Framed IP ***************************************************/
	/* TYPE, offset 45 */
	ucCode = 8;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], (unsigned char *)&pStop->uiFramedIP, ucParaLen);
	dCurPos += ucParaLen;


	/* ACCT-STATUS-TYPE *********************************************/
	/* TYPE */
	ucCode = 40;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAStatType);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;


	/* Calling-Station-ID ***********************************/
	/* TYPE, offset 60 */
	ucCode = 31;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = MAX_IMSI_LEN;
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	memcpy(&buf[dCurPos], &pStop->szMIN[0], ucParaLen);
	dCurPos += ucParaLen;

#if 0
	/* STOP */
	/* Acct-Session-Time ***************************************/
	/* TYPE */
	ucCode = 46;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = 4;
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAcctSessTime);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;


	/* Acct-Input-Octets ***************************************/
	/* TYPE */
	ucCode = 42;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAcctInOctes);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;

	/* Acct-Output-Octets **************************************/
	/* TYPE */
	ucCode = 43;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(int);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	dTmpVal = htonl(pStop->dAcctOutOctes);
	memcpy(&buf[dCurPos], &dTmpVal, ucParaLen);
	dCurPos += ucParaLen;

#endif

	/* Accounting Session ID **************************************/
	/* TYPE */
	ucCode = 44;
	memcpy(&buf[dCurPos], &ucCode, RADIUS_ATTR_T_LEN);
	dCurPos += RADIUS_ATTR_T_LEN;

	/* LENGTH */
	ucParaLen = sizeof(long long);
	ucLen = ucParaLen + RADIUS_ATTR_OFFSET;
	memcpy(&buf[dCurPos], &ucLen, RADIUS_ATTR_L_LEN);
	dCurPos += RADIUS_ATTR_L_LEN;

	/* DATA */
	llTmpVal = htonl(pStop->llAcctID);
	memcpy(&buf[dCurPos], &llTmpVal, ucParaLen);
	dCurPos += ucParaLen;


    dTmpMsgLen = htons(dCurPos);
    memcpy(&buf[RADIUS_LENGTH_POS], &dTmpMsgLen, RADIUS_LENGTH_LEN);

    return dCurPos;
}

