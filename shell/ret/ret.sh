#!/bin/bash
ret=
ret_msg=30
function multi()
{
	echo "inner:$1"
	n=$1
	ret=$((n*2))
	return $ret_msg
}

a=$1
ret=$a

multi $a
return_val=$?
echo $return_val

