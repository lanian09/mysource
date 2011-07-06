#!/bin/bash
FILENAME="CVS*BACKUP*.tar.gz"
function clear()
{
	#echo $1
        cd $1
        CNT=`find . -name "$FILENAME" -print | wc -l`
	
	FCNT=`find . -size 0 -exec ls -al {} \; | wc -l`
	if [ $FCNT -ne 0 ]
	then
		find . -size 0 -exec ls -al {} \;
		echo "[SIZE ZERO]removed data? (Y/N)"
		read YORN
		if [ $YORN = "Y" -o $YORN = "y" ]
		then
			find . -size 0 -exec rm -rf {} \;
		fi
	fi

	echo "Current Cnt="$CNT", MAX Check Cnt="$MAX_CNT
        if [ $(($CNT)) -gt $(($MAX_CNT)) ]
        then
                find . -name "$FILENAME" -ctime +30 -exec ls -al {} \;
		echo "[OVER TIME]removed data? (Y/N)"
		read YORN
		if [ $YORN = "Y" -o $YORN = "y" ]
		then
			find . -name "$FILENAME" -ctime +30 -exec rm -rf {} \;
		fi
	else
		echo "NOT OVER CURRENT="$CNT ", MAX_CNT=$MAX_CNT"
		echo
        fi
        cd ..
}
MAX_CNT=30

# remove, if -gt 30
clear CVS_1ST
clear CVS_2ND
