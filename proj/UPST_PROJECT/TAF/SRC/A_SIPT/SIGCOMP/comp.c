#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>

#include "sigcomp.h"



/**********************************
 *   Ascii Character to Hex Binary
 *   **********************************/
char
ascii2hex(char c)
{
    if(c >= '0' && c <= '9') return (c-'0');
    else if(c >= 'a' && c <= 'z') return (c-'a'+10);
    else if(c >= 'A' && c <= 'Z') return (c-'A'+10);
}

/**********************************
 *   convert Hex-String to Binary
 *   **********************************/
int
hexs2bin(char *buf, char *bin)
{
    char nn1, nn2;
    int len = strlen(buf);
    int i, j;


    for(i=0, j=0; i<len; j++, i+=2) {
        nn1 = ascii2hex(buf[i]);
        nn2 = ascii2hex(buf[i+1]);
        bin[j] = nn1 << 4 | nn2;
    }

    return (len/2);
}

main()
{
    struct sockaddr_in dest_addr, from_addr;
    struct timeval timeout;
    fd_set fdset;
    int fd, ret;
    int from_len=sizeof(from_addr), reuse=1;
    int ilen, olen;
    char in[65536], out[65536];
    FILE *fp;

    int cnt = 1;
	memset(in, 0x00, sizeof(in));
	memset(out, 0x00, sizeof(out));

    printf("sigcomp_init_protocol!\n");
    sigcomp_init_protocol();

    fd = OpenUDPSocket(8888);
    if (fd < 0) {
        printf("socket open error\n");
        exit(0);
    }

#if 1
    fp = fopen("msg", "r");
    fread(in, sizeof(in), 1, fp);
#else
	strcpy(in, "REGISTER sip:sunny@sip.softeleware.com SIP/2.0\r\nVia: SIP/2.0/UDP 10.120.1.171:5060\r\nContent-Length: 0\r\n\r\n");
#endif

    /* destination */
    memset((char *)&dest_addr, 0, sizeof(struct sockaddr_in));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_addr.s_addr = INADDR_ANY;
    dest_addr.sin_port = 7777;

    while(1) {
#if 0
        memset(out, 0x00, sizeof(out));
        memset(in, 0x00, sizeof(in));

        gets(in);
#endif
        olen = comp_sig(1, in, strlen(in), out);

#if 1
		printf("comp_sig() len=%d\n", olen);
#endif

        if (olen > 0) {
            sendto(fd, out, olen, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));

            printf("SigComp Message ==========================================================\n");
            debugdump(out, olen);
        }
		udvm_state_print();




        /* release test */
        if (cnt++==2) {
            sigcomp_release(1, 1);
            udvm_state_print();
            exit(0);
        }
    }



}
