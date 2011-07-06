# 
# memory check shell script
# - written by uamyd
#
# e.g.)
# [SAMD MEMORY CHECK]
# time                    Kbytes RSS    Anon
# 2011-02-28 11:54:53     9040   6208   628
# 2011-02-28 11:54:56     9040   6208   628
# ...
#

LFN=SAMD_MEM.LOG

function printt()
{
	CDATE=`date +%Y-%m-%d' '%H:%M:%S`
	PN=$1
	PID=`ps -ef | grep $PN | grep -v grep | awk '{print $2}'`
	CHKMEM=`pmap -ax $PID | grep total | awk '{print $3 "   " $4 "   " $5}'`
	echo "$CDATE	$CHKMEM"
	
}


echo "" > $LFN
echo "[SAMD MEMORY CHECK]" >> $LFN
echo "time                    Kbytes RSS    Anon" >> $LFN

while [ 1 ]
do
	printt samd >> $LFN
	sleep 3600
done

echo ""

