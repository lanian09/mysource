#!/bin/bash
LIST=`cat /home/uamyd/WNTAS2/WNTAM_GNGI/DATA/McInit | grep -v "##" | grep -v "#E" | grep "#@" | awk '{print $2}'`
echo $LIST
