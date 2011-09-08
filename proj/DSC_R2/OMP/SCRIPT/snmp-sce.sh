#!/usr/bin/bash

SCEA='scea'
SCEB='sceb'
DESTA='192.168.100.11'
DESTB='192.168.100.13'

if [ "$1" = "$SCEA" ] 
then
	echo $1 '#### 실행 SCE-A'
	HOST='192.168.100.3'
elif [ "$1" = "$SCEB" ]
then
	echo $1 '#### 실행 SCE-B'
	HOST='192.168.100.4'
else
	echo 'Invalid parameter...'
	echo $0 'scea|sceb'
	exit 1
fi

ALIVE_CHK=`/usr/sbin/ping $HOST 1 | awk '{ print $3; }'`

if [ $ALIVE_CHK != "alive" ]; then
	echo "sce not alive";
	exit 1
fi

counter=0
#while [ $counter -lt 22 ]; do
#while true; do
snmpget -v 1 -c public $HOST \
PCUBE-SE-MIB::tpCpuUtilization.1.1 \
PCUBE-SE-MIB::tpCpuUtilization.1.2 \
PCUBE-SE-MIB::tpCpuUtilization.1.3 \
PCUBE-SE-MIB::tpFlowsCapacityUtilization.1.1 \
PCUBE-SE-MIB::tpFlowsCapacityUtilization.1.2 \
PCUBE-SE-MIB::tpFlowsCapacityUtilization.1.3 \
PCUBE-SE-MIB::tpServiceLoss.1.1 \
PCUBE-SE-MIB::tpServiceLoss.1.2 \
PCUBE-SE-MIB::tpServiceLoss.1.3 \
PCUBE-SE-MIB::diskNumUsedBytes.0 \
PCUBE-SE-MIB::diskNumFreeBytes.0 \
PCUBE-SE-MIB::sysOperationalStatus.0 \
PCUBE-SE-MIB::pchassisPowerSupplyAlarm.0 \
PCUBE-SE-MIB::pchassisFansAlarm.0 \
PCUBE-SE-MIB::pchassisTempAlarm.0 \
PCUBE-SE-MIB::pchassisVoltageAlarm.0 \
PCUBE-SE-MIB::subscribersNumIntroduced.1 \
PCUBE-SE-MIB::subscribersNumActive.1 \
PCUBE-SE-MIB::pmoduleOperStatus.1 \
PCUBE-SE-MIB::linkOperMode.1.1 \
PCUBE-SE-MIB::linkAdminModeOnFailure.1.1 \
PCUBE-SE-MIB::pportOperStatus.1.1 \
PCUBE-SE-MIB::pportOperStatus.1.2 \
PCUBE-SE-MIB::pportOperStatus.1.3 \
PCUBE-SE-MIB::pportOperStatus.1.4 \
PCUBE-SE-MIB::pportOperStatus.1.5 \
PCUBE-SE-MIB::pportOperStatus.1.6 \
PCUBE-SE-MIB::rdrFormatterDestStatus.$DESTA.33000 \
PCUBE-SE-MIB::rdrFormatterDestConnectionStatus.$DESTA.33000 \
PCUBE-SE-MIB::rdrFormatterDestStatus.$DESTB.33000 \
PCUBE-SE-MIB::rdrFormatterDestConnectionStatus.$DESTB.33000 \
PCUBE-SE-MIB::sysVersion.0 \
	
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
