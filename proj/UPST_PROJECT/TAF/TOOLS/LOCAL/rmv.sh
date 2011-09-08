#!/bin/sh

# REMOVE SHM
ipcs -m
ipcs -m | sed -n '/0x/p' |  awk '{print $1}' | xargs ipcrm -M
# REMOVE MSGQ
ipcs -q
ipcs -q | sed -n '/0x/p' | awk '{print $1}' | xargs ipcrm -Q
# REMOVE SEMA
ipcs -s
ipcs -s | sed -n '/0x/p' | awk '{print $1}' | xargs ipcrm -S

ipcs -a

