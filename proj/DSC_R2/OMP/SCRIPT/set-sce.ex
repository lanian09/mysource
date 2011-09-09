#!/usr/local/bin/expect -f
set side 	[lindex $argv 0]
set command	[lindex $argv 1]
set mode	[lindex $argv 2]

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
set prompt	"SCE2020_B>"
set prompt1	"SCE2020_B#>"
set prompt2	"SCE2020_B(config)#>"
set prompt3	"SCE2020_B(config if)#>"
}

spawn   /usr/sbin/ping $host 1
expect "no" { exit 1 }

spawn /usr/bin/telnet $host
#set pid [ exp_pid -i $spawn_id ]

expect 	"Password:" {} timeout { exit 1 }
send 	"$pass\r"
expect 	"$prompt" {} timeout { exit 1 }
send 	"en 15\r"
expect 	"Password:" {} timeout { exit 1 }
send 	"$pass\r"
expect  "$prompt1" {}                                                                        

if {$command == "reload"} {
	send    	"reload\r"                                                                           
	expect  	"Are you sure?" {}                                                                   
	send    	"y\r"                                                                                
	send_user 	"y\n"                                                                              

	close
	wait 
	exit 0

} elseif {$command == "recovery"} {
	send    	"configure\r"                                                                        
	expect  	"$prompt2" {}                                                                        
	send    	"failure-recovery operation-mode $mode\r"                                            
	expect 		"$prompt2" {}

} elseif {$command == "link"} {
	send 		"configure\r"
	expect 		"$prompt2" {}
	send 		"interface LineCard 0\r"
	expect 		"$prompt3" {}
	send 		"link mode all-links $mode\r"

	if {$mode == "cutoff"} {
		expect  	"Changing the link mode to cutoff will cut the line - do you want to continue?"
		send 		"y\r"
		send_user 	"y\n"
	}
	expect 	"$prompt3" {}

} elseif {$command == "onfail"} {
	send    	"configure\r"                                                                        
	expect  	"$prompt2" {}                                                                        
	send    	"interface LineCard 0\r"                                                             
	expect  	"$prompt3" {}                                                                        
	send    	"shutdown\r"                                                                         
	expect  	"$prompt3" {}                                                                        
	send    	"connection-mode inline on-failure $mode\r"                                          
	expect  	"$prompt3" {}                                                                        
	send    	"no shutdown\r"                                                                      
	expect 		"$prompt3" {}
}
send 		"do logout\r" 
expect 		"Are you sure?" {}
send 		"y\r"
send_user 	"y\n"

close
wait 
exit 0
