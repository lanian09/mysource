#!/usr/bin/bash

host=192.168.100.15

RESULT=`/usr/sbin/ping $host 1 | awk '{ print $3; }'`

if [ $RESULT = "alive" ]; then
	/usr/bin/rsh -l root $host /opt/VRTSvcs/bin/hastatus -sum | /usr/bin/tr "[:lower:]" "[:upper:]" | egrep 'SCM_SG' | awk '{
		if($6=="ONLINE") 	printf "%d ", 1;
		if($6=="OFFLINE") 	printf "%d ", 0;
	}
	END {
		printf "\n"
	}'
else
	echo "0 0 ";
fi
