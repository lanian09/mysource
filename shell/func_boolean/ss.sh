#!/bin/bash
function chkb()
{
	if [ $1 = "B" ]
	then
		return 1
	fi
	return 0
}

PARAM="A"
echo "PARAM="$PARAM
chkb $PARAM
RET=$?
if [ $RET -eq 1 ]
then 
	echo "TRUE1"
else
	echo "FALSE1"
fi

PARAM="B"
echo "PARAM2="$PARAM
chkb $PARAM
RET=$?
if [ $RET -eq 1 ]
then 
	echo "TRUE2"
else
	echo "FALSE2"
fi


