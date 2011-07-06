#!/bin/bash
CVTTIME=2
ENDTIME=1
echo "CVTTIME="$CVTTIME", ENDTIME="$ENDTIME
while [ 1 ]
do
	if [ $(($CVTTIME)) -gt $(($ENDTIME)) ]
	then
		echo $CVTTIME" A "
		break
	fi
	echo "NOT BREAK"
done
echo "END"


#!/bin/bash
STARTTIME=1199113200
ENDTIME=1201100400
DELIMITER=1234560
ADDTIME=86400
SRCFILE=temp_up_c.sql
DESTFILE=cluster.sql
CVTTIME=$STARTTIME
while [ 1 ]
do
    if [ $(($CVTTIME)) -gt $(($ENDTIME)) ]
    then
        break
    fi
    #bash cvt.sh $DELIMITER $STARTTIME $SRCFILE $DESTFILE
    CVTTIME=$(($CVTTIME + $ADDTIME))
    echo $CVTTIME
done
