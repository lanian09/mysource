#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim.h"
#include "radius.h"

char        g_http_header [MAX_HTTP_CACHE][2048];


/* conflib_getNthTokenInFileSection 함수는 하나의 config feild를 가져올 때마다 
 * file 을 open/close 한다. 일단 사용하되 추후 변경해야 할 것이다.
 * HOW: 하나의 config file을 open하여 처음부터 끝까지 일은후 close하도록 . */
int getConfig(char *path, PCONF_INFO pci)
{
	char szData[128];
	struct in_addr	addr;

	/* RADIUS PACKET CONFIGURATION ****************************************************/
	/* get data path */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "DATA_PATH", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pci->stRadOpt.szPath, szData, MAX_PATH_LEN);
	pci->stRadOpt.szPath[MAX_PATH_LEN] = 0;

	/* get dest ip */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "DEST_IP", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pci->stRadOpt.szIP, szData, MAX_IP_LEN);
	pci->stRadOpt.szIP[MAX_IP_LEN] = 0;

	/* get dest port */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "DEST_PORT", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stRadOpt.usPort = (unsigned int)strtol(szData,0,0);

	/* get msg type */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "MSG_TYPE", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stRadOpt.uiMsgType = (unsigned int)strtol(szData,0,0);

	/* get msg send count */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "SEND_COUNT", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stRadOpt.uiLoopCnt = (unsigned int)strtol(szData,0,0);

	/* get send delay count */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "DELAY_COUNT", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stRadOpt.uiDelayCnt = (unsigned int)strtol(szData,0,0);

	/* get send delay time */
	if (conflib_getNthTokenInFileSection(path,"RADIUS", "DELAY_TIME", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stRadOpt.uiDelayTime = (unsigned int)strtol(szData,0,0);

	/* HTTP PACKET CONFIGURATION ****************************************************/
	/* get data path */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "DATA_PATH", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pci->stHttpOpt.szPath, szData, MAX_PATH_LEN);
	pci->stHttpOpt.szPath[MAX_PATH_LEN] = 0;

	/* get dest ip */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "DEST_IP", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pci->stHttpOpt.szIP, szData, MAX_IP_LEN);
	pci->stHttpOpt.szIP[MAX_IP_LEN] = 0;

	/* get dest port */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "DEST_PORT", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stHttpOpt.usPort = (unsigned int)strtol(szData,0,0);

	/* get msg type */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "MSG_TYPE", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stHttpOpt.uiMsgType = (unsigned int)strtol(szData,0,0);

	/* get msg send count */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "SEND_COUNT", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stHttpOpt.uiLoopCnt = (unsigned int)strtol(szData,0,0);

	/* get send delay count */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "DELAY_COUNT", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stHttpOpt.uiDelayCnt = (unsigned int)strtol(szData,0,0);

	/* get send delay time */
	if (conflib_getNthTokenInFileSection(path,"HTTP", "DELAY_TIME", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getConfig] read error :%s", strerror(errno));
		return -1;
	}
	pci->stHttpOpt.uiDelayTime = (unsigned int)strtol(szData,0,0);


	return 0;
}

int getRadiusData(char *path, PACCT_REQ pAcctReq)
{
	char szData[128];
	struct in_addr	inAddr;
	PACCT_START		pAcctStart 	= &pAcctReq->stAcctStart;
	PACCT_STOP		pAcctStop 	= &pAcctReq->stAcctStop;


	/* ACCOUNT START *******************************************************************/
	/* Code */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Code", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->ucCode = (unsigned char)atoi(szData);

	/* ID */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Identifier", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->ucID = (unsigned char)atoi(szData);
	
	/* Length */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Length", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->usLen = (unsigned short)atoi(szData);

	/* Attribute ---------------------------------------------------------------------*/
	/* Acct-Session-Id(44) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Acct-Session-Id", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->llAcctID = strtoll(szData,0,16);
	
	/* Calling-Station-ID(31) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Calling-Station-Id", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pAcctStart->szMIN, szData, MAX_MIN_SIZE);
	pAcctStart->szMIN[MAX_MIN_SIZE] = 0;
	
	/* User-Name(1) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "User-Name", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pAcctStart->szUserName, szData, MAX_USERNAME_SIZE);
	pAcctStart->szUserName[MAX_USERNAME_SIZE] = 0;
	
	/* Framed-IP-Address(8) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Framed-IP-Address", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->uiFramedIP = htonl((unsigned int)inet_addr(szData));

	
	/* Acct-Status-Type(40) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Acct-Status-Type", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->dAStatType = (unsigned int)strtol(szData,0,0);
	
	/* NAS-Port-Type(61) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "NAS-Port-Type", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->ucNASType = (unsigned int)strtol(szData,0,0);
	
	
	/* NAS-IP-Address(4) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "NAS-IP-Address", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->uiNASIP = (unsigned int)inet_addr(szData);


	/* Event-Timestamp(55) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "Event-Timestamp", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStart->dEventTime = (unsigned int)strtol(szData,0,0);

	/* C/P/H Bit (26/1) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "CBit", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] CBit read error :%s", strerror(errno));
		return -1;
	}
	strcpy(pAcctStart->szCBit, szData);

	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "PBit", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] PBit read error :%s", strerror(errno));
		return -1;
	}
	strcpy(pAcctStart->szPBit, szData);

	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_START", "HBit", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] HBit read error :%s", strerror(errno));
		return -1;
	}
	strcpy(pAcctStart->szHBit, szData);


	/////////////////
	/* ACCOUNT STOP *******************************************************************/
	/* Code */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Code", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStop->ucCode = (unsigned char)atoi(szData);

	/* ID */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Identifier", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStop->ucID = (unsigned char)atoi(szData);
	
	/* Length */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Length", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStop->usLen = (unsigned short)atoi(szData);

	/* Attribute ---------------------------------------------------------------------*/
	/* Acct-Session-Id(44) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Acct-Session-Id", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStop->llAcctID = strtoll(szData,0,16);
	
	/* Calling-Station-ID(31) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Calling-Station-Id", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pAcctStop->szMIN, szData, MAX_MIN_SIZE);
	pAcctStop->szMIN[MAX_MIN_SIZE] = 0;
	
	/* User-Name(1) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "User-Name", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	strncpy(pAcctStop->szUserName, szData, MAX_USERNAME_SIZE);
	pAcctStop->szUserName[MAX_USERNAME_SIZE] = 0;
	
	/* Framed-IP-Address(8) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Framed-IP-Address", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStop->uiFramedIP = htonl((unsigned int)inet_addr(szData));

	
	/* Acct-Status-Type(40) */
	if (conflib_getNthTokenInFileSection(path,"ACCOUNT_STOP", "Acct-Status-Type", 1, szData) < 0) {
		dAppLog(LOG_CRI, "[getRadiusData] read error :%s", strerror(errno));
		return -1;
	}
	pAcctStop->dAStatType = (unsigned int)strtol(szData,0,0);

	return 0;
}


void
load_cfg_http (char *config_file)
{   
    FILE            *fp;
    char            buf[1024];
    char            seps[] = " ,\t\n";
    char            *token;
    int             index, index_cnt;

    
    if ( (fp = fopen (config_file, "r")) == NULL)  {
        fprintf(stderr, "configuration File Open Error! path:%s. \n", config_file);
        exit(0);
    }
    
    while ( fgets(buf, 1024, fp) != NULL) {
        
        if (buf[0] != '#') {
            token = strtok (buf, seps);
            
            if (strncmp (token, "RESP_CODE", 9) == 0) {
                token = strtok (NULL, seps); /**< = **/
                token = strtok (NULL, " \t\n"); /** 끝까지 **/
                
                index_cnt = 0;
                
				index = atoi(token);
				if (index > MAX_HTTP_CACHE) {
        			fprintf(stderr, "configuration File! index :%d.\n", index);
        			exit(0);
				}
                memset (g_http_header[index], 0x00, sizeof(g_http_header[index]));
				continue;
#if 0
                switch (atoi(token)) {
                case 200: 
                    index = RESP_CODE_200;
                    memset (g_http_header[index], 0x00, sizeof(g_http_header[index]));
                    continue;
                case 304: 
                    index = RESP_CODE_304;
                    memset (g_http_header[index], 0x00, sizeof(g_http_header[index]));
                    continue;
                }
#endif
            } else if (strncmp (token, "RESP_LINE", 9) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "DATE", 4) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "HOST", 4) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "ACCEPT", 5) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "LAST_MODIFIED", 13) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "CONTENT_TYPE", 12) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "CONTENT_LENGTH", 14) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "IMS_CODE", 8) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
             } else if (strncmp (token, "USER_AGENT", 10) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n");
            } else if (strncmp (token, "CONNECTION", 10) == 0) {
                token = strtok (NULL, seps);    /**< = **/
                token = strtok (NULL, "\n");    /** 끝까지 **/
                strcat (g_http_header[index], token);
                strcat (g_http_header[index], "\r\n\r\n");
            }
        }
    }
#if 0  
    printf ("200 OK :: %s\n", g_http_header[RESP_CODE_200]);
    printf ("304 OK :: %s\n", g_http_header[RESP_CODE_304]);
#endif
}

