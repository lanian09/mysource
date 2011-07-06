#!/usr/bin/expect -f

# ex2
set timout 3600
#connection via sftp
#spawn ssh uamyd@211.254.95.248
spawn sftp uamyd@211.254.95.248

#logon - input password
expect "?*password:"
send "qweR!234\n"

expect "*sftp*"
send "cd test\n"

expect "*sftp*"
send "put TEST_S1.tar.gz\n"

expect "*sftp*"
send "exit\n"

interact

# ex1

#set password [lrange $argv 0 0]
#set ipaddr [lrange $argv 1 1]
#set timeout -1
## now connect to remote UNIX box (ipaddr) with given script to execute
#spawn ssh uamyd@$ipaddr
#match_max 100000
## Look for passwod prompt
#expect "*?assword:*"
## Send password aka $password
#send -- "$password\r"
## send blank line (\r) to make sure we get back to gui
#send -- "\r"

#expect eof


