BTIME=`cat /proc/stat | grep btime | awk '{print $2}'`
PROCS=`ls -al /proc | awk '{print $9}' | grep "^[0-9]*$"`

#RESULT_TIME=`perl -e '$cdate=localtime($BTIME);print "$cdate\n";'`
#RESULT_TIME=`perl -e "print scalar(localtime($BTIME))"`
RUN_PROC=`./DisMC | awk '{print $1}'`

for i in $PROCS
do
    if [ ! -e /proc/$i/cmdline ]
    then
        continue
    fi
    PN=`cat /proc/$i/cmdline`

    if [ ! -e /proc/$i/stat ]
    then
        continue
    fi

    for ii in $RUN_PROC
    do
        if [ "$ii" == "$PN" ]
        then

            PROC_TIME=`cat /proc/$i/stat | awk '{print $22}'`
            PROC_TIME=$((PROC_TIME/100))
            _OPT=`cat /proc/$i/stat | awk '{print $3" "$4" "$35" "$36" "$37}'`
            CHKTIME=$((PROC_TIME+BTIME))
            RESULT_TIME=`perl -e "print scalar(localtime($CHKTIME))"`
            echo "$PN($i) : $RESULT_TIME ( OPT=$_OPT, BTIME=$BTIME, START_JIFFIES=$PROC_TIME )"
        fi
    done
done
