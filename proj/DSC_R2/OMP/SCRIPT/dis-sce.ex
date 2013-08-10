#!/usr/local/bin/expect -f
set side 	[lindex $argv 0]
set pass	"cisco"
set timeout	10

if {$side < 1} {
set host	"192.168.100.3"
set prompt	"SCE2020_A>"
set prompt1	"SCE2020_A#>"
set prompt2	"SCE2020_A(config)#>"
set prompt3	"SCE2020_A(config if)#>"
} else {
set host	"192.168.100.4"
set prompt	"SCE2020"
set prompt	"SCE2020_B>"
set prompt1	"SCE2020_B#>"
set prompt2	"SCE2020_B(config)#>"
set prompt3	"SCE2020_B(config if)#>"
}

spawn 	/usr/sbin/ping $host 1
expect "no" { exit 1 }

spawn 	/usr/bin/telnet $host

expect 		"Password:" {} timeout { exit 1 }
send 		"$pass\r"
expect 		"$prompt" {} timeout { exit 1 }
send 		"en 15\r"
expect 		"Password:" {} timeout { exit 1 }
send 		"$pass\r"

expect 		"$prompt1" {}
send 		"show failure-recovery operation-mode\r"
expect 		"$prompt1" {}
send 		"show interface Linecard 0 link mode\r"
expect 		"$prompt1" {}
send 		"logout\r" 
expect 		"Are you sure?" {}
send 		"y\r"
send_user 	"y\n"

close
wait
exit 0
