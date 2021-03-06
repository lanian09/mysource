#!/bin/bash
function Recovery(){
	exit
}

function SubBlock(){
	CMD=$1
	shift
	if [ $TOTALFLAG -eq 1 ]; then
		#total
		#echo "$CMD -v"
		$CMD -v
	else
		CNT=$AC
		for var in $1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15} ${16} ${17} ${18} ${19} ${20}
		do
			if [ $CNT -ne 0 ]; then
				#echo "$CMD -b $var"
				$CMD -b $var
			fi
			CNT=$(($CNT -1))
		done
	fi
}

function Apply(){
	echo "Apply Package..."
	cd $INSTDIR
	# Kill Block
	SubBlock "./KillMC" $@
	DisMC

	# Copy Block
	gzip -fd $PKGDIR/$PKGFN
	#echo tar -xvf $PKGDIR/${PKGFN%.*}
	tar -xvf $PKGDIR/${PKGFN%.*}
	
	# Start Block
	SubBlock "./StartMC" $@
	DisMC

} 

function chkdir(){
	SUBDIR=$1
	if [ -e $SUBDIR ]
	then
		if [ -d $SUBDIR ]
		then
			return
		else 
			echo "[INFO]$SUBDIR is NOT DIRECTORY!!!!"
			echo "[INFO]move existed $SUBDIR file > $SUBDIR.$CDATE"
			mv $SUBDIR $SUBDIR.$CDATE
		fi
	else
		echo "[INFO]$SUBDIR is NOT EXIST"
	fi
	# make OLD directory
	echo "Creating $INSTDIR/OLD Directory"
	mkdir $SUBDIR 
}

function Backup(){
	echo "Backup Starting..."
	cd $INSTDIR

	if [ $TOTALFLAG -eq 1 ]; then
		#total
		cd ../
		cp -r BIN BIN.$CDATE
	else
		CNT=$AC
		chkdir "../OLD"
		for var in $1 $2 $3 $4 $5 $6 $7 $8 $9 ${10} ${11} ${12} ${13} ${14} ${15} ${16} ${17} ${18} ${19} ${20}
		do
			if [ $CNT -ne 0 ]; then
				cp $var ../OLD/$var.$CDATE
			fi
			CNT=$(($CNT -1))
		done
	fi
	echo "Backup Terminated"
	ls $INSTDIR/../OLD
}

function Ready(){
	echo "Ready to Installation Package..."

	if [ $SYSTYPE -eq 1 -o $SYSTYPE -eq 3 ];then
		if [ $SYSTYPE -eq 1 ]; then
			# WGTAM
			INSTDIR="/WGTAMAPP/BIN"
		elif [ $SYSTYPE -eq 3 ]; then
			# GTAM
			INSTDIR="/gtamapp/BIN"
		fi
		PKGDIR="$PKGDIR"
		echo "Installation Target Directory is $INSTDIR"
		confirm
		cd $PKGDIR
	elif [ $SYSTYPE -eq 2 -o $SYSTYPE -eq 4 ]; then
		if [ $SYSTYPE -eq 2 ]; then
			# WNTAM
			INSTDIR="/WNTAMAPP/BIN"
		elif [ $SYSTYPE -eq 4 ]; then
			# NTAM
			INSTDIR="/tamapp/BIN"
		fi
		echo "Installation Target Directory is $INSTDIR"
		confirm
		cd /home
		ftp_ntam
		PKGDIR="/home"
		
	fi
	ls
}

function ftp_ntam(){
	(
		echo "user $USERID $PASSWD"
		echo "cd $PKGDIR"
		echo "binary"
		echo "prompt"
		echo "ls"
		echo "hash"
		echo "mget $PKGFN"
		echo "quit"
	)|
	ftp -n $SVRIP
}

function usage(){
	echo "usage)"
	echo ">./apppack.sh [A] [B] [C] [D] [E] [F] [G]+"
	echo " - A : Server IP, xxx.xxx.xxx.xxx "
	echo " - B : User ID"
	echo " - C : Password"
	echo " - D : PKG Stored Directory !! ABSOLUTE !! Path in GTAM. "
	echo "       Server-side PKG Stored Directory !! RELATIVE !! Path in NTAM"
	echo " - E : Package File Name. File extention is MUST '.tar.gz' and '.' not include. ex) xx~x.tar.gz"
	echo " - F : System Type. (1:WGTAM, 2:WNTAM, 3:GTAM, 4:NTAM)"
	echo " - G+: Block List to Install. if you want to restart all-block, write 'TOTAL'"
	echo "       restart individual, write block name e.g. STAT_HOUR STAT_BTS ALMD"
	echo ""
}

function bye(){
	echo ""
	echo "Terminated Installation Package ..."
	echo "bye"
	echo ""
	exit
}

function confirm(){
    echo ""
    echo "Do you want to stop Installation?(y/n)"
    read answer
    if [ $answer = 'y' -o $answer = 'Y' ]; then
	bye
    fi
}

function chkparam(){
	echo "Server IP : $SVRIP"
	echo "User ID   : $USERID"
	echo "Password  : $PASSWD"
	echo "Directory : $PKGDIR"
	echo "PKG FILE  : $PKGFN"
	echo "SYSTEM    : $SYSTYPE"
	if [ $BLOCK = "TOTAL" -o $BLOCK = "Total" -o $BLOCK = "total" ]; then
		echo "Block List: Whole Block"
		TOTALFLAG=1
	else
		echo "Block List: "$@
	fi
}

#
# main --------------------------------------------------
#

CDATE=`/bin/date +%Y%m%d`
TOTALFLAG=0
PMAX=7
MAX_BLOCK_CNT=20
PARAM_CNT=$#

echo "Starting Installation Package ... "
echo ""

#
# step 1. check parameter count, min = 7
#
if [ $PARAM_CNT -lt $PMAX ]; then
	usage
	exit
fi

SVRIP=$1
USERID=$2
PASSWD=$3
PKGDIR=$4
PKGFN=$5
SYSTYPE=$6
BLOCK=$7

#
# step 2. check package file type( extention is must 'tar.gz' )
#
if [ ${PKGFN#*.} != "tar.gz" ]; then
	echo "[ERROR] PKG file name is MUST with '.tar.gz'"
	exit
fi

#
# step 3. check system type( only 4, WGTAM, WNTAM, GTAM, NTAM )
#
if [ $SYSTYPE != "WGTAM" -a $SYSTYPE != "wgtam" -a $SYSTYPE != "WNTAM" -a $SYSTYPE != "wntam" -a $SYSTYPE != "GTAM" -a $SYSTYPE != "gtam" -a $SYSTYPE != "NTAM" -a $SYSTYPE != "ntam" ]; then
	echo "[ERROR] System type is MUST WGTAM/WNTAM/GTAM/NTAM :Param[$SYSTYPE]"
fi

if [ $SYSTYPE = "WGTAM" -o $SYSTYPE = "wgtam" ]; then
	SYSTYPE=1
elif [ $SYSTYPE = "WNTAM" -o $SYSTYPE = "wntam" ]; then
	SYSTYPE=2
elif [ $SYSTYPE = "GTAM" -o $SYSTYPE = "gtam" ]; then
	SYSTYPE=3
elif [ $SYSTYPE = "NTAM" -o $SYSTYPE = "ntam" ]; then
	SYSTYPE=4
fi

#
# step 4. check block list
#
if [ $BLOCK = "TOTAL" -o $BLOCK = "Total" -o $BLOCK = "total" ]; then
	echo "Whole Process Restart..."
else
	for var in $1 $2 $3 $4 $5 $6 
	do 
		shift
	done
	AC=$#
	#
	# step 4.1 check block count : MAX BLOCK COUNT is 20
	#
	if [ $AC -gt $MAX_BLOCK_CNT -o $AC -lt 1 ]; then
		echo "[ERROR]Block Count is UNVALID( MAX:$MAX_BLOCK_CNT, Block-List:$AC)"
	    exit
	fi

fi 


#
# step 5. Check Parameter
#
chkparam $@

#
# step 6. Ready to Installation Package
#
Ready
Backup $@
Apply $@


bye

