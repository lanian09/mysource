#!/usr/local/bin/expect -f

set side 	[lindex $argv 0]

set user	"admin"
set pass	"admin"
set timeout	10
set prompt	"sc>"

if {$side < 1} {
set host	"192.168.100.21"
} else {
set host	"192.168.100.22"
}

spawn 	/usr/bin/telnet $host
#set sid $spawn_id
#set pid [ exp_pid -i $spawn_id ]

expect 		"Please login:" {} timeout { exit 1 }
send 		"$user\r"
expect 		"Please Enter password:" {} timeout { exit 1 }
send 		"$pass\r"
expect 		"$prompt" {} timeout { exit 1 }

send 		"showenvironment\r"
expect 		"$prompt" {}
send 		"logout\r" 
send_user 	"logout\n"

close
wait
exit 0
