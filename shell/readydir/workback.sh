#!/bin/bash
CMD_MV=`which mv`
CMD_CP=`which cp`

if [ $# != 1 ]
then
    echo needs param...
    exit
fi

PARAM=$1
if [ -e $PARAM ]
then
    if [ -d $PARAM ]
    then
        if [ -e $PARAM.OLD ]
        then
            echo "older directory moving..."
            $CMD_MV $PARAM.OLD $PARAM.OLD.`date +%Y%m%d%H%M%S`
        fi

        echo "old directory creating..."
        $CMD_MV $PARAM $PARAM.OLD

        echo "new directory changing..."
        $CMD_MV $PARAM.NEW $PARAM

        echo "new directory copying..."
        $CMD_CP -r $PARAM $PARAM.NEW
    fi
    echo needs directory
    exit
fi

echo needs existed-param

