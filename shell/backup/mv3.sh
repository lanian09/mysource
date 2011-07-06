#!/bin/bash
#
# sample : CVS_1ST_BACKUP_200712210510.tar.gz
# 

function DEBUG()
{
	echo "[DEBUG] "$1
}

MAX_FCNT=4	# move phase1 max condition
EXIST_CNT=1	# only one match ok
DAY_LEN=8	# move phase2 argument length is ...

TRUE=1		# my defined true value
FALSE=0		# my defined false value

if [ -z $1 ]
then
	CDATE=`date +%Y%m%d`
	CFLAG=$FALSE
else
	if [ ${#1} -eq $DAY_LEN ]
	then
		DEBUG "check parameter length : "$DAY_LEN
		CDATE=$1
		CFLAG=$TRUE
	else
		echo "Param len is must $DAY_LEN. ex)20080304 - YYYYMMDD"
fi

function chk_name()
{
	#1st step
	MAINSTR=$1
	SUBSTR=$2
	IDX=`expr index $MAINSTR $SUBSTR`
	RSTSTR=${MAINSTR:$(($IDX-1))}

	if [ `expr match $RSTSTR $SUBSTR` -eq $DAY_LEN ]
	then
		return $TRUE
	else
		#2nd step
		IDX=`expr index $RSTSTR $SUBSTR`
		$RSTSTR=${RSTSTR:$(($IDX-1))}
		if [ `expr match $RSTSTR $SUBSTR` -eq $DAY_LEN ]
		then
			return $TRUE
		fi
	fi
	return $FALSE
}

function move2()
{
	echo "Start moving work-phase2 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo "Source Directory : "$1
	echo "Target Filename  : "$2
	echo "Destination Directory : "$3
	echo "========================================================"
	cd $1
	FCNT=`find . -name "$2" -ctime "$WCNT" -print | wc -l`
	for FN in `ls`
	do
		chk_name $FN $CDATE
		FLAG=$?
		if [ $FLAG -eq $TRUE ]
		then
			mv $FN $3
		fi
	done
	cd ..
	echo "========================================================"
	echo "Finished moving work-phase2 "
}

function move()
{
	echo "Start moving work-phase1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
	echo "Source Directory : "$1
	echo "Target Filename  : "$2
	echo "Destination Directory : "$3
	echo "========================================================"
	WCNT=0
	cd $1
	while [ $(($WCNT)) -lt $(($MAX_FCNT)) ]
	do
		FCNT=`find . -name "$2" -ctime "$WCNT" -print | wc -l`
		if [ $(($FCNT)) -eq $(($EXIST_CNT)) ]
		then
			echo "move target list is ...."
			find . -name "$2" -ctime "$WCNT" -print
			echo "move data?(Y/N)"
			read YORN
			if [ $YORN = "Y" -o $YORN = "y" ]
			then
				find . -name "$2" -ctime "$WCNT" -exec mv {} "$3" \;
			fi
			break;
		fi
		WCNT=$(($WCNT + 1))
	done
	cd ..
	echo "========================================================"
	echo "Finished moving work-phase1 "
}

function mv2()
{
	echo "PHASE 2"
}

function mv1()
{
	echo "PHASE 1"
}

PARAM1="/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
PARAM2="/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"

if [ $CFLAG -eq $TRUE ]
then
	echo "PHASE 1"
	mv2
#	move2  $PARAM1
#	move2  $PARAM2
else
	echo "PHASE 2"
	mv1
#	move $PARAM1
#	move $PARAM2
	#move "/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
	#move "/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"
fi

#echo "FINISHED MOVED"
#df -kh |grep "home"|awk '{print $4}'
exit
