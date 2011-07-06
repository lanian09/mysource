#!/bin/bash
EPOCH=1000000000
DATE=$(perl -e ¡°require ¡®ctime.pl¡¯; print &ctime($EPOCH);¡±)
echo $DATE
