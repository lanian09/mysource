#!/bin/bash
echo "TETS SSS"
echo "TEST BBB"
echo "\n TETS CCC"
exit
echo "TEST START"
echo "(Y/N)?"
read YORN
echo "answer="$YORN
if [ $YORN = "Y" -o $YORN = "y" ]
then
        echo "answer is YES"
else
        echo "answer is No..."
fi
echo "FIN."
