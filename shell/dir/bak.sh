#!/bin/bash
DELIMITER="./"
LIMIT_LEN=${#DELIMITER}
if [ -z $1 ]
then
	echo "need to param "
	exit
fi

if [ -d $1 ]
then
	echo "moved $1"
else
	echo "param is must directory, '$1' is not directory"
	exit
fi

cd $1

CDATE=`date +%Y%m%d`

function mk_pkg()
{
	CD=$1
	ND=$CD".new"
	cp -r $CD $ND
	echo "tar cvfz PKG_"$CD"_"$CDATE".tar.gz" $ND
	rm -rf $ND
}

PCNT=0
FLIST=`find . -type d -print`
for DN in $FLIST
do
	if [ ${#DN} -lt $LIMIT_LEN ]
	then
		continue
	fi

	if [ ${DN:0:$LIMIT_LEN} == $DELIMITER ]
	then
		#DEBUG CONFIRM 1 
		#[CODE]BEFORE=$DN"->"
		DN=${DN:$LIMIT_LEN}
		#DEBUG CONFIRM 2
		#[CODE]echo $BEFORE""$DN
	fi

	mk_pkg $DN
	PCNT=$(($PCNT+1))
	echo "[DEBUG] running count is = "$PCNT
done
