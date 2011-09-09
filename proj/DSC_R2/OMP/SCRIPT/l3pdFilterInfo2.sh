#!/usr/local/bin/expect -f

set HOST 		"192.168.100.6"
set USER 		"lgtdsc2"
set PASSWD 		"netoptics"
set PROMPT		"Net Optics>"
set timeout	3

#
#	SSH PROCEDURE
#
spawn ssh -T -oStrictHostKeyChecking=no customer@$HOST
expect "password:" { send "$PASSWD\r" } timeout { exit 1 }

#
#	LOGIN USER
#
expect "login user" { send "$USER\r" } timeout { exit 2 }
expect "password:" { send "$PASSWD\r" } timeout { exit 2 }

#
#	COMMAND
#
expect "$PROMPT" { send "filter list\r"	} timeout { exit 3 }

#
#	EXIT PROCEDURE
#
expect "$PROMPT" { send	"exit\r" } timeout { exit 0 }
expect "do you want to exit? (yes/no)" { send "yes\r" } timeout { exit 0 }
send_user "\ncomplete\n"

close
wait
exit 0
