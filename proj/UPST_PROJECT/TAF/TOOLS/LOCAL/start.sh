#!/bin/sh

cd /AQUA/BIN

./PRE_A &
sleep 10

./A_TCP &
sleep 1

./A_HTTP &
sleep 1

./A_MEKUN &
sleep 1

./A_WIPI &
sleep 1

./A_VOD &
sleep 1

./A_CALL &
sleep 1

./CILOG &
sleep 1

./A_ONLINE &
sleep 1

./CAPD &
sleep 1
