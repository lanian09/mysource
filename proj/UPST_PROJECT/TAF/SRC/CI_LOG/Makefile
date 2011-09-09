
CC		= gcc

BIN_DIR = ../../BIN
LIB_DIR = ../../../LIB
OAM_DIR = ../../../OAM

LIBS    = -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR+=/lib/modules/$(KERNEL_VER)/source/include

OPTIMIZE= 
INCLUDES= -I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEFINES = 

ifeq ($(SYSTYPE), RP)
DEFINES += -DRP
endif
ifeq ($(SYSTYPE), PI)
DEFINES += -DPI
endif
ifeq ($(SYSTYPE), PIOLD)
DEFINES += -DPIOLD
endif

DEBUG   = -g3 -Wall 

CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
SRCS    =
SRCS   += ci_log_init.c
SRCS   += ci_log_sock.c
SRCS   += ci_log_func.c
SRCS   += ci_log_util.c
SRCS   += ci_log_main.c 

TARGET  = CI_LOG 

OBJS	= $(SRCS:.c=.o)

#LIBRARY = -lSHMUTIL -lInitShm -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho -lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGcommon_stg -lSTGhashg
LIBRARY = -lLOGLIB -lVERLIB -lFILELIB -lUTILLIB -lNSOCKLIB -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg
LDFLAGS = $(LIBS) $(LIBRARY)

#
#-----------------------------------------------------------
#

#

all: $(TARGET)

#$(SRCS):
#	$(GET) -s $@


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) 
	cp $(TARGET) $(BIN_DIR)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	/bin/rm -f *.o $(TARGET) core*

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
