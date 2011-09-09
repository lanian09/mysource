#!/usr/bin/bash
counter=0
while [ 1 ]; do
	counter=$(($counter+1))
	echo $counter;
	dis-sce.snmp 0;
	sleep 1
done;
