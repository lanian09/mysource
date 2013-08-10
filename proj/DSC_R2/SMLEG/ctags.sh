#!/bin/sh

rm -rf tags
echo 'find . -name "*.[ch]" > cout'
echo 'ctags -a cout'
find . -name "*.[chl]" > cout
/bin/cat cout | grep -v "CVS" > cout2
/bin/cat cout2 | grep -v accels > cout3
/usr/local/bin/ctags -L cout3
cscope -i cout3 -p 4
