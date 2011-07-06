#!/bin/bash
ctime=`date +%s`
cdate=`date +%Y%m%d`

echo "ctime:"$ctime
echo "cdate:"$cdate


#!/bin/bash
CD=`date +%d`
CM=`date +%m`
CY=`date +%Y`

ACD=$(($CD-1))
ACM=$(($CM-1))
ACY=$(($CY-1))

if [ $CD -ne 1 ]
then
	echo "ACD:"$ACD
else
then
	if [ $CM -ne 1 ]
	then
		echo "ACM-ACD:"$ACM-$ACD

echo '[1]'$CY-$CM-$CD
echo '[1]'$ACY-$ACM-$ACD

echo '[2]'$CY-$CM-$CD
echo '[2]'$ACY-$ACM-$ACD
