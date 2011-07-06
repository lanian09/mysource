#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct sample_hdr{
	short int ver;
	short int type;
	char      filename[32];
	char      desc[32];
} st_Hdr, *pst_Hdr;

typedef struct sample_data{
	long int     offset;
	unsigned int captime;
	char         key[16];
	short int    len;
} st_Data, *pst_Data;

typedef struct sample_pcap_hdr{
	unsigned int ts_sec;
	unsigned int ts_usec;
	unsigned int incl_len;
	unsigned int orig_len;
} st_pcap_hdr, *pst_pcap_hdr;

#define BUF_SIZE 30000
#define PACKET_CNT 15

void writeFile(char *fn,char *fnd)
{
	int     fd,fdd,dRet,i,len;
	st_Hdr  ah;
	st_Data ad[PACKET_CNT];
	st_pcap_hdr pcap;
	char    buf[BUF_SIZE];
	char    cbuf[BUF_SIZE];
	char    err[BUF_SIZE];
	time_t  tt;
	ssize_t ss;

	for(i = 0; i< BUF_SIZE; i++){
		buf[i] = 'a'+(i%25);
	}

	memset((void*)cbuf,0x00,BUF_SIZE);

	ah.ver=123;
	ah.type=0;
	sprintf(ah.filename,"FILENAME0");
	sprintf(ah.desc,"DESC0");

	fd = open(fn, O_CREAT|O_RDWR, S_IRUSR|S_IRGRP|S_IROTH );
	if( fd < 0 ){ perror("failed open"); return; }

	fdd = open(fnd, O_CREAT|O_RDWR, S_IRUSR|S_IRGRP|S_IROTH );
	if( fdd < 0 ){ perror("failed open"); return; }

	//global header write
	ss = write(fd, (char*)&ah, sizeof(st_Hdr));
	if( ss < 0 ){ perror("failed write"); return; }


	printf("write----\noffset:captime:key:len :: PCAP:ts_sec:ts_usec:incl_len:orig_len :packet\n");
	for( i=0,len=0; i <PACKET_CNT;i++){
		tt = time(NULL);
		ad[i].offset = len;
		ad[i].captime= tt+(i*100 + tt%1000 + tt>>(i*10));
		sprintf(ad[i].key,"4500601%04d%04d",i,i+10);
		ad[i].len = i+10;

		len += sizeof(st_pcap_hdr) + ad[i].len;

		//INDEX
		ss = write(fd, (char*)&ad[i], sizeof(st_Data));
		if( ss < 0 ){
			printf("failed write1[%d] file=%s\n",i, fn);
			sprintf(err,"%d:%s", errno,strerror(errno));
			perror(err);
			return;
		}
		printf("%ld:%u:%s:%d", ad[i].offset, ad[i].captime, ad[i].key, ad[i].len);
#if 1
		pcap.ts_sec = 12345+i;
		pcap.ts_usec = 123456;
		pcap.incl_len= ad[i].len;
		pcap.orig_len= ad[i].len;

		//DATA-HDR
		ss = write(fdd, (char*)&pcap, sizeof(st_pcap_hdr));
		if( ss < 0 ){ perror("pcaphdr write fail"); };
		//DATA-BODY
		ss = write(fdd, buf, ad[i].len );
		if( ss < 0 ){
			printf("failed write2[%d] file=%s\n",i, fn);
			sprintf(err,"%d:%s", errno,strerror(errno));
			perror(err);
			return;
		}
		strncpy(cbuf, buf, ad[i].len);
		printf(" :: %u:%u:%u:%u :%s", pcap.ts_sec, pcap.ts_usec, pcap.incl_len, pcap.orig_len, cbuf);
#endif
		printf("\n");

	}

	close(fd);
	close(fdd);
}

long int readd(int fd, st_Data *sd)
{
		int ret;
		ret = read(fd, (char*)sd, sizeof(st_Data));
		if( ret < 0 ) perror("read error2:");
		printf("%ld:%u:%s:%d\n",sd->offset, sd->captime, sd->key, sd->len);
		return sd->offset;
}

void readFile(char *fn,char *fnd)
{
	int fd,fdd, i,ret, pcnt;
	long int len, offset;
	st_Hdr sh;
	st_Data sd;
	st_pcap_hdr pcap;
	char buf[BUF_SIZE];

	memset(buf,0x00,BUF_SIZE);
	fd = open(fn,O_RDONLY);
	if( fd < 0 ) perror("open error:");

	fdd = open(fnd, O_RDONLY);
	if( fdd < 0 ){ perror("failed open"); return; }

	ret = read(fd, (char*)&sh, sizeof(st_Hdr));
	if( ret < 0 ) perror("read error1:");

	printf("ver       =%d\n", sh.ver);
	printf("type      =%d\n", sh.type);
	printf("filename  =%s\n", sh.filename);
	printf("desc      =%s\n", sh.desc);

	len = (long int)lseek(fd,0,SEEK_END);
	printf("sizeof file=%ld\n", len);
	pcnt = (len - sizeof(st_Hdr))/sizeof(st_Data);
	printf("packet cnt=%d, sthdr=%d, data=%d\n", pcnt, sizeof(st_Hdr), sizeof(st_Data));
	lseek(fd,sizeof(st_Hdr),SEEK_SET);
	for(i=0;i<pcnt;i++){
		printf("HDR[%d]",i);
		//DATA offset read
		offset = readd(fd, &sd);
#if 1
		//FILE POSITION MOVING
		lseek(fdd, offset, SEEK_SET);

		//pcap hdr read
		ret = read(fdd, (char*)&pcap, sizeof(pcap));
		if( ret < 0 ) { perror("read pcap");  continue; }
		printf("pcap[%ld] info[%d] ts_sec(%u) ts_usec(%u) offset(%u) orig_len(%u)\n", offset, i, pcap.ts_sec, pcap.ts_usec, pcap.incl_len, pcap.orig_len);

		//pcap data read
		ret = read(fdd, buf, pcap.incl_len);
		if( ret < 0 ) { perror("read error3:"); continue; }
		printf("buf=%s\n",buf);

		buf[sd.len] = 0x00;
#endif

	}
/*
	ret = read(fd,buf,30000);
	for( i = 0; i< ret; i++){
		printf("%c",buf[i]);
		if( !i%50 )
			printf("\n");
	}
*/

	close(fd);
	close(fdd);
}

int main()
{
	char idx[30];
	char data[30];
	sprintf(idx,"test.idx");
	sprintf(data,"test.pcap");
	writeFile(idx,data);
	readFile(idx,data);
	return 0;
}

