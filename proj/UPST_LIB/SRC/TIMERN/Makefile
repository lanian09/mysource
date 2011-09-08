#-----------------------------------------------------------
SHELL	= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
#DEPEND	= makedepend
LINT	= lint
MAKEFILE= Makefile

STD_INC	=
LOC_INC	=-I. -I../../INC
GBL_INC	=

DST_DIR = ../../LIB

#-----------------------------------------------------------
OPTIMIZE=
INCLUDES= $(STD_INC) $(LOC_INC) $(GBL_INC)
DEFINES =
DEBUG   = -g3 -Wall
M_FLAG	= 

LDFLAGS = 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)

#-----------------------------------------------------------
TARGET2	= libhashg.a
SRCS2	= hashg.c
SRCS2  += stHASHGINFO_Prt.c stHASHGNODE_Prt.c
SRCS2  += stTIMERNKEY_Prt.c stTIMERNINFO_Prt.c
OBJS2   = $(SRCS2:.c=.o)

LIBS    =
TARGET	= libtimerN.a
SRCS	= timerN.c $(SRCS2)
OBJS	= $(SRCS:.c=.o)


#-----------------------------------------------------------
#
.SUFFIXES: .c .s .o .i .u
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<


#-----------------------------------------------------------
all:: $(TARGET)

$(TARGET): $(OBJS)
	ar rcu $@ $(OBJS)
	@for i in $(TARGET); do \
		echo Installing $$i in $(DST_DIR); \
		cp $$i $(DST_DIR); \
	done

hashg:: $(TARGET2)

$(TARGET2): $(OBJS2)
	ar rcu $@ $(OBJS2)
	@for i in $(TARGET2); do \
		echo Installing $$i in $(DST_DIR); \
		cp $$i $(DST_DIR); \
	done

#$(TARGET) : $(OBJS)
#	$(RM) -f $@
#	$(AR) clq $@ $(OBJS)
#	$(AR) ts $@
#	$(MV) *.a ../

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(CC) -M $(CFLAGS) -c $?
#	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(DST_DIR)

doc:
	doxygen

clean::
	$(RM) -rf *.o core* $(TARGET) $(TARGET2) $(OBJS) $(OBJS2)

# DO NOT DELETE THIS LINE -- make depend depends on it.
