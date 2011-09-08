include ../../MkConfig.mk

TOP_LIB		= ../../lib
TOP_BIN		= ../../bin
#LIBRARY 	= -L../../libsrc/cgalib/lib -L../../lib -L../../libsrc/SM_API/sm-c-api-3.0.6_B35/solaris -L$(TOP_LIB)
LIBRARY 	= -L$(TOP_LIB)

#INCLUDES	= -I./ -I../../include -I../../libsrc/SM_API/include -I../../libsrc/SM_API/include/system -I../../libsrc/COMM
INCLUDES	= -I./ -I../../include -I../../libsrc/SM_API/include -I../../libsrc/COMM
#INCLUDES	= -I../../libsrc/cgalib/inc -I../../include -I./ -I../../libsrc/SM_API/include -I../../libsrc/SM_API/include/system -I../../libsrc/COMM
#####################################################################################
# DEFINES INFO
# 	-D_REENTRANT	: mandatory field (thread 사용시 포함)
# 	-DDEBUG 		: debugging info print
# 	-D_LOGIN 		: login info print
# 	-D_LOGOUT		: logout info print
# 	-DCPS   		: call per seconds print
# 	-DSTATISTIC 	: account & login 5min statistics print
# 	-DSESS_CNT  	: session node count print
# 	-DSM_SESS_CNT_PRT : SM session node count print
#	-DSM_SESS		: SM SESS
#	-D_SM_CONN_CHECK: message 마다 sm connection check 여부를 결정
#	-D_SM_CONN_FIND : message 처리 전 sm connection 이 끊어진 경우 연결된 connection 을 찾고 그 연결로 message 처리.
#	-DHBIT			: hbit test
#	-DSM_MUTEX		: SM API Response가 동시에 응답하는 경우 제어하기 위해 사용.
#	-DFUNC_TIME_CHECK: Function Performance Measurement
#####################################################################################

#DEFINES		= -D_REENTRANT -D_LOGIN -D_LOGOUT -DDISCONN_REQ -DSM_MUTEX -DFUNC_TIME_CHECK
DEFINES		= -D_REENTRANT -DPRT_LOGIN -DPRT_LOGOUT #-DPRT_SM_SESS_CNT

CFLAGS		= -g -Wall $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)

#SRCS		= leg_init.c leg_main.c leg_msgq.c leg_radius.c leg_sce_comm.c leg_thread.c mmc_hld.c mmc_proc.c leg_utill.c tmo.c
#SRCS        += leg_session.c
SRCS		= leg_init.c leg_main.c leg_radius.c leg_sce_comm.c leg_mmc_proc.c leg_sm_sess.c leg_tmo.c

TARGET		= RLEG0

GET			= sccs
OBJS		= $(SRCS:.c=.o)

#LIBS		= $(LIBRARY) -lIPC -lLogUtil -lcomm -lsmapi -lpthread -lsocket -lnsl -lInitShm -lHASHO -ltimerN -lhashg -lSTGtimerN -lSTGhashg
LIBS		= $(LIBRARY) -lIPC -lLogUtil -lcomm -lsmapi -lpthread -lsocket -lnsl -lInitShm -lHASHO -ltimerN -lhashg 

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

all: $(TARGET) 

$(SRCS):
#	$(GET) -s $@

$(TARGET): $(OBJS)
	echo $(LDFLAGS)
	$(CC) -mt  $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	cp $(TARGET) $(TOP_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
#	cp $(TARGET) $(TOP_BIN)


clean:
	rm -f *.o RLEG* core*
#	sccs clean

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
