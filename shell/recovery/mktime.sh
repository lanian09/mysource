#!/bin/bash
#1. make time...
TIME=$1
RUNCNT=$2
CNT=1
echo "RUNTIME="$TIME
echo "--->"
while [ 1 ]
do
	if [ $(($CNT)) -gt $(($RUNCNT)) ]
	then
		break
	fi
	RUNTIME=$(($TIME + (3600*($CNT-1))))
	echo "STARTTIME+"$CNT" ="$RUNTIME
	CNT=$(($CNT+1))
done
