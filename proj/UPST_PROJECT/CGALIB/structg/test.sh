#!/bin/sh
#
# $Id: test.sh,v 1.1.1.1 2011/08/29 05:56:44 dcham Exp $
#

export LANG=C
rm -rf TEST
mkdir TEST
cd TEST

cvs co memg
cd memg
cp -f ../../* .
../../structg.pl ./memg.stg OUTPUT
echo $?
cd OUTPUT
make
if(test $? != 0)
then
	echo "ERROR memg"
	exit
fi
cd ../..

cvs co hashg/hashg.stg
cd hashg
cp -f ../../* .
../../structg.pl ./hashg.stg  OUTPUT
echo $?
cd OUTPUT
make
if(test $? != 0)
then
	echo "ERROR hashg"
	exit
fi
cd ../..


cvs co hasho/hasho.stg
cd hasho
cp -f ../../* .
../../structg.pl ./hasho.stg OUTPUT
echo $?
cd OUTPUT
make
if(test $? != 0)
then
	echo "ERROR hasho"
	exit
fi
cd ../..

cvs co timerN/timerN.stg
cd timerN
cp -f ../../* .
../../structg.pl ./timerN.stg  OUTPUT
echo $?
cd OUTPUT
make
if(test $? != 0)
then
	echo "ERROR timerN"
	exit
fi
cd ../..
  
cvs co mems/mems.stg
cd timerN
cp -f ../../* .
../../structg.pl ./mems.stg  OUTPUT
echo $?
cd OUTPUT
make
if(test $? != 0)
then
	echo "ERROR timerN"
	exit
fi
cd ../..
  
cvs co nifo/nifo.stg
cd timerN
cp -f ../../* .
../../structg.pl ./nifo.stg  OUTPUT
echo $?
cd OUTPUT
make
if(test $? != 0)
then
	echo "ERROR timerN"
	exit
fi
cd ../..
  
  
echo "SUCCESS ALL"

#   
# $Log: test.sh,v $
# Revision 1.1.1.1  2011/08/29 05:56:44  dcham
# NEW OAM SYSTEM
#
# Revision 1.1  2011/08/19 04:25:48  uamyd
# CGALIB moved in DQMS
#
# Revision 1.1  2011/08/03 06:02:44  uamyd
# CGA, HASHO, TIMERN library added
#
# Revision 1.2  2011/01/11 04:09:04  uamyd
# modified
#
# Revision 1.1.1.1  2010/08/23 01:13:06  uamyd
# DQMS With TOTMON, 2nd-import
#
# Revision 1.1  2009/06/10 16:45:50  dqms
# *** empty log message ***
#
# Revision 1.1.1.1  2009/05/26 02:13:20  dqms
# Init TAF_RPPI
#
# Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
# WATAS3 PROJECT START
#
# Revision 1.1  2007/08/21 12:22:39  dark264sh
# no message
#
# Revision 1.6  2006/11/10 06:01:38  cjlee
# clex.stc
#
# Revision 1.5  2006/05/29 05:15:22  cjlee
# *** empty log message ***
#
# Revision 1.4  2006/05/25 06:30:35  cjlee
# logtest
#
# Revision 1.3  2006/05/25 06:29:41  cjlee
# *** empty log message ***
#
#
