#!/bin/bash

COUNT=0
ELEMENT=$1

if [ "${ELEMENT}" = "0" ]
then
    EQUIP=SWITCH1
    HOST=192.168.0.251
elif [ "${ELEMENT}" = "1" ]
then
    EQUIP=SWITCH2
    HOST=192.168.0.252
else
    exit -1
fi

Timer()
{
    ${PROCESS} &
    PROC_PID=`echo $!`

    while [ -d /proc/${PROC_PID} ]
    do
        COUNT=$(( ${COUNT} + 1 ))
        if [ ${COUNT} = 50 ]
        then
            kill -SIGKILL ${PROC_PID}
            exit -1
        else
            sleep 1
        fi
    done
}

PROCESS="snmpget -t 20 -v 1 -c DQMS $HOST \
IF-MIB::ifOperStatus.10101 \
IF-MIB::ifOperStatus.10102 \
IF-MIB::ifOperStatus.10103 \
IF-MIB::ifOperStatus.10104 \
IF-MIB::ifOperStatus.10105 \
IF-MIB::ifOperStatus.10106 \
IF-MIB::ifOperStatus.10107 \
IF-MIB::ifOperStatus.10108 \
IF-MIB::ifOperStatus.10109 \
IF-MIB::ifOperStatus.10110 \
IF-MIB::ifOperStatus.10111 \
IF-MIB::ifOperStatus.10112 \
IF-MIB::ifOperStatus.10113 \
IF-MIB::ifOperStatus.10114 \
IF-MIB::ifOperStatus.10115 \
IF-MIB::ifOperStatus.10116 \
IF-MIB::ifOperStatus.10117 \
IF-MIB::ifOperStatus.10118 \
IF-MIB::ifOperStatus.10119 \
IF-MIB::ifOperStatus.10120 \
IF-MIB::ifOperStatus.10121 \
IF-MIB::ifOperStatus.10122 \
IF-MIB::ifOperStatus.10123 \
IF-MIB::ifOperStatus.10124 \
enterprises.9.2.1.56.0 \
enterprises.9.2.1.57.0 \
enterprises.9.2.1.58.0 \
CISCO-MEMORY-POOL-MIB::ciscoMemoryPoolUsed.1 \
CISCO-MEMORY-POOL-MIB::ciscoMemoryPoolFree.1"

Timer | awk '{ print $4 }' | cut -d '(' -f 1
