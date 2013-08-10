#!/usr/bin/bash

TAPA='tapa'
TAPB='tapb'

if [ "$1" = "$TAPA" ]
then
    echo $1 '#### 실행 TAP-A'
	HOST='192.168.100.5'
elif [ "$1" = "$TAPB" ]
then
    echo $1 '#### 실행 TAP-B'
    HOST='192.168.100.6'
else
    echo 'Invalid parameter...'
    echo $0 'tapa|tapb'
    exit 1
fi


counter=0
#while [ $counter -lt 22 ]; do
#while true; do
snmpget -v 1 -c public $HOST  \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.32 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.33 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.34 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.35 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.36 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.37 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.38 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.39 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.40 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.41 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.42 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.43 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.12 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.13 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.14 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.15 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.16 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.17 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.18 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.19 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.20 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.21 \ 
SNMPv2-SMI::enterprises.23022.2.8.1.1.12.1.2.0.1 \
SNMPv2-SMI::enterprises.23022.2.8.1.1.12.1.2.0.2 2>&1 | awk '{ print $3, $4 }'
#SNMPv2-SMI::enterprises.23022.2.8.1.1.6.1.4.0.21 2>&1 | awk '{ print $4 }' | cut -d '(' -f 1 
#echo  `date` ' [Count: ' $counter ' ] ============================='
#let counter=counter+1;
#sleep 1
#done
#done
