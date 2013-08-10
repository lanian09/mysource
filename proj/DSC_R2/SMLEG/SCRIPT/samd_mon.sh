#!/usr/bin/bash

# samd auto-start 등록 방법.
# 1.MY_SYS_NAME=SCMA or SCMB 등록 
# 2.본 스크립트를 /DSC/NEW/STATUS 로 복사한다. (cp samd_mon.sh /DSC/NEW/STATUS)
# 3. crontab에 해당 쉘을 구동 등록 
#	 crontab -e
#    * * * * * /DSC/NEW/STATUS/samd_mon.sh > /dev/null 2>&1
# 4. samd 구동 확인

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

