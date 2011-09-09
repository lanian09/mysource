#!/usr/bin/bash

L2A="l2a"
L2B="l2b"

if [ "$1" = "$L2A" ]
then
	HOST='192.168.100.1'
elif [ "$1" = "$L2B" ]
then
    HOST='192.168.100.2'
else
    echo 'Invalid parameter...'
    echo $0 'l2a|l2b'
    exit 1
fi

#IF-MIB::ifOperStatus.1 \
#IF-MIB::ifOperStatus.5001  \

#snmpget -v 1 -c LGT 192.168.100.1  \
snmpget -v 1 -c LGT $HOST \
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
CISCO-MEMORY-POOL-MIB::ciscoMemoryPoolFree.1 \
2>&1 | awk '{ print $4 }' | cut -d '(' -f 1
