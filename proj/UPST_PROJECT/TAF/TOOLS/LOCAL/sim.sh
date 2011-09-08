#!/bin/sh

DEVICE='bond0'
INTERFACE=`/sbin/ifconfig $DEVICE | grep "inet addr" | cut -d":" -f2 | cut -d' ' -f1`
DAGBIN=../../TOOLS/SIM/dagconvert
DATA=../SIM/DATA
DEFECTCODE=$1
#PACKET=japan_${DEFECTCODE}.pcap
PACKET=china_${DEFECTCODE}.pcap

echo 'IP='$INTERFACE

#./dagconvert -i DATA/WICGS_test.cap -o a.cap  -T PCAP:ERF -X 1 -Z 100000000 -N $INTERFACE -P 2009 -M 2

echo $DAGBIN -i $DATA/$PACKET -o a.cap -T PCAP:ERF -X 1 -Z 10000 -N $INTERFACE -P 2009 -M 5
$DAGBIN -i $DATA/$PACKET -o a.cap -T PCAP:ERF -X 1 -Z 10000 -N $INTERFACE -P 2009 -M 5


exit
