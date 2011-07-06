# memory check shell script
# - written by uamyd
#
# e.g.)
# [2011-02-26 10:12:00]
# block :           Kbytes     RSS    Anon
# samd1 : totla kb  7440    4564    1736
# samd2 : totla kb  7440    4564    1736
# samd3 : totla kb  7440    4564    1736
# samd4 : totla kb  7440    4564    1736
#
# ...
#

CDATE=`date +%Y-%m-%d' '%H:%M:%S`

function printt()
{
    PN=$1
    PID=`ps -ef | grep $PN | grep -v grep | awk '{print $2}'`
    CHKMEM=`pmap -ax $PID | grep total`
    echo "$PN : $CHKMEM"

}


echo ""
echo [$CDATE]
echo "block : (unit)    Kbytes     RSS    Anon  Locked"

printt samd1

echo ""
