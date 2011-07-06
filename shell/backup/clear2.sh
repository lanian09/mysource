#!/bin/bash
function clear()
{
	#echo $1
        cd $1
        CNT=`find . -name "CVS*BACKUP*.tar.gz" -print | wc -l`
        if [ $(($CNT)) -gt $(($MAX_CNT)) ]
        then
                find . -name "CVS*BACKUP*.tar.gz" -ctime +30 -exec rm -rf {} \;
	else
		#echo "NOT OVER MAX_CNT=$MAX_CNT"
		echo
        fi
        cd ..
}
MAX_CNT=25

# remove, if -gt 30
clear CVS_1ST
clear CVS_2ND
