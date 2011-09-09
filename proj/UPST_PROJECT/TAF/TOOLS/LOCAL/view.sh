#!/bin/sh

ps -ef | grep CAPD  | grep -v grep
ps -ef | grep PRE_A  | grep -v grep
ps -ef | grep A_TCP  | grep -v grep
ps -ef | grep A_HTTP  | grep -v grep
ps -ef | grep A_MEKUN  | grep -v grep
ps -ef | grep A_BREW  | grep -v grep
ps -ef | grep A_WIPI  | grep -v grep
ps -ef | grep A_VOD  | grep -v grep
ps -ef | grep A_CALL  | grep -v grep
ps -ef | grep CILOG  | grep -v grep
ps -ef | grep A_ONLINE  | grep -v grep

