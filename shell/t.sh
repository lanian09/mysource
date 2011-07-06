#!/bin/bash
TMP=/tmp/sssss.sh
echo "TMP="$TMP
echo "echo XXXXX > $TMP"
echo "XXXXXX" > $TMP
cp $TMP ${TMP:5}
exit
