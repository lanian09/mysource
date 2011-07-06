#!/bin/bash

DEFAULT_PARAM='DDD FFF'

function chk_dir(){
	echo "count : $#"
	NEW_PARAM=$@
}

if [ $# != 0 ]
then
	echo 'none-zero param'
	i=0
	for p in $@
	do
		echo "param [$i:$p]"
		i=$((i+1))
		if [ -e $p ]
		then
			if [ -d $p ]
			then
				echo 'directory ok'
			else
				echo 'directory NOK'
			fi
		else
			echo 'directory is NOT EIXST'
		fi
	done
	chk_dir $@
	PARAM=$NEW_PARAM
else
	echo "param default"
	PARAM=$DEFAULT_PARAM
fi

echo "param=$PARAM"
