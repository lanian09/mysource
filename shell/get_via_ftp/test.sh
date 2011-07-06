#!/bin/bash
HOST=211.254.95.161
USERID=uamyd
PASSWD=$1

#SAMPLES
FILES="DSC_NEW.tar.gz NMSIF.NEW222.tar.gz"

echo "getting process via ftp."
ftp -n $HOST << EOF
	user $USERID $PASSWD
	prompt
	b
	mget $FILES
	bye
EOF
echo "termintated get process via ftp."
