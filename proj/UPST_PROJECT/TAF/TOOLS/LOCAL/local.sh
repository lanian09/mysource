#!/bin/sh

cd ../../BIN

./PRE_A &
sleep 20

./A_TCP &
sleep 1

./A_HTTP &
sleep 1

./A_MEKUN &
sleep 1

./A_WIPI &
sleep 1

./A_BREW &
sleep 1

./A_VOD &
sleep 1

./A_CALL &
sleep 1

./CILOG &
sleep 1

./A_ONLINE &
sleep 1

./CAPD 192.168.233.129 2009 &
sleep 1
