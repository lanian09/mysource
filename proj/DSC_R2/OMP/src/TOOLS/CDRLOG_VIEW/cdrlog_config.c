
#include <stdio.h>
#include <errno.h>
#include <cdrlog.h>

#define  CONFIG_FILE    "./SERVICE_TYPE.conf"

st_svc_register		svc_register[1000];
int					gIndex;

void print_config(void)
{
	int    i;

	fprintf(stdout, "\n\n========================================\n");
	fprintf(stdout, "         Registered IP & Port           \n");
	fprintf(stdout, "========================================\n");
	for (i=0; i<gIndex; i++)
	{
		fprintf(stdout, "IP : %s, PORT : %d\n", 
				svc_register[i].dest_ip_addr,
				svc_register[i].dest_port);
	}
	fprintf(stdout, "========================================\n\n");

}


char *
TreamNullData(char *sParseStr)
{
    int i = 0;

    for(i = 0; i<strlen(sParseStr); i++)
    {
        if( sParseStr[i] == ' ' || sParseStr[i] == '\t' )
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return (sParseStr+i);

}


/**
 *	get first token data input data buffer
 **/
void
Get1Data(char *sParseStr, char *getData)
{

    int     i = 0;

    for(i = 0; i<strlen(sParseStr); i++)
    {
        if( sParseStr[i] == ' ' || sParseStr[i] == '\t'
            || sParseStr[i] == '\0' || sParseStr[i] == '\n')
        {
            break;
        }
        else
        {
            getData[i] = sParseStr[i];
        }
    }

    getData[i] = '\0';

    return;
}


int
GetConfData(char *sParseStr, unsigned char ppstData[16][16], int *iCount)
{

    int i = 0;
    char *tmpStr = NULL;
    char getData[64];

    tmpStr = sParseStr;

    for(i = 0; i<32; i++)
    {
        memset(getData, 0x00, 64);
        tmpStr = TreamNullData(tmpStr);
        Get1Data(tmpStr, getData);
        if(getData[0] == '\0')
        {
            break;
        }
        snprintf(ppstData[i], 64, "%s", getData);
        tmpStr += strlen(ppstData[i]);

        *iCount = i;
     }


    return (1);

}


int
load_config_data()
{

    unsigned char   getData[16][16];
    unsigned char   szTempBuff[256];
    int             dCount;
	char			*check_cdr;     
    FILE            *pFd;
	int				count;

    /** variables intialize  **/
    dCount = 0;
    pFd = NULL;

    pFd = fopen(CONFIG_FILE, "r");
    if (!pFd)
    {
        fprintf(stdout, "FILE OPEN ERROR [NAME=%s, ERRNO=%d]", CONFIG_FILE, strerror(errno));
        return (-1);
    }

	gIndex = 0;
	memset(svc_register, 0x00, sizeof(svc_register));
	count = 0;

    while(fgets(szTempBuff, sizeof(szTempBuff), pFd) != NULL)
    {

		/** comment line **/
		if (szTempBuff[0] == '#')
		{
			continue;
     	}

		/** CDR involve line **/
		if (!strstr(szTempBuff, "CDR") && !strstr(szTempBuff, "cdr"))
		{
			count++;
			//printf("none CDR.... : %d\n", count);
			continue;
		}

        memset(getData, 0x00, sizeof(getData)); 
        GetConfData(szTempBuff, getData, &dCount);

        if (!strcmp(getData[0], "@START"))
        {
            continue;
        }
        else if(!strcmp(getData[0], "@END"))
        {
            break;
        }

	
        memcpy(svc_register[gIndex].dest_ip_addr, getData[3], strlen(getData[3]));

		if (!strcmp(getData[5], "ALL"))
		{
			svc_register[gIndex++].dest_port = -1;
		}
			
		else
		{
        	svc_register[gIndex++].dest_port = atoi(getData[5]);
		}

    }

#ifdef _CONFIG_PRINT
	print_config();
#endif
    fclose(pFd);

}


int
check_register(char *ip_addr, unsigned short port)
{

	int 	i;


	for (i=0; i<gIndex; i++)
	{
		if (!strcmp(ip_addr, svc_register[i].dest_ip_addr))
		{
			if (svc_register[i].dest_port == 0 || svc_register[i].dest_port == -1)
			{
				return (-1);
			}
			else
			{
				if (port == svc_register[i].dest_port)
					return (-1);
				else 
					continue;
			}
		}
		else
			continue;

	}
	return (1);

}
