#!/usr/bin/bash
cnt=0
while [ 1 ] ; do
	cnt=$(($cnt+1));
	echo -n $cnt" ";
	dis-scm.sh 0 | /usr/bin/tr "[:lower:]" "[:upper:]" | egrep '^PS|^FT0' | awk '{
		if(index($2, "OK")<=0) printf "A %10s%10s\n",$1,$2;
	}'
	dis-scm.sh 1 | /usr/bin/tr "[:lower:]" "[:upper:]" | egrep '^PS|^FT0' | awk '{
		if(index($2, "OK")<=0) printf "A %10s%10s\n",$1,$2;
	}'
	echo  ""
	sleep 1
done;
