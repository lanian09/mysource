#!/bin/bash

SRC=t.c
HSRC=hdr.h

rm -rf $SRC $HSRC a.out

if [ $# -eq 1 ]
then
	if [ $1 == "-c" ]
	then
		echo "cleaning file $SRC $HSRC a.out"
		exit
	fi
fi



## MAKE hdr.h
ls -al *.h | awk '{print "#include \""$9"\""}' > $HSRC

## MAKE t.c
echo "#include \"$HSRC\"" > $SRC
echo "" >> $SRC
echo "int main()" >> $SRC
echo "{" >> $SRC
echo "	printf(\"header file test\n\");" >> $SRC
echo "}" >> $SRC

cc $SRC -I.
./a.out


