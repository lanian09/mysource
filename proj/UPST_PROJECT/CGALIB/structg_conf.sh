#!/bin/sh

#/bin/cp -f ../../INC/clisto.h ./
#/bin/cp -f ../../INC/mems.h ./
#/bin/cp -f ../../INC/nifo.h ./
#/bin/cp -f ../../INC/hasho.h ./
#/bin/cp -f ../../INC/hashg.h ./
#/bin/cp -f ../../INC/timerN.h ./

/bin/cp -f BODY.stc ./structg
/bin/cp -f LOG_member_Get_func.stc ./structg
/bin/cp -f flow.stc ./structg
/bin/cp -f flow_dot.stcI ./structg
cd structg
perl pstg.pl ../common_stg.pstg
perl structg.pl ../common_stg.stg PRE
cd PRE
make
if(test $? != 0)
then
    echo "ERROR structg"
    exit    1
fi
cd ../
cd ../

#cp -f *.h ../../INC/
#cp -f *.a ../../LIB/
#cp -r *.h ../TAF_RPPI/INC
#cp -r *.a ../TAF_RPPI/LIB
#cp -r *.h ../TAM_APP/INC
#cp -r *.a ../TAM_APP/LIB
#cp -r *.h ../TOTMON_APP/INC
#cp -r *.a ../TOTMON_APP/LIB
