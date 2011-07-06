#!/bin/bash
SUBSTR="20080202"
LEN=${#SUBSTR}
SUBSTR2="CVS_"
for VAR in "CVS_2ST_BACKUP_200802020201.tar.gz" "CVS_2ND_BACKUP_200802010301.tar.gz"
do
	echo $VAR
	#echo "$SUBSTR index="`expr index $VAR $SUBSTR`
	#echo "$SUBSTR2 index="`expr index $VAR $SUBSTR2`
	#echo "$SUBSTR match="`expr match $VAR $SUBSTR`
	#echo "$SUBSTR2 match="`expr match $VAR $SUBSTR2`
	IDX=`expr index $VAR $SUBSTR`
	echo "[IDX1]$SUBSTR($LEN) index="$IDX
	SEARCH_STR=${VAR:$(($IDX-1))}
	echo "[SEARCH_STR1]="$SEARCH_STR
	if [ `expr match $SEARCH_STR $SUBSTR` -eq $LEN ]
	then
		echo "[MATCH1]= "$SEARCH_STR
	else
		#echo "SEARCH_STR="${SEARCH_STR:1}
		if [ $SEARCH_STR != $SUBSTR ]
		then
			SUB=${VAR:$(($IDX))}
			IDX=`expr index $SUB $SUBSTR`
			echo "[IDX2]$SUBSTR($LEN) index="$IDX
			SEARCH_STR=${SUB:$(($IDX-1))}
			echo "[SEARCH_STR2]="$SEARCH_STR
			if [ `expr match $SEARCH_STR $SUBSTR` -eq $LEN ]
			then 
				echo "[MATCH2]="$SEARCH_STR
			fi
		fi
	fi
done

