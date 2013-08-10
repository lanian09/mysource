#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <taf_names.h>
#include <pre_a_api.h>
#include <taf_shm.h>
#include <ippool_bitarray.h>
#include <memg.h>

stHASHOINFO *pInfo;

int main(int argc, char *argv[])
{
	TAG_KEY_LPREA_CONF key;
	struct in_addr in;
	char *addr;
	int port, rppi_flag;

    InitAppLog(getpid(), SEQ_PROC_PRE_A, LOG_PATH"PRE_A", "PRE_A");
    Init_shm_common();

	printf("\n");
	if( argc < 4 ) {
		printf("USAGE: %s [IP] [PORT] [RPPIFLAG]\n", argv[0]);
		printf("\n");
		exit(-1);
	}

	addr = argv[1];
	if( inet_pton(AF_INET, addr, &in) <= 0 ) {
		printf("IP ADDRESS [%-15s] INVALID FORMAT.\n", addr);
		printf("\n");
		exit(-3);
	}

	port = atoi(argv[2]);
	if( port < 0 ) {
		printf("USAGE: %s [IP] [PORT] [RP|PI]\n", argv[0]);
		printf("\n");
		exit(-1);
	}

	if( strcasecmp(argv[3], "RP") == 0 )
		rppi_flag = RP_FLAG;
	else if( strcasecmp(argv[3], "PI") == 0 )
		rppi_flag = PI_FLAG;
	else {
		printf("USAGE: %s [IP] [PORT] [RP|PI]\n", argv[0]);
		printf("\n");
		exit(-1);
	}

    if((pInfo = hasho_init(S_SSHM_LPREA_HASH, TAG_KEY_LPREA_CONF_SIZE, TAG_KEY_LPREA_CONF_SIZE, LPREA_CONF_SIZE, CONF_PREA_CNT , 0 )) == NULL ) {
        printf("[%s][%s.%d] hasho_init LPREA_CONF NULL.\n", __FILE__, __FUNCTION__, __LINE__);
		printf("\n");
        exit(-2);
    }

	memset(&key, 0, sizeof(key));
	key.SIP = ntohl(in.s_addr);
	key.SPort = port;
	key.RpPiFlag = rppi_flag;

	if( hasho_find(pInfo, (char *)&key) == NULL ) {
		printf("[%s]IP ADDRESS [%-15s : %10u] NOT FOUND.\n", rppi_flag==RP_FLAG?"RP":"PI", addr, key.SIP);
	}
	else {
		printf("[%s]IP ADDRESS [%-15s : %10u] FOUND.\n", rppi_flag==RP_FLAG?"RP":"PI", addr, key.SIP);
	}
	printf("\n");

	return 0;
}

