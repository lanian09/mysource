#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/errno.h>
#include <nmsif.h>

void trigger_msg ();
char *decode_primitive (int);
char *int2dot (int);

int		my_port;
char	svr_ipaddr[20];

static int	swhw_flag;

extern	AppFdTbl	nms_fd_tbl;

extern	int	errno;

main (int argc, char **argv)
{
	int			ret, r_sfd;
	int			argval;
	int			l_cnt, k;
	char		rbuf[MAX_NMS_BUF];
	CommonPkt	*rx_pkt;
	Packet1		*rpkt1;
	Packet2		*rpkt2;
	Packet3		*rpkt3;

	if (argc != 2) {
		printf ("[usage] nmsim 1/2/3/4/5\n");
		printf ("              1 => Alarm Port (5023)\n");
		printf ("              2 => Console Port (5021)\n");
		printf ("              3 => Configure Port (5041)\n");
		printf ("              4 => MMC Port (5061)\n");
		printf ("              5 => Perform Port (6110)\n");
		puts ("");
		exit (1);
	}

	argval = atoi (argv[1]);

	if (argval == 1)		my_port = 5023;
	else if (argval == 2)	my_port = 5021;
	else if (argval == 3)	my_port = 5041;
	else if (argval == 4)	my_port = 5061;
	else if (argval == 5)	my_port = 6110;


	if (init (my_port) < 0)
		exit (1);

	swhw_flag = 0;

	printf ("myport=%d\n", my_port);

	if (my_port == 5023 || my_port == 5041)
		signal (SIGINT, trigger_msg);

	while (1) {

		memset (rbuf, 0, MAX_NMS_BUF);	ret=0;
		ret = lookup_tcpaction (rbuf, &r_sfd);

		if (ret == NMS_DATA_EVENT) {
			rx_pkt = (CommonPkt *)rbuf;

			if (rbuf[0] == 'A' || rbuf[0] == 'F') {
				printf ("[*] (%s)\n", rbuf);

				if (rbuf[0] == 'F') {
					send_finfo ((char *)&rbuf[1]);
				}
				continue;
			}

#if 0
			printf ("-------------------------------------------------------------\n");
			printf (" Ver CtrlBit Type CtrlSeq Src Dest Prmt Len  \n");
			printf ("-------------------------------------------------------------\n");
			printf (" %d    %d     %d    %d     %d  %d   %d  %d\n", 
					rx_pkt->hdr.chd.ver, rx_pkt->hdr.chd.ctrlbit, rx_pkt->hdr.chd.pktype,
					rx_pkt->hdr.chd.ctrlseq, rx_pkt->hdr.chd.src, rx_pkt->hdr.chd.dest,
					rx_pkt->hdr.prmt, rx_pkt->hdr.len);
			printf ("-------------------------------------------------------------\n");
#endif

			/* packet 1 贸府 : control packet ************************************/
			if (rx_pkt->hdr.chd.pktype == 1) {
				rpkt1 = (Packet1 *)rx_pkt;

				if (rpkt1->hdr.len > PKT1_HEAD_SIZE)
					printf ("[%s]\n\n", rpkt1->data);
			}

			/* packet 2 贸府 : console packet ************************************/
			else if (rx_pkt->hdr.chd.pktype == 2) {

				rpkt2 = (Packet2 *)rx_pkt;

				//printf ("[%s]\n", decode_primitive (rpkt2->msgtype));

				if (rpkt2->hdr.len > PKT2_HEAD_SIZE + sizeof (rpkt2->msgtype))
					printf ("[%s]\n\n", rpkt2->data);

			}

			/* packet 3 贸府 : current_alm packet ************************************/
			else if (rx_pkt->hdr.chd.pktype == 3) {
				rpkt3 = (Packet3 *)rx_pkt;
				printf ("[%s][%d][%d]\n", decode_primitive (rpkt3->msgtype), 
							rpkt3->serno, rpkt3->attrcnt);

				if (rpkt3->hdr.len > PKT3_HEAD_SIZE + sizeof (rpkt2->msgtype) 
									+ sizeof (rpkt3->serno) + sizeof (rpkt3->attrcnt))
					printf ("[%s]\n\n", rpkt3->data);
			}
		}

		if (!(l_cnt++%60)) {
			printf ("[*] check send_heartbit () : %d\n", time ((time_t *)0));
			for (k=0; k<MAX_NMS_CON; k++)
				if (nms_fd_tbl.fd[k] > 0)
					send_heartbit ();

			if (l_cnt > 10000)	l_cnt = 0;
		}

		sleep (1);
	}

} /* End of main () */



init (int port)
{

	memset (&nms_fd_tbl, 0, sizeof (AppFdTbl));

	/* get listen ipaddr */
	if (get_server_ipaddr () < 0)
		return -1;

	if (init_client (port) < 0)
		return -1;

	return 1;

} /* End of init () */



get_server_ipaddr ()
{
	int		cnt=0;
	char	lbuf[200];
	FILE	*fp;

	if ((fp = fopen ("./SVRIPADDR", "r")) == NULL) {
		printf ("[*] can't open file (%s) :%s\n", "./SVRIPADDR", strerror (errno));
		return -1;
	}

	while (fgets (lbuf, 200, fp) != NULL) {
		if (lbuf[0] == '#')	continue;
		strcpy (svr_ipaddr, lbuf);
		cnt++;
	}
	if (cnt > 0)
		return 1;
	else return -1;

} /* End of get_server_ipaddr () */


void trigger_msg ()
{
	char		txbuf[4096];
	Packet2		*pkt2;
	Packet3		*pkt3;

	signal (SIGINT, trigger_msg);

	memset (txbuf, 0, 4096);

	/* alarm port */
	if (my_port == 5023) {
		swhw_flag++;
		pkt2 = (Packet2 *)txbuf;
		pkt2->hdr.chd.ver		= 0;
		pkt2->hdr.chd.ctrlbit	= 0;
		pkt2->hdr.chd.pktype	= 2;
		pkt2->hdr.chd.ctrlseq	= 0;
		pkt2->hdr.chd.src		= 0x01;
		pkt2->hdr.chd.dest		= 0x03;
		pkt2->hdr.prmt			= 0;
		pkt2->hdr.len			= PKT2_HEAD_SIZE;

		if (!(swhw_flag % 2))
			pkt2->msgtype		= PRMT_INIT_HWALM_REQ;
		else pkt2->msgtype		= PRMT_INIT_SWALM_REQ;

		if (swhw_flag > 10000)	swhw_flag = 0;

		//lala
		printf ("[*] trigger_msg : msgtype=%d, len=%d\n", 
				pkt2->msgtype, pkt2->hdr.len);
		send_packet (txbuf, PKT2_HEAD_SIZE);
	}
	/* configuration port */
	else if (my_port == 5041) {
	}
}


char *decode_primitive (int msgid)
{
}


char *int2dot (int ipaddr)
{
	static char     dot_buf[20];
	struct in_addr  n_info;

	memset (dot_buf, 0, 20);
	n_info.s_addr = ipaddr;
	sprintf (dot_buf, "%s", inet_ntoa (n_info));

	return dot_buf;
}


send_heartbit ()
{
	char	hbbuf[200];

	memset (hbbuf, 0, 200);

	hbbuf[0] = 'I';

	printf ("[send_heartbit(%d)] \n(%c)\n", time ((time_t *)0), hbbuf[0]);
	send_packet (hbbuf, 1);

	return 1;

} /* End of send_heartbit () */


send_finfo (char *st_fname)
{
	char	hbbuf[200];

	memset (hbbuf, 0, 200);

	sprintf (hbbuf, "Y%s", st_fname);

	printf ("[send_finfo] (%s)\n", st_fname);
	send_packet (hbbuf, 129);

	return 1;

} /* End of send_finfo () */
