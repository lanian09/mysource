#!/bin/bash
if [ -z $1 ]
then
	echo "NULL..."
else
	echo $1
	echo "PARAM1 LEN="${#1}
fi

