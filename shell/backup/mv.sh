#!/bin/bash
#
# sample : CVS_1ST_BACKUP_200712210510.tar.gz
# 

#function DEBUG()
#{
#	echo "[DEBUG] "$1
#}

MAX_FCNT=4	# move phase1 max condition
EXIST_CNT=1	# only one match ok
DAY_LEN=8	# move phase2 argument length is ...

TRUE=1		# my defined true value
FALSE=0		# my defined false value

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

PARAM1="/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
PARAM2="/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"

if [ $CFLAG -eq $TRUE ]
then
	move2  $PARAM1
	move2  $PARAM2
else
	move $PARAM1
	move $PARAM2
	#move "/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
	#move "/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"
fi

#echo "FINISHED MOVED"
#df -kh |grep "home"|awk '{print $4}'
#exit
