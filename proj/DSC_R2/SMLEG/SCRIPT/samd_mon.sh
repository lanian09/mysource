#!/usr/bin/bash

# samd auto-start ��� ���.
# 1.MY_SYS_NAME=SCMA or SCMB ��� 
# 2.�� ��ũ��Ʈ�� /DSC/NEW/STATUS �� �����Ѵ�. (cp samd_mon.sh /DSC/NEW/STATUS)
# 3. crontab�� �ش� ���� ���� ��� 
#	 crontab -e
#    * * * * * /DSC/NEW/STATUS/samd_mon.sh > /dev/null 2>&1
# 4. samd ���� Ȯ��

export IV_HOME=/DSC
export MY_SYS_NAME=SCMA
export MYSQL_HOME=/opt/mysql/mysql
export PATH=:/usr/bin:/usr/sbin:/usr/local/bin:/usr/ccs/bin:/usr/local/sbin/:/usr/ucb:.:/usr/sbin:/usr/bin:/opt/mysql/mysql/bin:/usr/java/bin
export LD_LIBRARY_PATH=/lib:/usr/lib:.:/usr/local/lib:/usr:/usr/local/ssl/lib:/opt/mysql/mysql/lib:/DSC/NEW/LIB                  
NumProc=`ps -ef|grep -i 'samd$'|grep -v grep|wc -l`
 
if [ $NumProc = 0 ]
then
    echo SAMD "DEAD" $NumProc
    /DSC/NEW/BIN/startprc -b SAMD
else
echo SAMD "ALIVE" $NumProc
    /DSC/NEW/BIN/disprc
fi

