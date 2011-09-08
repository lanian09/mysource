include ../../MkConfig.mk

TOP_LIB		= ../../lib
TOP_BIN		= ../../bin
LIBRARY 	= -L$(TOP_LIB)

INCLUDES	= -I./ -I../../include  -I../../libsrc/COMM
#####################################################################################
# DEFINES INFO
# 	-D_REENTRANT	: mandatory field (thread 사용시 포함)
# 	-DDEBUG 		: debugging info print
# 	-D_LOGIN 		: login info print
# 	-D_LOGOUT		: logout info print�
# 	-DCPS   		: call per seconds print
# 	-DSTATISTIC 	: account & login 5min statistics print
# 	-DSESS_CNT  	: session node count print
# 	-DPROPERTY_NULL	: SCE login 시 property를 사용하지 않음.
# 	-DPROPERTY_ONE 	: SCE login 시 property를 1개 사용 (packageid)
#	-D_SM_CONN_CHECK: message 마다 sm connection check 여부를 결정
#####################################################################################

DEFINES		= 

CFLAGS		=  $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)

SRCS		= 	conflib.c \
				conflib.h \
				mmc_hld.c \
				mmc_hld.h \
				mmc_proc.c \
				rcapd.h \
				rcapd_file.c \
				rcapd_file.h \
				rcapd_init.c \
				rcapd_main.c \
				rcapd_msgq.c \
				rcapd_proc.c \
				rcapd_util.c

TARGET		= RDRCAPD
#TARGET		= RCAPD

GET			= sccs
OBJS		= $(SRCS:.c=.o)

LIBS		= $(LIBRARY) -lIPC -lLogUtil -lcomm -lsmapi -lsocket -lnsl -lInitShm 

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

all: $(TARGET) 

$(SRCS):
#	$(GET) -s $@

$(TARGET): $(OBJS)
	echo $(LDFLAGS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	cp $(TARGET) $(TOP_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
#	cp $(TARGET) $(TOP_BIN)


clean:
	rm -f *.o $(TARGET) core*
#	sccs clean

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
