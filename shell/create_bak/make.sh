#!/bin/bash
function bye(){
	echo ""
	echo "Shell Terminated"
	echo ""
	exit
}
function usage(){
	echo ""
	echo "usage)"
	echo ""
	echo "!! RUN IN SRC-DIRECTORY !!!!"
	echo "/>./make.sh [src-dir(r)] [dest-dir(a)] [remote-ip] [remote-dir(a)] [id] [pwd] [backup-name]"
	echo " (a) : absolute directory"
	echo " (r) : relative directory"
	echo " backup-name : ex) TXT -> TXT_bak2006.12.07.tar.gz"
	return
}
function makebackup(){
	#
	# make backup script
	#

	cd ..
	echo "Starting Create Backup.sh ..."
	SCRIPT=$DESTDIR"/.BAK_$BNAME.sh"

	# script start ------------------------------
	echo "#!/bin/bash" > $SCRIPT
	echo CDATE='`'/bin/date +%Y%m%d'`' >> $SCRIPT
	echo "cd $(pwd)/$SRCDIR" >> $SCRIPT
	echo "cd ../" >> $SCRIPT
	echo tar cvfz $DESTDIR/$BNAME"_BAK$"CDATE.tar.gz $SRCDIR >> $SCRIPT
	echo "cd $DESTDIR" >> $SCRIPT
	# script end --------------------------------

	/bin/chmod 755 $SCRIPT
	CMSGBAK=$SCRIPT

	echo "Success Create BAK_$BNAME.sh !"
	cat $SCRIPT
	answer
	return

}
function makeftp(){
	#
	# make ftp script
	#

	echo "Starting Create Ftp.sh ..."
	SCRIPT=$DESTDIR"/.FTP_$BNAME.sh"

	# script start ------------------------------
	echo "#!/bin/bash" > $SCRIPT
	echo "FTP()" >> $SCRIPT
	echo "{" >> $SCRIPT
	echo "(" >> $SCRIPT
	echo 	echo '"'user '$'FTP_USER '$'FTP_PASS'"' >> $SCRIPT
	echo 	echo '"'bin'"' >> $SCRIPT
	echo	echo '"'cd $RDIR'"' >> $SCRIPT
	echo	echo '"'put $BACKUP_FILE_FULL_NAME'"' >> $SCRIPT
	echo ")|" >> $SCRIPT
	echo ftp -n "$"BACKUP_HOST >> $SCRIPT
	echo "}" >> $SCRIPT
	echo "" >> $SCRIPT
	echo CDATE='`'/bin/date +%Y%m%d'`' >> $SCRIPT
	echo "FTP_USER=$ID" >> $SCRIPT
	echo "FTP_PASS=$PASS" >> $SCRIPT
	echo "BACKUP_HOST=$IP" >> $SCRIPT
	echo "" >> $SCRIPT
	echo "cd $DESTDIR" >> $SCRIPT
	echo "FTP" >> $SCRIPT
	# script end --------------------------------
	/bin/chmod 755 $SCRIPT
	CMSGFTP=$SCRIPT

	echo "Success Craete FTP_$BNAME.sh !"
	cat $SCRIPT
	answer
	return
	
}
function makeclean(){
	# 
	# make clean script
	#

	echo "Starting Create Clean.sh ..."
	SCRIPT=$DESTDIR"/.CLEAN_$BNAME.sh"

	# script start ------------------------------
	echo "#!/bin/bash" > $SCRIPT
	echo "cd $DESTDIR" >> $SCRIPT
	echo "/usr/bin/find $DESTDIR -ctime +7 -ls -exec rm -rf {} \;" >> $SCRIPT
	echo "exit" >> $SCRIPT
	# script end --------------------------------
	/bin/chmod 755 $SCRIPT
	CMSGCLEAN=$SCRIPT

	echo "Success Create CLEAN_$BNAME.sh !"
	cat $SCRIPT
	answer
	return
}
function makeclone(){
	#
	# make clone registry
	#

	SCRIPT=$DESTDIR"/clon_msg"
	echo "00 02 * * * $CMSGBAK" > $SCRIPT
	echo "20 02 * * * $CMSGFTP" >> $SCRIPT
	echo "40 02 * * * $CMSGCLEAN" >> $SCRIPT
	cat $SCRIPT
	answer
}
function chkrsc(){
	echo "SRCDIR   : $SRCDIR"
	echo "DESTDIR  : $DESTDIR"
	echo "REMOTEIP : $IP"
	echo "REMOTEDIR: $RDIR"
	echo "ID       : $ID"
	echo "PASSWORD : $PASS"
	echo "BACKUP NAME: $BNAME"
	answer
	return
}
function answer(){
	echo "Do you want confirm?(y/n)"
	read answer1
	if [ $answer1 != 'y' -a $answer1 != 'Y' ]
	then
		bye
	fi
}

CDATE=`/bin/date +%Y%m%d`
if [ $# -ne 7 ] 
then
	usage
	bye
else
	SRCDIR=$1
	DESTDIR=$2
	IP=$3
	RDIR=$4
	ID=$5
	PASS=$6
	BNAME=$7
	chkrsc
	BACKUP_FILE_FULL_NAME="$BNAME"_BAK"$"CDATE.tar.gz
	makebackup
	makeftp
	makeclean
	makeclone
fi

cd $DESTDIR
ls

bye




