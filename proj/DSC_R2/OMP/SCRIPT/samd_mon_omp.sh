#!/bin/bash

export IV_HOME=/DSC
export MY_SYS_NAME=DSCM
export MYSQL_HOME=/opt/mysql/mysql
export PATH=:/usr/bin:/usr/sbin:/usr/local/bin:/usr/ccs/bin:/usr/local/sbin/:/usr/ucb:.:/usr/sbin:/usr/bin:/opt/mysql/mysql/bin:/usr/java/bin
export LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:/usr:/usr/local/ssl/lib:/opt/mysql/mysql/lib:/usr/sfw/lib:/usr/sfw/lib


NumProc=`ps -ef|grep -i 'samd$'|grep -v grep|wc -l`

if [ $NumProc = 0 ]
then
	echo SAMD "DEAD" $NumProc
	/DSC/BIN/startprc -b SAMD
else
	echo SAMD "ALIVE" $NumProc
	/DSC/BIN/disprc
fi
