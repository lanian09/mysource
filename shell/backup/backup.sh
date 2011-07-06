#!/bin/bash
# sample : CVS_1ST_BACKUP_200712210510.tar.gz
# 
MAX_FCNT=4
EXIST_CNT=1
CDATE=`date +%Y%m%d`
function move()
{
	WCNT=0
	cd $1
	while [ $(($WCNT)) -lt $(($MAX_FCNT)) ]
	do
		FCNT=`find . -name "$2" -ctime "$WCNT" -print | wc -l`
		if [ $(($FCNT)) -eq $(($EXIST_CNT)) ]
		then
			#echo "move target list is ...."
			#find . -name "$2" -ctime "$WCNT" -print
			find . -name "$2" -ctime "$WCNT" -exec mv {} "$3" \;
			break;
		fi
		WCNT=$(($WCNT + 1))
	done
	cd ..
}

move "/home/cvsbackup/CVS_1ST/" "CVS_1ST_BACKUP*.tar.gz" "/.backup/cvs_1st/"
move "/home/cvsbackup/CVS_2ND/" "CVS_2ND_BACKUP*.tar.gz" "/.backup/cvs_2nd/"

#echo "FINISHED MOVED"
#exit

SYS_DF=`df -kh |grep "home"|awk '{print $4}'`
SYS_RET=${SYS_DF%\%}
MAX_SIZE=95
MAX_CNT=30
function clear()
{
	cd $1
	CNT=`find . -name "CVS*BACKUP*.tar.gz" -print | wc -l`
	
	# removed condition is file-size zero
	find . -size 0 -exec rm -rf {} \;

	# removed condition is file-count 
	if [ $(($CNT)) -gt $(($MAX_CNT)) ]
	then
		find . -name "CVS*BACKUP*.tar.gz" -ctime +30 -exec rm -rf {} \;
	fi

	# removed condition is disk-usage
	if [ $(($SYS_RET)) -gt $(($MAX_SIZE)) ]
	then
		cd ..
		bash clear.sh
		cd $1
	fi
	cd ..
}

# remove, if -gt 30
clear /home/cvsbackup/CVS_1ST
clear /home/cvsbackup/CVS_2ND

#df -kh |grep "home"|awk '{print $4}'
