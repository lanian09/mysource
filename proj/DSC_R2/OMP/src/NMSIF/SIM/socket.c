#include <nmsif_proto.h>
#include <nmsif.h>

int			max_sfd_num;

AppFdTbl	nms_fd_tbl;
fd_set		rfd_set, wfd_set, efd_set;
fd_set		tmp_rset, tmp_wset, tmp_eset;

struct timeval	wakeup_tmr;

extern	char	svr_ipaddr[];

init_client (int port)
{
	int		fd;
	struct	sockaddr_in	rmt_addr;

	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		printf ("[*] fail socket () : %s\n", strerror (errno));
		return -1;
	}

	if (set_sock_option (fd) < 0) {
		close (fd);
		return -1;
	}

	if (set_nonblock (fd) < 0) {
		close (fd);
		return -1;
	}

	memset (&rmt_addr, 0, sizeof (struct sockaddr_in));
	rmt_addr.sin_family		= AF_INET;
	rmt_addr.sin_port		= htons (port);
	rmt_addr.sin_addr.s_addr= inet_addr (svr_ipaddr);

	if (connect (fd, (struct sockaddr *)&rmt_addr, sizeof (rmt_addr)) < 0) {
		printf ("[*] fail connect () : %s\n", strerror (errno));
		return -1;
	}

	if (add_fd_tbl (fd, inet_addr (svr_ipaddr)) < 0) {
		return -1;
	}

	if (add_fd_set (fd) < 0) {
		del_fd_tbl (fd);
		return -1;
	}

	return fd;

} /* End of init_client () */



set_sock_option (int sfd)
{
	int		option=1;
	int		reuse=1;
	int		bsize;
	struct	linger	lin;

	if (setsockopt (sfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&option, sizeof (option)) < 0) {
		printf ("[*] fail setsockopt () for keepalive : %s\n", strerror (errno));
		return -1;
	}

	if (setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof (reuse)) < 0) {
		printf ("[*] fail setsockopt () for reuse : %s\n", strerror (errno));
		return -1;
	}

	lin.l_onoff		= 1;
	lin.l_linger	= 0;
	if (setsockopt (sfd, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof (lin)) < 0) {
		printf ("[*] fail setsockopt () for linger : %s\n", strerror (errno));
		return -1;
	}

	bsize = NMS_SEG_MAX_BUF;
	if (setsockopt (sfd, SOL_SOCKET, SO_RCVBUF, (char *)&bsize, sizeof (bsize)) < 0) {
		printf ("[*] fail setsockopt () for rx_size : %s\n", strerror (errno));
		return -1;
	}
	bsize = NMS_SEG_MAX_BUF;
	if (setsockopt (sfd, SOL_SOCKET, SO_SNDBUF, (char *)&bsize, sizeof (bsize)) < 0) {
		printf ("[*] fail setsockopt () for tx_size : %s\n", strerror (errno));
		return -1;
	}

	return 1;

} /* End of set_sock_option () */


set_nonblock (int sfd)
{
	int	flag;

	if ((flag = fcntl (sfd, F_GETFL, 0)) < 0) {
		printf ("[*] fail get fcntl : %s\n", strerror (errno));
		return -1;
	}

	flag |= O_NONBLOCK;

	if (fcntl (sfd, F_SETFL, flag) < 0) {
		printf ("[*] fail set fcntl : %s\n", strerror (errno));
		return -1;
	}
	return 1;

} /* End of set_nonblock () */


lookup_tcpaction (char *tcp_buf, int *act_sfd)
{
	int		rval=0;
	int		port=0;

	if (poll_fd_set () <= 0)
		return NMS_NO_EVENT;

	rval = analysis_event (act_sfd);
	
	if (rval > 0) {
		if (read_nms_data (tcp_buf, act_sfd) < 0) {
			close_nms_con (*act_sfd);
			return NMS_CLOSE_EVENT;
		}
		else return rval;
	}
	else return rval;

} /* End of lookup_tcpaction () */


poll_fd_set ()
{
	int		ret=0;
	struct	timeval	poll_tmr;

	memcpy (&tmp_rset, &rfd_set, sizeof (fd_set));
	memcpy (&tmp_eset, &efd_set, sizeof (fd_set));

	poll_tmr.tv_sec		= 0;
	poll_tmr.tv_usec	= POLLING_FD_TMR;

	ret = select (max_sfd_num, &tmp_rset, NULL, &tmp_eset, &poll_tmr);

	return ret;

} /* End of poll_fd_set () */


analysis_event (int *s_fd)
{
	int		i;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (nms_fd_tbl.fd[i] > 0) {
			if (FD_ISSET (nms_fd_tbl.fd[i], &rfd_set)) {
				*s_fd = nms_fd_tbl.fd[i];
				return NMS_DATA_EVENT;
			}
		}
	}
	return -1;

} /* End of analysis_event () */



read_nms_data (char *rxbuf, int *rsfd)
{
	int			i, rlen, blen, tlen;
	char		*rptr, tmp[200];
	Packet1		*rhd;

	rlen = blen = tlen =0;

	rptr = rxbuf;

	tlen = read (*rsfd, rptr, 1);

	if (tlen != 1) {
		printf ("[*] fail read 1th byte : %s\n", strerror (errno));
		return -1;
	}

	rptr += 1;
	rlen += 1;

	if (rxbuf[0] == 'A') {
		printf ("[*] rcv heart-beat (%c)\n", rxbuf[0]);
		return 1;
	}

	else if (rxbuf[0] == 'F') {
		for (i=0; (i<IO_RETRY_CNT) && (rlen < 129); i++) {

//lala
//printf ("[%d] tlen=%d, rlen=%d\n", i, tlen, rlen);

			tlen = read (*rsfd, rptr, 129-rlen);

			if (tlen == 0) {
				printf ("[*] fail read file_info (len=0) : %s\n", strerror (errno));
				return -1;
			}
			else if (tlen < 0) {
				printf ("[*] fail read file_info (len<0) : %s\n", strerror (errno));

				if (errno == EAGAIN || errno == EINTR) {
					select (0, 0, 0, 0, &wakeup_tmr);
					printf ("[*] retry read count (%d/%d)\n", i+1, IO_RETRY_CNT);
					continue;
				}
				return -1;
			}
			rptr += tlen;
			rlen += tlen;
		}

#if 0
		if (rxbuf[0] == 'A')
			send_heartbit ();
#endif

		return rlen;
	}
	else {
		for (i=0; (i<IO_RETRY_CNT) && (rlen < PKT1_HEAD_SIZE); i++) {

			tlen = read (*rsfd, rptr, NMS_PKT_HLEN-rlen);

//lala
//printf ("[%d] rlen=%d, tlen=%d\n", i, rlen, tlen);
			if (tlen == 0) {
				printf ("[*] fail read head (len=0) : %s\n", strerror (errno));
				return -1;
			}
			else if (tlen < 0) {

				printf ("[*] fail read head (len<0) : %s\n", strerror (errno));

				if (errno == EAGAIN || errno == EINTR) {
					select (0, 0, 0, 0, &wakeup_tmr);
					printf ("[*] retry read count (%d/%d)\n", i+1, IO_RETRY_CNT);
					continue;
				}
				return -1;
			}

			rptr += tlen;
			rlen += tlen;
		}
		if (i == IO_RETRY_CNT && rlen < blen) {
			printf ("[*] can't read body, disconnect(fd=%d) soon!\n", *rsfd);
			return -1;
		}

		rhd = (Packet1 *)rxbuf;
		blen = rhd->hdr.len - PKT1_HEAD_SIZE;

//lala
//printf ("[*] read header => body_len=%d(%d-%d), i=%d\n", 
//		blen, rhd->hdr.len, PKT1_HEAD_SIZE, i);

		for (i=0, rlen=0; (i<IO_RETRY_CNT) && (rlen < blen); i++) {

			tlen = read (*rsfd, rptr, blen-rlen);

			if (tlen == 0) {
				printf ("[*] fail read body (len=0) : %s\n", strerror (errno));
				return -1;
			}
			else if (tlen < 0) {

				printf ("[*] fail read body (len<0) : %s\n", strerror (errno));

				if (errno == EAGAIN || errno == EINTR) {
					select (0, 0, 0, 0, &wakeup_tmr);
					printf ("[*] retry read count (%d/%d)\n", i+1, IO_RETRY_CNT);
					continue;
				}
				return -1;
			}

			rptr += tlen;
			rlen += tlen;
		}

		if (i == IO_RETRY_CNT && rlen < blen) {
			printf ("[*] can't read body, disconnect(fd=%d) soon!\n", *rsfd);
			return -1;
		}

	//lala
#if 0
	{
		int		k;
		printf ("[nms<-bsdm] ----------------------------------------\n");
		for (k=0; k<rlen+NMS_PKT_HLEN; k++)
			printf ("[%x]", rxbuf[k]);
		printf ("\n----------------------------------------------------\n");
	}
#endif
		*rptr = 0;
		return (rlen+NMS_PKT_HLEN);
	}


} /* End of read_nms_data () */



add_fd_tbl (int sfd, int ip_addr)
{
	int		i;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (nms_fd_tbl.fd[i] == 0) {
			nms_fd_tbl.fd[i] 	= sfd;
			nms_fd_tbl.ipaddr[i] = ip_addr;
//lala
printf ("[add_fd_tbl] i=%d, sfd=%d\n", i, sfd);
			break;
		}
	}

	if (i == MAX_NMS_CON) {
		printf ("[*] fail add_fd_tbl () : fd(%d), ip(%s)\n", sfd, int2dot (ip_addr));
		return -1;
	}
	return 1;

} /* End of add_fd_tbl () */


del_fd_tbl (int sfd)
{
	int		i;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (nms_fd_tbl.fd[i] == sfd) {
			nms_fd_tbl.fd[i] 	= 0;
			nms_fd_tbl.ipaddr[i] = 0;
//lala
printf ("[del_fd_tbl] i=%d, sfd=%d\n", i, sfd);
			return 1;
		}
	}

	if (i == MAX_NMS_CON) {
		printf ("[*] fail del_fd_tbl () : not found fd(%d)\n", sfd);
		return -1;
	}
	return 1;

} /* End of del_fd_tbl () */


add_fd_set (int sfd)
{
	FD_SET	(sfd, &rfd_set);
	FD_SET	(sfd, &wfd_set);

	if (sfd >= max_sfd_num)
		max_sfd_num = sfd + 1;

	return 1;

} /* End of add_fd_set () */


del_fd_set (int sfd)
{
	FD_CLR (sfd, &rfd_set);
	FD_CLR (sfd, &wfd_set);

	if ((sfd + 1) == max_sfd_num)
		max_sfd_num--;

	return 1;

} /* End of del_fd_set () */


close_nms_con (int sfd)
{
	if (del_fd_tbl (sfd) < 0)
		return -1;

	del_fd_set (sfd);

	return 1;

} /* End of close_nms_con () */


send_packet (char *sbuf, int slen)
{
	int		sfd;
	int		i, wlen, tlen=0, ret=0;
	char	*sptr;
	fd_set	w_set;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (nms_fd_tbl.fd[i] > 0) {
			sfd = nms_fd_tbl.fd[i];
			break;
		}
	}
	if (i == MAX_NMS_CON) {
		printf ("[*] not found socket fd\n");
		return -1;
	}

	wakeup_tmr.tv_sec 	= 0;
	wakeup_tmr.tv_usec 	= 0;

	FD_ZERO (&w_set);
	FD_SET (sfd, &w_set);

	sptr = sbuf;

	for (i=0,wlen=0; (i<IO_RETRY_CNT) && (wlen<slen); i++) {

		ret = select (sfd+1, NULL, &w_set, NULL, &wakeup_tmr);

		if (ret == 0) {
			printf ("[*] would be blocked select : (try=%d/tot=%d/wlen=%d/remain=%d)\n",
					i+1, slen, wlen, slen-wlen);
			continue;
		}
		else if (ret < 0) {
			printf ("[*] fail select : %s\n", strerror (errno));
			close_nms_con (sfd);
			return -1;
		}

		tlen = write (sfd, sptr, slen-wlen);

		if (tlen < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				printf ("[*] would be blocked write : (try=%d/tot=%d/wlen=%d/remain=%d)\n",
					i+1, slen, wlen, slen-wlen);
				select (0, 0, 0, 0, &wakeup_tmr);
				continue;
			}
			printf ("[*] fail write : %s\n", strerror (errno));
			close_nms_con (sfd);
			return -1;
		}
		sptr += tlen;
		wlen += tlen;
	}

	if (wlen < slen) {
		printf ("[*] fail write completely : (try=%d/tot=%d/wlen=%d/remain=%d)\n",
			i+1, slen, wlen, slen-wlen);
		printf ("[*] ---> disconnect sfd (%d)\n", sfd);
		close_nms_con (sfd);
		return -1;
	}
#if 0
{
	int	k;
	printf ("[nms->bsdm] ----------------------------------------\n");
	for (k=0; k<wlen; k++)
		printf ("[%x]", sbuf[k]);
	printf ("\n----------------------------------------------------\n");
}
#endif
	return wlen;

} /* End of send_packet () */
