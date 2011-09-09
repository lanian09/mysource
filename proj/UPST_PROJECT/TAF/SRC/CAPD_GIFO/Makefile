CC=gcc

# Specify here the 
#KERNEL_VER=$(shell uname -r)
#KERNEL_DIR=/lib/modules/$(KERNEL_VER)/source/include

#DON'T WORK :: INCLUDE=-I$(KERNEL_DIR) -I../../INC
INCLUDE=-I. -I../../INC -I../../TOOLS/SIM/include -I../../../OAM/INC -I../../../LIB/INC -I../../../INC

# SHOW_TRAFFIC	: show traffic
DEFINES=-DBUFFERING
DEBUG  = -g3 -Wall

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

TARGET	= CAPD
OBJS 	= capd_main.o capd_func.o capd_init.o capd_test.o

#LIBS	= -lInitShm -lLogUtil -lnifo -lmems -lSTGnifo -lSTGmems -lSTGcommon_stg -lUtil -ldag -lSHMUTIL
#LIBS	= -lLOGLIB -lnifo -ldag 
LIBS 	= -lLOGLIB -lVERLIB -lFILELIB  -lIPCLIB
LIBS	+= -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg ### -lPKT 

$(TARGET): $(OBJS)
	$(CC) -L../../../LIB/LIB -L../../LIB $(OBJS) -o $(TARGET) $(LIBS)
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
