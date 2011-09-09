#!/usr/bin/sh
if [ $2 = "port" ]; then
	/DSC/SCRIPT/dis-tap.ex $1 $2 | egrep 'n2\.0[1-4]|m\.0[1-9]|m\.10' \
		| sed 's/up/1/g' | sed 's/dwn/0/g' | /usr/bin/tr "[:lower:]" "[:upper:]"  | /usr/bin/awk '
	{ printf "%s ", $3; } 
	END { printf "\n"; }'
else
	/DSC/SCRIPT/dis-tap.ex $1 $2 | egrep '^Filter|in_ports|ip_protocol|l4_src_port|redir_ports' \
		| /usr/bin/tr "[:lower:]" "[:upper:]" | sed 's/N2\.//g' | sed 's/M\.//g' | sed 's/\/65535//g' | sed 's/\/0000//g' \
		| sed 's/_PORTS//g' | sed 's/_PORT//g'  | sed 's/IP_//g' | sed 's/L4_SRC=0000,//g' | sed 's/VLAN=0000,//g' \
		| sed 's/,ACTION=REDIR//g'
fi
