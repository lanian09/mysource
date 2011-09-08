CC=gcc

# Specify here the 
KERNEL_VER=$(shell uname -r)
KERNEL_DIR=/lib/modules/$(KERNEL_VER)/source/include

#INCLUDE=-I$(KERNEL_DIR) -I../../INC -I/usr/include/dag
INCLUDE=-I. -I../../INC -I../../TOOLS/SIM/include -I../../../OAM/INC -I../../../LIB/INC -I../../../INC

# SHOW_TRAFFIC	: show traffic
DEFINES= -DAQUA_DEBUG -DNEW_LOGDEFINE
ifeq ($(AQUA_DEBUG),)
DEBUG   = -g3 -Wall
else
DEBUG   = $(AQUA_DEBUG)
endif

ifeq ($(SYSTYPE), RP)
DEFINES += -DRP
endif
ifeq ($(SYSTYPE), PI)
DEFINES += -DPI
endif
ifeq ($(SYSTYPE), PIOLD)
DEFINES += -DPIOLD
endif

#DEBUG	+= -DCOMMERCIALLOG

TARGET	= CAPD_SIM
OBJS 	= capd_main.o capd_func.o capd_init.o capd_test.o

#LIBS	= -lInitShm -lLogUtil -lnifo -lmems -lSTGnifo -lSTGmems -lSTGcommon_stg -lUtil -ldag -lSHMUTIL
LIBS	= -lLOGLIB -lnifo -ldag

$(TARGET): $(OBJS)
	$(CC) -L../../../LIB/LIB $(OBJS) -o $(TARGET) $(LIBS)
	cp $(TARGET) ../../BIN/

capd_main.o: capd_main.c
	$(CC) -c $(DEFINES) $(DEBUG) $(INCLUDE) $<

capd_func.o: capd_func.c
	$(CC) -c $(DEFINES) $(DEBUG) $(INCLUDE) $<

capd_init.o: capd_init.c
	$(CC) -c $(DEFINES) $(DEBUG) $(INCLUDE) $<

capd_test.o: capd_test.c
	$(CC) -c $(DEFINES) $(DEBUG) $(INCLUDE) $<

clean:
	/bin/rm -f $(TARGET) *.o core* 
