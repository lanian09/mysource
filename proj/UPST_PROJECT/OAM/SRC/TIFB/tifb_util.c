/**A.1*  File Inclusion *******************************************************/

/* SYS HEADER */
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <syscall.h>
#include <sys/procfs.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
/* LIB HEADER */
#include "commdef.h"
#include "mems.h"
#include "cifo.h"
#include "clisto.h"
#include "gifo.h"
#include "loglib.h"
/* PRO HEADER */
#include "msgdef.h"
#include "path.h"
#include "procid.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
/* LOC HEADER */
#include "tifb_util.h"

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/

/**C.1*  Declaration of Variables  ********************************************/
stMEMSINFO	*gpRECVMEMS;
stCIFO	  	*gpCIFO;

long    starttime;
char    mtime_str[81];
char    STR_TSW_COM[MAX_SW_COUNT][30];

/**D.1*  Declare of Functions  ************************************************/
void makelower(char *str);
void makeupper(char *str);

/**D.2*  Definition of Functions  *********************************************/
int init_proc()
{
	gpRECVMEMS = nifo_init_zone((unsigned char *)"TIFB", SEQ_PROC_TIFB, FILE_NIFO_ZONE);
	if( gpRECVMEMS == NULL ){
		fprintf(stderr, "F=%s:%s.%d: FAILED IN nifo_init_zone, NULL\n",  __FILE__, __FUNCTION__, __LINE__);
		exit(0);
	}

	gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( gpCIFO == NULL ){
		fprintf(stderr, "F=%s:%s.%d: FAILED IN gifo_init_group. cifo=%s, gifo=%s\n",
				__FILE__, __FUNCTION__, __LINE__, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(0);
	}
	return 0;
}

/*******************************************************************************
 READ MC_INIT
*******************************************************************************/
int dGetBlocks(char *fn, char (*p)[30])
{
	int     ln, rdcnt, scan_cnt;
	char    buf[BUF_SIZE], Bname[PROC_NAME_LEN];
	FILE    *fp;

	if( (fp = fopen(fn, "r")) == NULL)
		return -1;  /* fopen error */

	ln		= 0;
	rdcnt	= 0;
	while(fgets(buf, BUF_SIZE, fp) != NULL)
	{
		ln++;
		/*
		* from Source to Target : sscanf
		*/
		if(buf[0] != '#')
		{
			printf("SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n",fn, ln);
			return -1;
		}
		else if(buf[1] == '#')
			continue;
		else if(buf[1] == 'E')
			break;
		else if(buf[1] == '@')
		{
			if( (scan_cnt= sscanf(&buf[2], "%s %*s", Bname)) != 1)
				sprintf(Bname, " - ");

			sprintf(*(p+rdcnt), "%s", Bname);
			rdcnt++;
		}
		else
		{
			printf("SYNTAX ERROR FILE:%s, LINK:%d\n",fn, ln);
			return -2;
		}
	}/* while */
	fclose(fp);

	return rdcnt;
}

/*******************************************************************************
 STRING CONVERT TO LOW CASE
*******************************************************************************/
void makelower(char *str)
{
	while(*str)
	{
		*str = tolower(*str);
		str++;
	}
}

/*******************************************************************************
 STRING CONVERT TO UPPER CASE
*******************************************************************************/
void makeupper(char *str)
{
	while (*str)
	{
		*str = toupper(*str);
		str++;
	}
}


/*******************************************************************************
 CHECK REGISTERED BLOCK USING BLOCK NAME
*******************************************************************************/
int is_registered_block(char *comm)
{
	int i;

    for( i = 0;i < MAX_SW_COUNT;i++)
	{
        if(!strcmp(comm, STR_TSW_COM[i]))
		{
			return i;
        }
	}

	return -1;
}


/*******************************************************************************
 TIME CONVERT TO STRING
*******************************************************************************/
char *mtime()
{
    time_t t;
	struct tm *tmptm;

    t = time(&t);
	tmptm = localtime(&t);

	memset( mtime_str, 0x00, 81 );

    strftime(mtime_str, 80, "%Y-%m-%d %T %a", (const struct tm*)tmptm );
	mtime_str[21] = toupper( mtime_str[21] );
	mtime_str[22] = toupper( mtime_str[22] );
    mtime_str[23] = 0x00;

    return mtime_str;
}


/*******************************************************************************
 GET PROCESS ID USING PROCESS NAME
*******************************************************************************/
int GetProcessID(char *name)
{
    int         fd;
    int         dPCnt = 0;
    FILE        *fp_tot;
    FILE        *fp_pro;
    DIR         *dirp;
    struct dirent *direntp;
    char        pname[PROC_NAME_LEN];
    char        tempbuf[BUF_SIZE];

    char        szBuffer[20480];
    char        *szTmp;
    int         dReadSize;
    int         dTotVal;
    long        lProcVal;
	unsigned long ulProcVal;

    memset(szBuffer, 0x00, 20480 );


    if( (fp_tot = fopen( "/proc/stat", "r")) == NULL)
    {
        fprintf(stderr, "CANNOT OPEN STAT FILE\n");
        return -1;
    }

    dReadSize = fread( szBuffer, 20480, 1, fp_tot);

    szTmp = strstr(szBuffer, "btime");

    sscanf( szTmp, "%*s %d", &dTotVal );

    fclose(fp_tot);


    if((dirp = opendir(PROC_PATH)) == (DIR *)NULL)
    {
        fprintf(stderr, "\n\tCAN'T ACCESS PROCESS DIRECTORY (%s)\n", PROC_PATH);
        exit(0);
    }
    while((direntp = readdir(dirp)) != NULL)
    {
        dPCnt++;
        if( dPCnt > 1024 )
            break;

        if(!strcmp(direntp->d_name, PARENT_PATH) ||
           !strcmp(direntp->d_name, HOME_PATH)) continue;

        if( !atoi(direntp->d_name) )
        {
            continue;
        }

        sprintf(tempbuf, "%s/%s/cmdline", PROC_PATH, direntp->d_name);


        fd = open(tempbuf, O_RDONLY);

        if(fd < 0)
        {
            close(fd);
            continue;
        }

        memset( pname, 0x00, PROC_NAME_LEN);
        if( read(fd, pname, PROC_NAME_LEN-1) < 0 )
        {
            close(fd);
            continue;
        }
        else
        {
            close(fd);
        }

        pname[PROC_NAME_LEN-1] = 0x00;

        if( !strcmp(name, pname) )
        {
            sprintf(tempbuf, "%s/%s/stat", PROC_PATH, direntp->d_name);

            fp_pro = fopen( tempbuf, "r");

            dReadSize = fread( szBuffer, 20480, 1, fp_pro);

/*
            sscanf(szBuffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %lu %*lu %*ld %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*d %*d", &ulProcVal );
*/
            sscanf(szBuffer, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %lu %*u %*d %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*d %*d", &ulProcVal );

            fclose(fp_pro);

            lProcVal = (long)ulProcVal/100;

            starttime = dTotVal + lProcVal;

            //starttime = time(&starttime);
            return atoi(direntp->d_name);
        }
    }

    closedir(dirp);

	return -2;
}

int dGetUserPermission(void)
{
	FILE	*ptr;
	int		dRet = 0;
	char	szCommand[8], szBuffer[16];

	sprintf(szCommand, "whoami");
	if( (ptr = popen(szCommand, "r")) != NULL)
	{
		while(fgets(szBuffer, 16, ptr) != NULL)
		{
			szBuffer[strlen(szBuffer)-1] = 0x00;
			if(strcasecmp(szBuffer, "root") == 0)
			{
				dRet = 1;
				break;
			}
		}
	}
	pclose( ptr );

	return dRet;
}

/*******************************************************************************
  GET YES OR NO
 *******************************************************************************/
int GetYorN()
{
	char ch;

	do
	{
		ch = getchar();
	}
	while(ch != 'Y' && ch != 'y' && ch != 'N' && ch != 'n');

	if(ch == 'Y' || ch == 'y') return _YES;
	else return _NO;
}

/*******************************************************************************
 PrintOut:
*******************************************************************************/
void PrintOut(int flag, char* buf)
{
	char pbuf[MAX_MSGBUF_LEN];

	sprintf(pbuf, "\n%s\n", mtime());

	strcat(pbuf, "M1061  FINALIZATION OF MAIN COMPUTER PROCESSES\n");
	if(flag == TIFB_FAIL)
		strcat(pbuf, "  RESULT = FAIL\n  REASON = ");
	else
		strcat(pbuf, "  RESULT = SUCCESS\n  ");
	strcat(pbuf, buf);
	strcat(pbuf, "COMPLETED\n");

	printf("%s\n", pbuf);

	return;

} /* PrintOut */

int dGetNode(unsigned char **ppNODE, pst_MsgQ *ppstMsgQ)
{
	*ppNODE = NULL;
	*ppstMsgQ = NULL;

	*ppNODE = nifo_node_alloc(gpRECVMEMS);
	if( *ppNODE == NULL ){
		fprintf(stderr, "%s.%d:%s FAILED IN nifo_node_alloc, errno=%d:%s\n",
				__FILE__,__LINE__,__FUNCTION__,errno,strerror(errno));
		return -1;
	}

	*ppstMsgQ = (pst_MsgQ)nifo_tlv_alloc(gpRECVMEMS, *ppNODE, DEF_MSGQ_NUM, DEF_MSGQ_SIZE, DEF_MEMSET_OFF);
	if( *ppstMsgQ == NULL ){
		fprintf(stderr, "%s.%d:%s FAILED IN nifo_tlv_alloc, return NULL\n",__FILE__,__LINE__,__FUNCTION__);
		nifo_node_delete(gpRECVMEMS, *ppNODE);
		return -2;
	}

	return 0;
}

int dMsgrcv(pst_MsgQ *ppstMsg)
{
	OFFSET offset;

	if( (offset = gifo_read(gpRECVMEMS, gpCIFO, SEQ_PROC_TIFB)) <= 0 ){
		usleep(0);
		return -1;
	}

	*ppstMsg = (pst_MsgQ)nifo_get_value(gpRECVMEMS, DEF_MSGQ_NUM, offset);
	if( *ppstMsg == NULL ){
		fprintf(stderr, "%s.%d:%s FAILED IN nifo_get_value(st_MsgQ=%d), offset=%ld\n", __FILE__,__LINE__,__FUNCTION__, DEF_MSGQ_NUM, offset);
		return -2;
	}
	return 0;
}

int dMsgsnd(int procID, U8 *pNODE)
{

	OFFSET offset;

	offset = nifo_offset(gpRECVMEMS, pNODE);
	
	if(gifo_write(gpRECVMEMS, gpCIFO, SEQ_PROC_TIFB, procID, offset) < 0) {
        fprintf(stderr, "%s.%d:%s FAILED IN gifo_write(TIFB:%d > TARGET:%d), offset=%ld, errno=%d:%s\n",
                __FILE__,__LINE__,__FUNCTION__, SEQ_PROC_TIFB, procID, offset, errno, strerror(errno));
		nifo_node_delete(gpRECVMEMS, pNODE);
        usleep(0);
        return -1;
    }
    return 0;
}

int dGetBlockBIN(char *sBlockName, char *sBinName, int dBinLength)
{
    int     dLineNum, dRdCnt, dScanCnt;
    char    sBuf[BUF_LEN], sBname[PROC_NAME_LEN], sBinPath[BUFSIZ];
    size_t  szBinStrLen;
    FILE    *fp;

    if( (fp = fopen(FILE_MC_INIT, "r")) == NULL)
        return -1;

    dLineNum    = 0;
    dRdCnt      = 0;
    while(fgets(sBuf, BUF_LEN, fp) != NULL)
    {
        dLineNum++;
        /*  from Source to Target : sscanf  */
        if(sBuf[0] != '#')
        {
            fclose(fp);
            return -2;
        }
        else if(sBuf[1] == '#')
            continue;
        else if(sBuf[1] == 'E')
            break;
        else if(sBuf[1] == '@')
        {
            if( (dScanCnt = sscanf(&sBuf[2], "%s %s", sBname, sBinPath)) != 2)
                fprintf(stderr, "ERROR IN sscanf[%s] sBname[%s] sBinPath[%s]\n", &sBuf[2], sBname, sBinPath);
            else
            {
                if(strcmp(sBname, sBlockName) == 0)
                {
                    szBinStrLen = strlen(sBinPath);
                    if(dBinLength > szBinStrLen)
                    {
                        strncpy(sBinName, sBinPath, dBinLength);
                        break;
                    }
                    else
                    {
                        fclose(fp);
                        return -3;
                    }
                }
            }
            dRdCnt++;
        }
        else
        {
            fclose(fp);
            return -4;
        }
    }
    fclose(fp);

    return 0;
}
