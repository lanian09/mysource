#!/usr/local/bin/expect -f

set side 	[lindex $argv 0]
set cmd 	[lindex $argv 1]

set pass	"netoptics"
set timeout	10
set prompt	"Net Optics>"

if {$side < 1} {
set host	"192.168.100.5"
set user	"lgtdsc1"
} else {
set host	"192.168.100.6"
set user	"lgtdsc2"
}

spawn ssh -T -oStrictHostKeyChecking=no customer@$host
#set sid $spawn_id
#set pid [ exp_pid -i $spawn_id ]

stty -echo
expect "password:" 	{ send "$pass\r" } timeout { exit 1 }
expect "login user" { send "$user\r" } timeout { exit 2 }
expect "password:" 	{ send "$pass\r" } timeout { exit 2 }

stty echo
if {$cmd == "filter"} {
expect "$prompt" { send "filter list\r" } timeout { exit 3 }
} else {
expect "$prompt" { send "port show\r" } timeout { exit 3 }
}
stty -echo

#
#   EXIT PROCEDURE
#
expect "$prompt" { send "exit\r" } timeout { exit 0 }                                        
expect "do you want to exit? (yes/no)" { send "yes\r" } timeout { exit 0 }                   
send_user "yes\n"                                                                     
                                                                                             
close                                                                                        
wait                                                                                         
exit 0                                                                                       
