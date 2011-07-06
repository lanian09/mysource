#!/bin/bash
ROOT=/WGTAMAPP/SCRIPT/AQUA
ROOT=.
PRJ=wntas

if [ -z $1 ]
then 
#	DATE=`date -u +20%y%m%d`
#	DATE=`date --date " 1 days ago " +%Y%m%d`
	DATE=`env TZ=KST+15 date +'%Y%m%d'`
	CDATE=`date +%Y%m%d`
	TIME=
else
	DATE=$1
fi

if [ -z $2 ]
then
	CPATH=.
else
	CPATH=$2
fi

ARGS='s/%TIME%/'$DATE'/g'
CFN=$ROOT/$PRJ'_quality_'$DATE.sql
APATH=$CPATH/$PRJ'_quality_'$CDATE.dat


echo "set heading off" > $CFN
echo "set linesize 430" >> $CFN
echo "set pagesize 0" >> $CFN
echo "set feedback off" >> $CFN
echo "" >> $CFN
echo "spool $APATH" >> $CFN

sed $ARGS $ROOT/$PRJ.sql >> $CFN

echo "spool off" >> $CFN

echo "quit;" >> $CFN

#sqlplus tas/tas123 @$CFN

#rm $CFN
