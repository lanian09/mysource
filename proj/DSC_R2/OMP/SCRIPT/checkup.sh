#!/usr/bin/bash

clear
echo "====================================================="
echo " LGT DSC (DATA SERVICE CONTROLLER) SYSTEM            "
echo "   CONFORMANCE TEST TOOL (BATCH)                     "
echo "                                                     "
echo "                                        uPRESTO,Inc. "
echo "====================================================="
echo ""

dir=/DSC/SCRIPT/CONFORMANCE;
cnt=0;

for script in `ls $dir`; do
	cnt=$(($cnt+1));
	echo -n $cnt": ";
	$dir/$script;
done
echo ""
