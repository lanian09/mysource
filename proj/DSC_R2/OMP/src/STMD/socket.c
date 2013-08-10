#include <nmsif_proto.h>
#include <nmsif.h>

int			max_sfd_num;

//AppFdTbl	*nms_fd_tbl;

fd_set		rfd_set, wfd_set, efd_set;
fd_set		tmp_rset, tmp_wset, tmp_eset;

struct timeval	wakeup_tmr;

extern	ListenInfo	ne_info;
extern  SFM_sfdb	*sfdb;
extern	char		traceBuf[];
extern	int			trcFlag, trcLogFlag;


init_listen (int pidx)
{
	int		fd;
	struct	sockaddr_in	loc_addr;

	if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		sprintf (traceBuf, "fail socket () : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
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

	memset (&loc_addr, 0, sizeof (struct sockaddr_in));
	loc_addr.sin_family		= AF_INET;
	loc_addr.sin_port		= htons (ne_info.port[pidx]);
	loc_addr.sin_addr.s_addr= htonl (INADDR_ANY);

	if (bind (fd, (struct sockaddr *)&loc_addr, sizeof (loc_addr)) < 0) {
		printf ("[*] fail bind () : %s\n", strerror (errno));
		return -1;
	}

	if (listen (fd, MAX_NMS_CON) < 0) {
		printf ("[*] fail listen () : %s\n", strerror (errno));
		return -1;
	}

	if (add_fd_tbl (fd, inet_addr (ne_info.ipaddr[0]), 
					ne_info.port[pidx], FD_TYPE_LISTEN) < 0) {
		close (fd);
		return -1;
	}

	if (add_fd_set (fd) < 0) {
		del_fd_tbl (fd);
		return -1;
	}

	return fd;

} /* End of init_listen () */



set_sock_option (int sfd)
{
	int		option=1;
	int		reuse=1;
	int		bsize;
	struct	linger	lin;

	if (setsockopt (sfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&option, sizeof (option)) < 0) {
		sprintf (traceBuf, "fail setsockopt () for keepalive : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	if (setsockopt (sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof (reuse)) < 0) {
		sprintf (traceBuf, "fail setsockopt () for reuse : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	lin.l_onoff		= 1;
	lin.l_linger	= 0;
	if (setsockopt (sfd, SOL_SOCKET, SO_LINGER, (char *)&lin, sizeof (lin)) < 0) {
		sprintf (traceBuf, "fail setsockopt () for linger : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	bsize = NMS_SEG_MAX_BUF;
	if (setsockopt (sfd, SOL_SOCKET, SO_RCVBUF, (char *)&bsize, sizeof (bsize)) < 0) {
		sprintf (traceBuf, "fail setsockopt () for rx_size : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	bsize = NMS_SEG_MAX_BUF;
	if (setsockopt (sfd, SOL_SOCKET, SO_SNDBUF, (char *)&bsize, sizeof (bsize)) < 0) {
		sprintf (traceBuf, "fail setsockopt () for tx_size : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	return 1;

} /* End of set_sock_option () */


set_nonblock (int sfd)
{
	int	flag;

	if ((flag = fcntl (sfd, F_GETFL, 0)) < 0) {
		sprintf (traceBuf, "fail get fcntl : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	flag |= O_NONBLOCK;

	if (fcntl (sfd, F_SETFL, flag) < 0) {
		sprintf (traceBuf, "fail set fcntl : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	return 1;

} /* End of set_nonblock () */


lookup_tcpaction (char *tcp_buf, int *act_sfd)
{
	int	rval=0;
	int	port=0;
	int	result=0;

	if (poll_fd_set () <= 0)
		return NMS_NO_EVENT;

	rval = analysis_event (act_sfd, &port);

	if (rval == NMS_CONN_EVENT) {
		//result = handle_nms_connect (act_sfd, port);

		// handle_nms_connect에서 -1이면 fd_tbl에 add할수 없다. add by helca 20081109 
		if( handle_nms_connect (act_sfd, port) < 0 ){
			close_nms_con (*act_sfd);
			return NMS_CLOSE_EVENT;
		}
		/* NMS에서 통계 포트로 접속 시 이전에 저장된 통계화일명 전송
		*/
		/* handle_nms_connect의 결과 -1 인경우 통계화일명 전송 불가함. */
		if( result != -1) { // add by helca 2008.06.26
			retrans_file_name ();
		}

		return rval; 
	}
	else if (rval == NMS_DATA_EVENT) {
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
	int	ret=0;
	struct	timeval	poll_tmr;

	memcpy (&tmp_rset, &rfd_set, sizeof (fd_set));
	memcpy (&tmp_eset, &efd_set, sizeof (fd_set));

	poll_tmr.tv_sec		= 0;
	poll_tmr.tv_usec	= POLLING_FD_TMR;

	ret = select (max_sfd_num, &tmp_rset, NULL, &tmp_eset, &poll_tmr);

	return ret;

} /* End of poll_fd_set () */


analysis_event (int *s_fd, int *p_port)
{
	int		i;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (sfdb->nmsInfo.fd[i] > 0) {

			if (FD_ISSET (sfdb->nmsInfo.fd[i], &tmp_rset)) {
				*s_fd	= sfdb->nmsInfo.fd[i];
				*p_port	= sfdb->nmsInfo.port[i];

//lala
#if 0
printf ("[*] fd_isset => fd(%d), type(%d)\n", 
		sfdb->nmsInfo.fd[i], sfdb->nmsInfo.ptype[i]);
#endif
				if (sfdb->nmsInfo.ptype[i] == FD_TYPE_LISTEN)
					return NMS_CONN_EVENT;

				else return NMS_DATA_EVENT;

			}
		}
	}
	return -1;

} /* End of analysis_event () */


handle_nms_connect (int *lsfd, int pport)
{
	int		fd, len;
	struct	sockaddr_in	cliaddr;

	len = sizeof (cliaddr);
	memset (&cliaddr, 0, sizeof (struct sockaddr_in));

	if ((fd = accept (*lsfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
		sprintf (traceBuf, "[handle_nms_connect]fail accept for sfd(%d) : %s\n", *lsfd, strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

//lala
//printf ("[*] accept new sfd(%d)\n", fd);

	if (set_sock_option (fd) < 0) {
		close (fd);
		return -1;
	}

	if (set_nonblock (fd) < 0) {
		close (fd);
		return -1;
	}

	if (add_fd_tbl (fd, cliaddr.sin_addr.s_addr, pport, FD_TYPE_DATA) < 0) {
		close (fd);
		sprintf (traceBuf, "[handle_nms_connect] add_fd_tbl fail fd(%d), port(%d) \n",
			fd, pport);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	if (add_fd_set (fd) < 0) {
		del_fd_tbl (fd);
		return -1;
	}
	return 1;

} /* End of handle_nms_connect () */


read_nms_data (char *rxbuf, int *rsfd)
{
	int			i, k, rlen, blen, tlen;
	char		*rptr, tmp[200];
	Packet1		*rhd;

	rlen = blen = tlen =0;

	rptr = rxbuf;

	tlen = read (*rsfd, rptr, 1);

	if (tlen != 1){
		sprintf (traceBuf, "fail read 1th byte : %s\n", strerror (errno));
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}

	rptr += 1;
	rlen += 1;

	if (rxbuf[0] == 'I') {
		send_heartbit (rsfd);
		return 1;
	}
	if (rxbuf[0] == 'Y') {

		for (i=0; (i<IO_RETRY_CNT) && (rlen < 129); i++) {
			tlen = read (*rsfd, rptr, 129-rlen);

			if (tlen == 0) {
				sprintf (traceBuf, "fail read file_info (len=0) : %s\n", strerror (errno));
				trclib_writeLogErr (FL, traceBuf);
				return -1;
			}
			else if (tlen < 0) {
				sprintf (traceBuf, "fail read file_info (len<0) : %s\n", strerror (errno));
				trclib_writeLogErr (FL, traceBuf);

				if (errno == EAGAIN || errno == EINTR) {
					select (0, 0, 0, 0, &wakeup_tmr);
					sprintf (traceBuf, "retry read count (%d/%d)\n", i+1, IO_RETRY_CNT);
					trclib_writeLogErr (FL, traceBuf);
					continue;
				}
				return -1;
			}
			rptr += tlen;
			rlen += tlen;
		}

		memset (tmp, 0, 200);
#if 0
		for (k=1; k<129; k++) {
			if (rxbuf[k] != ' ')
				tmp[k-1] = rxbuf[k];
		}
		tmp[k-1] = '\0';
#endif
		//lala
		if (trcFlag || trcLogFlag) {
			sprintf (traceBuf, "rcv file_info (%s) from NMS\n", &rxbuf[1]);
			trclib_writeLog (FL, traceBuf);
		}

		reset_fname (&rxbuf[1], *rsfd);

		return rlen;
	}
	else {
		for (i=0; (i<IO_RETRY_CNT) && (rlen < PKT1_HEAD_SIZE); i++) {

			tlen = read (*rsfd, rptr, NMS_PKT_HLEN-rlen);

			if (tlen == 0) {
				sprintf (traceBuf, "fail read header (len=0) : %s\n", strerror (errno));
				trclib_writeLogErr (FL, traceBuf);
				return -1;
			}
			else if (tlen < 0) {

				sprintf (traceBuf, "fail read header (len<0) : %s\n", strerror (errno));
				trclib_writeLogErr (FL, traceBuf);

				if (errno == EAGAIN || errno == EINTR) {
					select (0, 0, 0, 0, &wakeup_tmr);
					sprintf (traceBuf, "retry read count (%d/%d)\n", i+1, IO_RETRY_CNT);
					trclib_writeLogErr (FL, traceBuf);
					continue;
				}
				return -1;
			}

			rptr += tlen;
			rlen += tlen;
		}

		if (i == IO_RETRY_CNT && rlen < NMS_PKT_HLEN) {
			sprintf (traceBuf, "can't read header, disconnect(fd=%d) soon!\n", *rsfd);
			trclib_writeLogErr (FL, traceBuf);
			return -1;
		}

		rhd = (Packet1 *)rxbuf;
		blen = rhd->hdr.len - PKT1_HEAD_SIZE;

		for (i=0, rlen=0; (i<IO_RETRY_CNT) && (rlen < blen); i++) {

			tlen = read (*rsfd, rptr, blen-rlen);

			if (tlen == 0) {
				sprintf (traceBuf, "fail read body (len=0) : %s\n", strerror (errno));
				trclib_writeLogErr (FL, traceBuf);
				return -1;
			}
			else if (tlen < 0) {

				sprintf (traceBuf, "fail read body (len<0) : %s\n", strerror (errno));
				trclib_writeLogErr (FL, traceBuf);

				if (errno == EAGAIN || errno == EINTR) {
					select (0, 0, 0, 0, &wakeup_tmr);
					sprintf (traceBuf, "retry read count (%d/%d)\n", i+1, IO_RETRY_CNT);
					trclib_writeLogErr (FL, traceBuf);
					continue;
				}
				return -1;
			}

			rptr += tlen;
			rlen += tlen;
		}

		if (i == IO_RETRY_CNT && rlen < blen) {
			sprintf (traceBuf, "can't read body, disconnect(fd=%d) soon!\n", *rsfd);
			trclib_writeLogErr (FL, traceBuf);
			return -1;
		}

	//lala
#if 0
	{
		int		k;
		printf ("[bsdm<-nms] ----------------------------------------\n");
		for (k=0; k<rlen+NMS_PKT_HLEN; k++)
			printf ("[%x]", rxbuf[k]);
		printf ("\n----------------------------------------------------\n");
	}
#endif
		*rptr = 0;
		return (rlen+NMS_PKT_HLEN);
	}


} /* End of read_nms_data () */



add_fd_tbl (int sfd, int ip_addr, int port_num, int port_type)
{
	int		i;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (sfdb->nmsInfo.fd[i] == 0) {
			sfdb->nmsInfo.fd[i] 	= sfd;
			sfdb->nmsInfo.ipaddr[i]	= ip_addr;
			sfdb->nmsInfo.port[i]	= port_num;
			sfdb->nmsInfo.ptype[i]	= port_type;

			if (trcFlag || trcLogFlag) {
				sprintf (traceBuf, "add_fd_tbl : i=%d, sfd=%d, ip=%s, port=%d, ptype=%d\n", i, 
						sfd, int2dot (ip_addr), port_num, port_type);
				trclib_writeLog (FL, traceBuf);
			}
			break;
		}
	}

	if (i >= MAX_NMS_CON) {
		sprintf (traceBuf, "fail add_fd_tbl () max_nms_con: fd(%d), ip(%s), port(%d), ptype(%d)\n",
				sfd, int2dot (ip_addr), port_num, port_type);
		trclib_writeLogErr (FL, traceBuf);
		return -1;
	}
	return 1;

} /* End of add_fd_tbl () */


del_fd_tbl (int sfd)
{
	int		i;

	for (i=0; i<MAX_NMS_CON; i++) {
		if (sfdb->nmsInfo.fd[i] == sfd) {
			if (trcFlag || trcLogFlag) {
				sprintf (traceBuf, "del_fd_tbl : i=%d, sfd=%d, ip=%s, port=%d, ptype=%d\n", i, 
						sfd, int2dot (sfdb->nmsInfo.ipaddr[i]), 
						sfdb->nmsInfo.port[i], sfdb->nmsInfo.ptype[i]);
				trclib_writeLog (FL, traceBuf);
			}
			sfdb->nmsInfo.fd[i] 	= 0;
			sfdb->nmsInfo.ipaddr[i]	= 0;
			sfdb->nmsInfo.port[i]	= 0;
			sfdb->nmsInfo.ptype[i]	= 0;

			return 1;
		}
	}

	if (i >= MAX_NMS_CON) {
		sprintf (traceBuf, "fail del_fd_tbl () : not found fd(%d)\n", sfd);
		trclib_writeLogErr (FL, traceBuf);
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

//printf ("[add_fd_set] max_fd=%d, sfd=%d\n", max_sfd_num, sfd);
	return 1;

} /* End of add_fd_set () */


del_fd_set (int sfd)
{
	FD_CLR (sfd, &rfd_set);
	FD_CLR (sfd, &wfd_set);

	if ((sfd + 1) == max_sfd_num)
		max_sfd_num--;

//printf ("[del_fd_set] max_fd=%d, sfd=%d\n", max_sfd_num, sfd);

	return 1;

} /* End of del_fd_set () */


close_nms_con (int sfd)
{
	if (del_fd_tbl (sfd) < 0)
		return -1;

	del_fd_set (sfd);

	return 1;

} /* End of close_nms_con () */


send_packet (int sfd, char *sbuf, int slen)
{
	int		i, wlen, tlen=0, ret=0;
	char	*sptr;
	fd_set	w_set;

	wakeup_tmr.tv_sec 	= 0;
	wakeup_tmr.tv_usec 	= 0;

	FD_ZERO (&w_set);
	FD_SET (sfd, &w_set);

	sptr = sbuf;

	for (i=0,wlen=0; (i<IO_RETRY_CNT) && (wlen<slen); i++) {

		ret = select (sfd+1, NULL, &w_set, NULL, &wakeup_tmr);

		if (ret == 0) {
			sprintf (traceBuf, "would be blocked select : (try=%d/tot=%d/wlen=%d/remain=%d)\n",
					i+1, slen, wlen, slen-wlen);
			trclib_writeLogErr (FL, traceBuf);
			continue;
		}
		else if (ret < 0) {
			sprintf (traceBuf, "fail select : %s\n", strerror (errno));
			trclib_writeLogErr (FL, traceBuf);
			close_nms_con (sfd);
			return -1;
		}

		tlen = write (sfd, sptr, slen-wlen);

		if (tlen < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				sprintf (traceBuf, "would be blocked write : (try=%d/tot=%d/wlen=%d/remain=%d)\n",
					i+1, slen, wlen, slen-wlen);
				trclib_writeLogErr (FL, traceBuf);
				select (0, 0, 0, 0, &wakeup_tmr);
				continue;
			}
			sprintf (traceBuf, "fail write : %s\n", strerror (errno));
			trclib_writeLogErr (FL, traceBuf);
			close_nms_con (sfd);
			return -1;
		}
		sptr += tlen;
		wlen += tlen;
	}

	if (wlen < slen) {
		sprintf (traceBuf, "fail write completely : (try=%d/tot=%d/wlen=%d/remain=%d)\n",
			i+1, slen, wlen, slen-wlen);
		trclib_writeLogErr (FL, traceBuf);
		//printf ("[*] ---> disconnect sfd (%d)\n", sfd);
		close_nms_con (sfd);
		return -1;
	}
//lala
#if 0
{
	int		k;
	printf ("[bsdm->nms] ----------------------------------------\n");
	for (k=0; k<wlen; k++)
		printf ("[%x]", sbuf[k]);
	printf ("\n----------------------------------------------------\n");
}
#endif
	return wlen;

} /* End of send_packet () */
