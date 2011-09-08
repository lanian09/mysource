#!/bin/sh

ps -ef | grep PRE_A | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_TCP | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_HTTP | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_MEKUN | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_WIPI | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_BREW | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_VOD | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep A_CALL | grep -v grep | awk '{print $2}' | xargs kill -15
ps -ef | grep CILOG | grep -v grep | awk '{print $2}' | xargs kill -15
