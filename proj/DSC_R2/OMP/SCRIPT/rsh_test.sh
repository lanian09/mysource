#!/usr/local/bin/expect -f

set side    [lindex $argv 0]

set SCMA_a "192.168.100.11"
set SCMA_b "192.168.100.12"
set SCMB_a "192.168.100.13"
set SCMB_b "192.168.100.14"
set Active "192.168.100.15"
set OMP_A  "192.168.100.10"
set OMP_B  "192.168.100.10"
set timeout	3

if {$side == $SCMA_a} {
	set host    "192.168.100.11"
} 

if {$side == $SCMA_b} {
	set host    "192.168.100.12"
} 

if {$side == $SCMB_a} {
	set host    "192.168.100.13"
}

if {$side == $SCMB_b} {
	set host    "192.168.100.14"
}

if {$side == $Active} {
	set host    "192.168.100.15"
}

if {$side == $OMP_A} {
	set host    "192.168.100.10"
}
if {$side == $OMP_B} {
	set host    "192.168.100.10"
}
#
#
#
spawn /usr/bin/rsh -l root $host /DSC/NEW/BIN/disprc
expect "TOTAL" { } timeout { exit 3 }
exit 0
