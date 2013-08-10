#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <arpa/inet.h>

#include "sigcomp.h"
#include "udvm.h"


int main(int argc, char *argv[])
{
    struct sockaddr_in from_addr;
    struct timeval timeout;
    fd_set fdset;
    int fd, ret;
    int ilen, olen, local_id;
    char in[65536], out[65536];

    int cnt = 0;


    printf("sigcomp_init_protocol!\n");
    sigcomp_init_protocol();


    fd = OpenUDPSocket(7777);
    if (fd  < 0) {
        printf("socket open error\n");
        exit(0);
    }


    while (1) {
        FD_ZERO(&fdset);
        FD_SET(fd, &fdset);

        timeout.tv_sec = 1;
        timeout.tv_usec = 1;

        ret = select(FD_SETSIZE, &fdset, NULL, NULL, &timeout);
        if (ret > 0) {
            if (FD_ISSET(fd, &fdset)) {
                FD_CLR(fd, &fdset);

                memset(out, 0x00, sizeof(out));
                memset(in, 0x00, sizeof(in));

                ilen = ReadUDPSocket(fd, in, sizeof(in), 0, (struct sockaddr *)&from_addr);

                if (ilen>0) {

                    printf("\n\nSigComp message received =================================================\n");
                    debugdump(in, ilen);
                    
                    udvm_state_print();
                    /* decompression */
                    local_id = decomp_sig(in, ilen, out, &olen);

                    udvm_state_print();
                    
                    if (local_id > 0) {
                        if (olen > 0) {
                            printf("Decompressed packet ======================================================\n(%d) [%s]\n", olen, out);
                            decomp_confirm(local_id, 10);
                            udvm_state_print();


                            /* SIP processing */



                            /* release test */
                            if(cnt++==7) {
                                sigcomp_release(10, 0);
                                udvm_state_print();
                                exit(0);
                            }
                        }
                    }
                    else if (local_id == 0) {
                        printf("plain text\n");

                            /* SIP processing */
                    }
                    else if (local_id == -1) {
                        printf("null, only TCP!\n");
                    }
                    else {
                        printf ("error!\n");
                    }

                }
                else {
                }
            }
            else {
            }
        }

    }
}
