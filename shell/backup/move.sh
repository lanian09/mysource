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

INVALID=99	# invalid return value

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
		CFLAG=$INVALID
	fi
fi

function chk_name()
{
	MAINSTR=$1
	SUBSTR=$2
	RSTSTR=$MAINSTR
	FFLAG=$FALSE
	while [ $FFLAG -eq $FALSE ]
	do
		if [ ${#RSTSTR} -lt $DAY_LEN ]
		then
			return $FALSE
		fi
		IDX=`expr index $RSTSTR $SUBSTR`
		RSTSTR=${RSTSTR:$(($IDX-1))}
		DEBUG "MAINSTR="$MAINSTR", SUBSTR="$SUBSTR", IDX="$IDX", RSTSTR(${#RSTSTR})="$RSTSTR
		if [ `expr match $RSTSTR $SUBSTR` -eq $DAY_LEN ]
		then 
			return $TRUE
		fi
		RSTSTR=${RSTSTR:$IDX}
	done
	return $FALSE
}

function move2()
{
	echo "Start moving work-phase2 "
	echo "	Source Directory      : "$1
	echo "	Target Filename       : "$2
	echo "	Destination Directory : "$3
	echo "========================================================"
	cd $1
	for FN in `ls`
	do
		chk_name $FN $CDATE
		FLAG=$?
		if [ $FLAG -eq $TRUE ]
		then
			DEBUG "FILENAME="$FN
			mv $FN $3
			RETFLAG=$TRUE
		fi
	done
	cd ..
	echo "========================================================"
	echo "Finished moving work-phase2 "
	echo ""
}

function move()
{
	cd $1
	WCNT=0
	FCNT=`find . -name "$2" -ctime "$WCNT" -print | wc -l`
	if [ $FCNT -gt 0  ]
	then
		echo "Start moving work-phase1 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
		echo "	Source Directory      : "$1
		echo "	Target Filename       : "$2
		echo "	Destination Directory : "$3
		echo "========================================================"
		#WCNT=0
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
					RETFLAG=$TRUE
				fi
				break;
			fi
			WCNT=$(($WCNT + 1))
		done
		echo "========================================================"
		echo "Finished moving work-phase1 "
		echo ""
	fi
	cd ..
}

SOURCE_DIR1="/home/cvsbackup/CVS_1ST/"
SOURCE_DIR2="/home/cvsbackup/CVS_2ND/"
TARGET1="CVS_1ST_BACKUP*.tar.gz"
TARGET2="CVS_2ND_BACKUP*.tar.gz"
DEST_DIR1="/.backup/cvs_1st/"
DEST_DIR2="/.backup/cvs_2nd/"

RETFLAG=$FALSE
echo "CHK TIME : "$CDATE
if [ $CFLAG -eq $TRUE ]
then
	move2 $SOURCE_DIR1 $TARGET1 $DEST_DIR1
	move2 $SOURCE_DIR2 $TARGET2 $DEST_DIR2
#	move2 "/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
#	move2 "/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"
elif [ $CFLAG -eq $FALSE ]
then
	move $SOURCE_DIR1 $TARGET1 $DEST_DIR1
	move $SOURCE_DIR2 $TARGET2 $DEST_DIR2
#	move "/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
#	move "/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"
fi

if [ $RETFLAG = $TRUE ]
then
	echo "FINISHED MOVED"
else
	echo "MOVED NOTHING"
fi
#df -kh |grep "home"|awk '{print $4}'
#exit
