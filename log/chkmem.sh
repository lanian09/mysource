#
# memory check shell script
# - written by uamyd
#
# e.g.)
# block time                    Kbytes RSS    Anon
# samd1 2011-02-28 11:54:53     9040   6208   628
# samd2 2011-02-28 11:54:56     9040   6208   628
# ...
#

LFN=chkmem.log

function printt()
{
        CDATE=`date +%Y-%m-%d' '%H:%M:%S`
        PN=$1
        PID=`ps -ef | grep $PN | grep -v grep | awk '{print $2}'`
        CHKMEM=`pmap -ax $PID | grep total | awk '{print $3 "   " $4 "   " $5}'`
        echo "$PN $CDATE        $CHKMEM" >> $LFN

}

echo "" >> $LFN
echo "block     time                    Kbytes RSS    Anon" >> $LFN
printt samd1
printt samd2
printt samd3
printt samd4
printt samd_new1
printt samd_new2
printt samd_new3
printt samd_new4
printt samd_new5
printt samd_new6
