#!/usr/bin/bash
# Shell script to copy all files recursively and upload them to
# remote FTP server (copy local all directories/tree to remote ftp server)
#
# If you want to use this script in cron then make sure you have
# file pointed by $AUTHFILE (see below) and add lines to it:
# host ftp.mycorp.com
# user myftpuser
# pass mypassword
#
# This is a free shell script under GNU GPL version 2.0 or above
# Copyright (C) 2005 nixCraft
# Feedback/comment/suggestions : http://cyberciti.biz/fb/
# -------------------------------------------------------------------------
# This script is part of nixCraft shell script collection (NSSC)
# Visit http://bash.cyberciti.biz/ for more information.
# -------------------------------------------------------------------------
 
FTP="/usr/sfw/bin/ncftpget"
CMD=""
AUTHFILE="/DATA/LOG_SCMA/.scmadnload"
 
RLEG_LOGDIR="/DSC/APPLOG/RLEG/"
RDRANA_LOGDIR="/DSC/APPLOG/RDRANA/"
SMPP_LOGDIR="/DSC/APPLOG/SMPP/"
localdir="/DATA/LOG_SCMA"
DATE=`/usr/bin/date -u '+%m%d'`		# korea time -9 hour (one day ago)

# use the file for auth
#CMD="$FTP -R -d ftplog.log -f $AUTHFILE $myf $localdir ${RLEG_LOGDIR}${DATE}"
CMD="$FTP -R -f $AUTHFILE $myf $localdir ${RLEG_LOGDIR}${DATE}"
$CMD
CMD="$FTP -R -f $AUTHFILE $myf $localdir ${RDRANA_LOGDIR}${DATE}"
$CMD
CMD="$FTP -R -f $AUTHFILE $myf $localdir ${SMPP_LOGDIR}${DATE}"
$CMD
