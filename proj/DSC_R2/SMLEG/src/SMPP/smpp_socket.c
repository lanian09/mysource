/**********************************************************
   Author   : 
   Revision : 
   Description:

   Copyright (c) , Inc.
***********************************************************/
 
#include <smpp.h>

SMPP_FD_TBL 	client;
fd_set          fdset, rset;

void cut_socket (int idx) 
{
    int 	fd;

    if (idx < 0 || idx > MAX_SMC_INFO) {
		dAppLog(LOG_CRI, "[cut_socket] SMSC_ID Invalid = %d", idx); 
        return;
	}

    fd = client.condata[idx].fd;
    if (fd <= 0) {
		dAppLog(LOG_CRI, "[cut_socket] FD Invalid: SMSC_ID=%d FD=%d", idx, fd); 
        return;
	}
    close(fd);
	
    client.condata[idx].fd = 0;
	client.condata[idx].bind = 0;
	client.condata[idx].prototype = 0;
	client.condata[idx].bindtry = 0;
	client.condata[idx].writeFailCnt = 0;

    FD_CLR (fd, (fd_set *) &fdset);

    if (fd+1 >= client.maxfd)
        client.maxfd--;
    client.cur_con--;

    memset(&client.condata[idx].rbuf, 0, sizeof(SMPP_RBUF));

	dAppLog(LOG_CRI, "[cut_socket] Success: SMSC=%d FD=%d -> CURR_CON_CNT=%d MAX_FD=%d"
			, idx, fd, client.cur_con, client.maxfd);
    
	return;
}


int read_socket(int idx, int msg_len, char *msg)
{
    int     read_len, toread_len, n, i, fd;
	//char	abc[32];

	//memset(abc, 0, 32);

    fd = client.condata[idx].fd;
	if (fd <= 0) {
		dAppLog(LOG_CRI, "[read_socket] FD Invalid: FD=%d SMSC=%d LEN=%d", fd, idx, msg_len); 
		return -1;
	}

	toread_len = msg_len;	/* toread_len: read해야 할 data size 
							** 매 read 시마다 변경된다.
							*/
	read_len = 0;			/* read에 성공한 data size 누적분 
							** read에 성공할 때마다 변경된다. 
							**/
    for (i = 0; i < 5; i++) {
        if (read_len < msg_len) {
            n = read(fd, &msg[read_len], (size_t)toread_len);
            //n = read(fd, abc, (size_t)toread_len);
            if (n == 0 || (n < 0 && errno != EAGAIN)) {
				dAppLog(LOG_CRI, "[read_socket] read fail: RES=%d ERR=%d.%s SMSC=%d TO_READ_LEN=%d FD=%d",
					n, errno,  strerror(errno), idx, msg_len, fd);	
                return eP_Sock_Disconnect2;
            } else if (n < 0) {
				/* errno가 EAGAIN인 경우 */
                break;
            } else {
                read_len += n;
                if (read_len >= toread_len)
                    return 1;

				toread_len -= n;
            }
        }

    }

	dAppLog(LOG_CRI, "[read_socket] read len mismatch: SMSC=%d FD=%d TO_READ_LEN=%d READ_LEN=%d",
		idx, fd, msg_len, read_len);

    return (read_len + 2);	/* msg_len만큼 못 읽은 경우: 읽은 data size를 반환 
							** rval 를 구분할 수 있도록 +2 해서 반환한다 
							*/
}


void init_client_data (void) 
{
    memset(&client, 0, sizeof(SMPP_FD_TBL));
    FD_ZERO (&fdset);
}


int add_fd_tbl(int idx, int sockfd)
{
    client.cur_con++;
    
    if (sockfd >= client.maxfd)
        client.maxfd = sockfd + 1;

    memset(&client.condata[idx].rbuf, 0, sizeof(SMPP_RBUF));
    client.condata[idx].fd = sockfd;
	client.condata[idx].prototype = 0;
	client.condata[idx].type = inet_addr(smc_tbl->smc_info[idx].ip_addr);
	client.condata[idx].port_no = smc_tbl->smc_info[idx].port_no;

    FD_SET (sockfd, (fd_set *) &fdset);
#if 0
	dAppLog(LOG_CRI, "[add_fd_tbl] connect SUCCESS: SMSC=%d ADDR=%s FD=%d CON_CNT=%d",
        idx, smc_tbl->smc_info[idx].ip_addr, sockfd, client.cur_con);
#endif
	return idx;
}


void wake_up (int signo)
{
    ;
}


int check_socket(int *active_fd)
{
    int		sockfd, cnt=0, i;
    struct 	timeval  select_tm;

    memcpy((char *)&rset, (char *)&fdset, sizeof(rset));

    select_tm.tv_sec = 0;
    select_tm.tv_usec = 1000;

    if (select(client.maxfd, &rset, NULL, NULL,  &select_tm) <= 0) {
        return SMPP_NO_EVENT;
    }

	memset(active_fd, -1, sizeof(char)*MAX_SMC_INFO);
    for (i = 0; i < MAX_SMC_INFO; i++) 
	{
        if (client.condata[i].fd <= 0)
            continue;

        sockfd = client.condata[i].fd;
        if (FD_ISSET(sockfd, &rset)) {
            active_fd[cnt] = i;
            cnt++;
        }
    }
    return 0;
}


int write_sock_data(int idx, char *data, int len)
{
    int	fd, n, i;
	int towrite_len, written_len;

	if (idx < 0 || idx > MAX_SMC_INFO) {
		dAppLog(LOG_CRI, "[write_sock_data] SMSC_ID Invalid: SMSC=%d", idx);
		return -1;
	}

    fd = client.condata[idx].fd;
    if (fd <= 0) {
		dAppLog(LOG_CRI, "[write_sock_data] FD Invalid: SMSC=%d FD=%d", idx, fd); 
	   	return -1; 
	}

	if (len <= 0) {
		dAppLog(LOG_CRI, "[write_sock_data] len Invalid: SMSC=%d FD=%d LEN=%d", 
			idx, fd, len);
		return -1;
	}

	/*for(i=0; i < len; i++) {
		fprintf(stderr, "%.2X ", data[i]);
	}*/
	//fprintf(stderr, "\n");
	towrite_len = len;
	written_len = 0;
    for (i = 0; i < 5; i++) {
        if (towrite_len <= 0) 
            return 0;

        if ((n = write(fd, data, (size_t)towrite_len)) < 0) {
            if (errno != EAGAIN) {
				dAppLog(LOG_CRI, "[write_sock_data] write Fail: FD=%d ERR=%d SMSC=%d LEN=%d"
						, fd, errno, idx, towrite_len);
                cut_socket(idx);
                return -1;
            }
        } else if (n == towrite_len) {
			if(client.condata[idx].writeFailCnt > 0)
				client.condata[idx].writeFailCnt=0;	
            return 0;
        } else {
            towrite_len -= n;   
			written_len += n;	
        }
    }
				
	dAppLog(LOG_CRI, "[write_sock_data] write Final Fail: FD=%d SMSC=%d ORI_LEN=%d WR_LEN=%d LOOP=%d"
			, fd, idx, len, written_len, i); 

	 /* 2005-12-05.EKYANG 
     */
    client.condata[idx].writeFailCnt++;
    if (client.condata[idx].writeFailCnt > 10) {
        cut_socket(idx);
       	client.condata[idx].writeFailCnt=0; 
    }
	
    return -1;
}


int read_proto_data(int idx)
{
    int     rval;

    rval = read_complete_msg(idx);
//dAppLog(LOG_DEBUG, "DEBUG read_complete_msg RVAL = %d", rval);
    if (rval == 0 || rval == 1) {
		rval = check_smpp_ack(idx);
//dAppLog(LOG_DEBUG, "DEBUG check_smpp_ack = %d", rval);
	}

    return rval;
}

void proc_socket_data (void)
{
    int		i, j, err, act_idx;
	int		con_tbl[MAX_SMC_INFO];

	i = 0;
//    for (i = 0; i < 5; i++)
//	{
        err = check_socket(con_tbl);
        if (err < 0) {
           // break;
            return;
        } else if (err == SMPP_NO_EVENT) {
            //break;
            return;
		}

		//fprintf(stderr, "read select: err\n");

        for (j = 0; j < MAX_SMC_INFO; j++) {
            if (con_tbl[j] == -1) {
                //break;
                return;
			}

            act_idx = con_tbl[j];

            err = read_proto_data(act_idx);
            if (err < 0) {
				dAppLog(LOG_DEBUG, "READ FAIL:%d", err);
				if ((err == eP_Sock_Disconnect) || (err == eP_Sock_Disconnect2))
                	cut_socket(act_idx);

                //break;
                return;
            } 
        }
 //   }
}


int tcp_connect(int idx) 
{
    struct  sockaddr_in client_addr;
    int     sockfd, val;
    struct  sigaction   sigAct;
    struct  itimerval   locTimer;   

    client_addr.sin_family 		= AF_INET;
    client_addr.sin_port 		= htons(smc_tbl->smc_info[idx].port_no);
	client_addr.sin_addr.s_addr = inet_addr(smc_tbl->smc_info[idx].ip_addr);

    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		dAppLog(LOG_CRI, "[tcp_connect] socket Fail: ERR=%d SMSC=%d PORT=%d ADDR=%s",
				errno, idx, smc_tbl->smc_info[idx].port_no, smc_tbl->smc_info[idx].ip_addr); 

		return -1;
    }

    sigAct.sa_handler = wake_up;
    sigAct.sa_flags = 0;

    if ( sigaction (SIGALRM, &sigAct, NULL ) < 0 ) {
		dAppLog(LOG_CRI, "[tcp_connect] sigaction errno=%d", errno); 
        return -1;
    }

    locTimer.it_interval.tv_sec  = 0;
    locTimer.it_interval.tv_usec = 0;
    locTimer.it_value.tv_sec     = 0;
    locTimer.it_value.tv_usec    = 50000;
    setitimer (ITIMER_REAL, &locTimer, NULL);

    if (connect(sockfd, (struct sockaddr *)&client_addr, sizeof (client_addr)) < 0) {
		dAppLog(LOG_CRI, "[tcp_connect] connect Fail: ERR=%d(%s) SMSC=%d PORT=%d ADDR=%s",
			errno, strerror(errno), idx, smc_tbl->smc_info[idx].port_no, smc_tbl->smc_info[idx].ip_addr); 
        close(sockfd);
        return -1;
    }

    alarm(0);

    val = fcntl (sockfd, F_GETFL, 0);
    fcntl (sockfd, F_SETFL, (val |= O_NDELAY));
    add_fd_tbl(idx, sockfd);

    return sockfd;
}


/* 1초 주기로 connection, bind 상태 점검하고 retry */
void manage_con_sts(void)
{
    int 	i;

	for (i = 0; i < MAX_SMC_INFO; i++)
	{
		/* 등록한 ipaddr 정보와 실제 connection의 ipaddr 정보가 다를 경우 
		** -> connection이 있는 상태에서 SMSC 정보를 변경한 경우
		*/  
		if (client.condata[i].fd != 0) {
			if (inet_addr(smc_tbl->smc_info[i].ip_addr) != client.condata[i].type ||
				smc_tbl->smc_info[i].port_no != client.condata[i].port_no) {
				dAppLog(LOG_CRI, "[Manage_Con_sts] IP_ADDR/PORT Chnaged(SMSC=%d) -> cut_socket", i);
				cut_socket(i);
				continue;
			}
		}

		/* 정보가 설정되어 있는 SMSC에 대한 connection이 없으면 tcp_connect */ 
		if (client.condata[i].fd == 0) {
    		if (strlen(smc_tbl->smc_info[i].ip_addr) <= 8)
				continue;

			tcp_connect(i);
		}

		/* connection은 있는데 SMSC와 Bind 가 되어 있지 않고
        ** Retry 한 상태가 아니면 bind_try 
		*/
		if (client.condata[i].fd != 0 && client.condata[i].bind == 0) {
			if (client.condata[i].bindtry == SOCK_BIND_TRY_STS) 
				continue;

			protocol_bind_try(i);
		}
	}
}

