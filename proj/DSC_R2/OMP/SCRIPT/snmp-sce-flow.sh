#!/usr/bin/bash

SCEA1='scea'
SCEA2='SCEA'
SCEB1='sceb'
SCEB2='SCEB'
DESTA='192.168.100.11'
DESTB='192.168.100.13'

if [ "$1" = "$SCEA1" ] || [ "$1" = "$SCEA2" ] 
then
	HOST='192.168.100.3'
elif [ "$1" = "$SCEB1" ] || [ "$1" = "$SCEB2" ]
then 
	HOST='192.168.100.4'
else
	echo 'Invalid parameter...'
	echo $0 'scea|SCEA|sceb|SCEB'
	exit 1
fi

ALIVE_CHK=`/usr/sbin/ping $HOST 1 | awk '{ print $3; }'`

if [ $ALIVE_CHK != "alive" ]; then
	echo "sce not alive";
	exit 1
fi

#counter=0
#while [ $counter -lt 22 ]; do
#while true; do
snmpget -v 1 -c public $HOST \
PCUBE-SE-MIB::tpNumActiveFlows.1.1 \
PCUBE-SE-MIB::tpNumActiveFlows.1.2 \
PCUBE-SE-MIB::tpNumActiveFlows.1.3 \
	
#2>&1 | cut -d'=' -f2 \
#| awk '{
#	if($1=="INTEGER:")  { split($2, tmp, "("); split(tmp[2], arr, ")"); printf "%s\n",arr[1]; }
#	if($1=="STRING:") printf "%s %s %s\n",$2, $3, $4;
#	if($1=="Gauge32:") printf "%s\n",$2;
#}'

#	if($1=="STRING:") printf "%s\n",$2;	# ()
#	if($1=="Gauge32:") printf "%s\n",$2;	# $2
#2>&1 | cut -d'=' -f2 
# 2 stderr
# 1 stdout
# 0 stdin


#2>&1 | cut -d '=' -f2 | cut -d ':' -f2 | cut -d '(' -f1 | sed 's/^ //g'
#2>&1 | cut -d '=' -f2 | cut -d ':' -f2 | cut -d '(' -f1 


#PCUBE-SE-MIB::tpCpuUtilization.1.3 2>&1 | awk '{ print $4 }' | cut -d '(' -f 1 
#echo $counter'========================================='
#let counter=counter+1;
#sleep 1
#done
#done
