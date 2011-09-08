#!/usr/bin/bash

export LANG="C"

HOST="211.254.95.88"
PASSWD="netoptics"

expect -c "
set $a = 0
spawn ssh -T -oStrictHostKeyChecking=no customer@$HOST
expect {
	password: {
                send \"$PASSWD\r\"
                exp_continue
                }
	user: {
                send \"admin\r\"
                exp_continue
                }
	(yes/no) {
                send \"yes\r\"
                exp_continue
                }
	Optics> {
                send \"filter list\r\"
                send \"logout\r\"
                exp_continue
                }
        }
        exit
"
exit
exit
