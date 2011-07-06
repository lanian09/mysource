#!/bin/bash
if [ -z $1 ]
then
	echo "PARAM NO"
	v=true
else
	echo "PARAM $1"
	v=false
fi

if $v
then
	echo "true !!"
fi

