#!/bin/bash
#1. date 관리 - 복구 시간을 binary time으로 받는다.
#2. table name - 결정한다. if문으로 분기
#3. 회수.
#*) GTAM 용

function usage
(
	echo "usage )"
	echo "./bash recovery.sh <STARTTIME> <TABLENAME> <RUNNING COUNT>"
	echo "- STARTTIME : Binary format time"
	echo "- TABLENAME : "
	echo "<No> 	<table> 			<timeunit>"
	echo "1 	STAT_BTS 			hour"
	echo "2 	STAT_BTS_TOTAL		day"
	echo "3 	STAT_CLUSTER 		hour" 
	echo "4		STAT_CLUSTER_TOTAL	day"
	echo "bye~ ^^"
)
# PARAMETER 개수 관리 3개를 받는다.
echo $#
if [ $# -ne 3 ]
then
	usage
	exit
fi

#2. date 관리;
TIME=$1

#3. FILE NAME
if [ $2 -eq 1 ]; then FILENAME=stat_bts.sql
elif [ $2 -eq 2 ]; then FILENAME=stat_bts_total.sql
elif [ $2 -eq 3 ]; then FILENAME=stat_cluster.sql
elif [ $2 -eq 4 ]; then FILENAME=stat_cluster_total.sql
else 
	echo "RANGE ERROR "
	usage
	exit
fi

#4.RUNNING COUNT
RUNCNT=$3
MAX_RUNCNT=24
if [ $(($RUNCNT)) -gt $(($MAX_RUNCNT)) ]
then
	echo "OVER RUN MAX_RUN CNT IS "$MAX_RUNCNT
	echo "bye ~"
	exit
fi

echo "TIME="$TIME
echo "RUN="$RUNCNT
echo "FILE="$FILENAME

CNT=1
echo "" > tmp_$FILENAME
while [ 1 ]
do
	if [ $(($CNT)) -gt $(($RUNCNT)) ]
	then
		break
	fi
	RUNTIME=$(($TIME + (3600*($CNT-1))))
	sed s/%u/$RUNTIME/g $FILENAME >> tmp_$FILENAME
	CNT=$(($CNT+1))
done
