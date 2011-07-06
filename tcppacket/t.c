#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define UINT unsigned int
#define USHORT unsigned short
#define UCHAR unsigned char

typedef struct PackHdr
{
/*
    struct timeval  t_TranStime;    // Trans. Start Time : 8 
    struct timeval  tCapTime;       // Pack Cap Time     : 8 
*/
    time_t transtime;
    int transmtime;
    time_t captime;
    int capmtime;

    UINT                        dwSrcIP;                /* Source IP address    : 4 ****/
    UINT                        dwDestIP;               /* Destination IP addr  : 4 ****/

    USHORT              wSrcPort;               /* Source Port No.      : 2 ****/
    USHORT              wDestPort;              /* Destination Port No. : 2 ****/
    USHORT              wTotalLength;           /**** Pack len          : 2 ****/
    USHORT              wIPHeaderLen;           /**** IP Header Len     : 2 ****/

    UINT                        seq;                    /* TCP Session Info     : 4 ****/
    UINT                        ack;                    /* TCP Session Info     : 4 ****/

    UCHAR               Timelive;               /**** Time To live      : 1 ****/
    UCHAR               nControlType;           /* TCP Control : SYN, ACK, RST, FIN, PSH¡Ë?¡Ë¢ç ¡Ë¡ÍeCN¢®¨¡I : 1 */
    USHORT              wDataLen;               /**** TCP Data Len      : 2 ****/
    UINT                        window;                 /* TCP Session Info     : 4 ****/

    UCHAR               retransFlag;            /* Retransmit Count   : 1 */
    UCHAR               ucDirection;
    UCHAR               reserved[6];            /* Reserved : 7 */
} st_PackHdr_t, *pst_PackHdr_t;

#define MAX_REQRES_HEADER ((256*2)+8)

typedef struct _st_HttpHeader
{       
/*** _st_HDR_Info ***/
/*
    struct timeval tvSynTime;
    struct timeval tvOpTime;
*/
        time_t transtime;
        int transmtime;
        time_t captime;
        int capmtime;
        
    UINT                uSrvIP;
    UINT                uCliIP;

    USHORT          usSrvPort;
    USHORT          usCliPort;
    USHORT          usDataSize;
    USHORT          usHttpSessID;

/*** ¢¯¨Ï¡¾a¡¾iAo HDR_Info ***/
    UCHAR           szData[MAX_REQRES_HEADER];
} st_HttpHeader, *pst_HttpHeader;

#ifdef HTTP

#define DATA_SIZE 552
#define TCP_DATA st_HttpHeader
#define FILENAME "HTTP_HEADER"

#else /* HTTP */

#define DATA_SIZE 56
#define TCP_DATA st_PackHdr_t
#define FILENAME "PACKET_LOG"

#endif /* TCP - default */

int main(int ac, char **ar)
{
        int cnt;
        int rdcnt = 0;
        int fd;
        int i,j,feof;
	unsigned int fcur;
        int x;
	int abc;
	int offset;
	
        time_t seektime;
        TCP_DATA td,td2;

        char fname[30];
        char fullpath[80];
        char fpath[30];
	char ttime[21];
	char ctime[21];

	if( ac != 5 && ac != 6 ){
                printf("usage)\t[%s]\n",FILENAME);
		printf(" print count limit :  RUN [while-cnt] dir-name[sysno/mmdd|...] file-name[mmddhh] 1 \n");
                printf(" print seek-time   :  RUN [while-cnt] dir-name[sysno/mmdd|...] file-name[mmddhh] 2 seek-time[longtype] \n");
                printf(" print all count   :  RUN [while-cnt] dir-name[sysno/mmdd|...] file-name[mmddhh] 3 \n");
                printf(" print all count   :  RUN [while-cnt] dir-name[sysno/mmdd|...] file-name[mmddhh] 3 offset\n");
                exit(-1);
        }

        cnt = atoi(ar[1]);
        sprintf( fpath,"%s", ar[2] );
        sprintf( fname,"%s", ar[3] );
        sprintf( fullpath,"%s/%s", fpath, fname );
	x = atoi(ar[4]);

        if( x == 2 ){
		if( ac == 6 )
			seektime = atoi(ar[5]);
		else{
			printf("[CAUTION] if opt = 2, then seek-time option IS MUST NOT NULL!!!\n");
			exit(-3);
		}
	}
        
        printf(" cnt     :[%d]\n",cnt );
        printf(" fname   :[%s]\n",fname );
        printf(" fpath   :[%s]\n",fpath);
        printf(" fullpath:[%s]\n",fullpath);
	printf(" option  :[%d]\n",x);

	memset(ttime, 0x00, 21);
	memset(ctime, 0x00, 21);

        fd = open(fullpath, O_RDONLY);
        if( fd < 0 ){
                printf("[ERROR]file not open[%s:%d]\n",fullpath,fd);
                close(fd);
                exit(-2);
        }
        
	if( x != 3 ){
		printf("==========================================================================\n");
		printf("Cnt.\tTransTime.\t    CapTime. \t        localtime(Transtime)\n");
		printf("==========================================================================\n");
	}else{
		printf("==========================================================================\n");
		printf("fcur      :Cnt.\tTransTime.\t    CapTime. \t        localtime(Transtime)\n");
		printf("==========================================================================\n");
	}

        td2.transtime= 0;
        td2.transmtime= 0;

        feof = lseek(fd,0,SEEK_END);
        lseek(fd,0,SEEK_SET);

        if( x == 1 ){
                for( i=0;i<cnt;i++ ){
                        rdcnt = read(fd,&td, DATA_SIZE);
                        if( rdcnt != DATA_SIZE ){
                                printf("<CNT OPT[1] - SIZE IS NOT EQUAL> DATA_SIZE[%d]:RDCNT[%d]\n",DATA_SIZE,rdcnt);
                                i=cnt;
                                break;
                        }

                        if( td2.transtime!= td.transtime&& td2.transmtime!= td.transmtime ){
				memcpy( &ttime, asctime(localtime(&td.transtime)), 20);

                                printf("%07d : %d.%06d : %d.%06d : %s",
                                i,
                                td.transtime, 
                                td.transmtime, 
                                td.captime, 
                                td.capmtime,
				ttime);
/*
                                asctime(localtime(&td.transtime)));
*/

                                memcpy( &td2, &td, DATA_SIZE );
                        }
                        
                        memset(&td,0x00, DATA_SIZE);
                }
        }
        else if( x == 2 ){
                i = 0;
                j = 1;
                while(1){
                        rdcnt = read(fd,&td,DATA_SIZE);
                        
                        if( rdcnt != DATA_SIZE ){
                                printf("<SEEK OPT[2] - END OF FILE> DATA_SIZE[%d]:RDCNT[%d]\n",DATA_SIZE,rdcnt);
                                break;
                        }
                        if( seektime == td.transtime ){
				memcpy( &ttime, asctime(localtime(&td.transtime)), 20);
				memcpy( &ctime, asctime(localtime(&td.captime)), 20);
                                printf("%03d(%03d) : %d.%06d : %d.%06d (%s:%s)\n",
                                j,
                                ++i,
                                td.transtime, 
                                td.transmtime, 
                                td.captime, 
                                td.capmtime,
				ttime, ctime);
/*
				asctime(localtime(&td.transtime)),
				asctime(localtime(&td.captime)));
*/
	
				/* added info */
                        }
                        j++;

                        fcur = lseek(fd,0,SEEK_CUR);
                        if( fcur >= feof ){
                                printf("ENDOFFILE : %d\n",fcur);
                                break;
                        }

                        
                }
        }
        else if( x == 3 ){
		if( ac == 6 ){
			offset = atoi(ar[5]);
			fcur = offset*DATA_SIZE;
			printf(" [INFO]offset is [%d]\n",fcur);
			if( fcur >= feof ){
				printf("[WARN]offset[%d] is must less than FEOF[%d]\n",fcur,feof);
				exit(1);
			}
		}else
			fcur =0 ;
                i = 0;
                j = 1;
		if( fcur != 0 ){
                        lseek(fd,fcur,SEEK_SET);
		}
		for( abc=0;abc<cnt;abc++){
                        rdcnt = read(fd,&td,DATA_SIZE);
                        
                        if( rdcnt != DATA_SIZE ){
                                printf("<SEEK OPT[3] - END OF FILE> DATA_SIZE[%d]:RDCNT[%d]\n",DATA_SIZE,rdcnt);
                                break;
                        }
			memcpy( &ttime, asctime(localtime(&td.transtime)), 20);
			memcpy( &ctime, asctime(localtime(&td.captime)), 20);
			printf("%10d:%03d(%03d) : %d.%06d : %d.%06d (%s:%s)\n",
			fcur,
			j,
			++i,
			td.transtime, 
			td.transmtime, 
			td.captime, 
			td.capmtime,
			ttime, ctime);
/*
			asctime(localtime(&td.transtime)),
			asctime(localtime(&td.captime)));
*/
			/* additional info */
                        j++;

                        fcur = lseek(fd,0,SEEK_CUR);
                        if( fcur >= feof ){
                                printf("ENDOFFILE : %d\n",fcur);
                                break;
                        }

                        
                }
        }
        else{
                printf("[Unvalid OPT:%d]\n",x);
        }
        printf("==========================================================================\n");

        close(fd);

        printf("Finished Program.\n");
        

        return 0;
}
