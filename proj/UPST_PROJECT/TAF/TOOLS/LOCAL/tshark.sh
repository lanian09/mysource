#!/bin/sh

FILE=`$1`

tshark -r ../SIM/DATA/$FILE -R "sip.Method == NITIFY" | wc
