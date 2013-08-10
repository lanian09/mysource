SHELL   = /bin/sh
CC      = gcc
AR      = ar
RM      = rm -f
CP      = /usr/bin/cp -f
MV      = mv -f
DEPEND  = makedepend
# SITE MAKE UTILITY LOCATION
MAKE    = /usr/local/bin/make
# UPRESTO MAKE UTILITY LOCATION
#MAKE    = /usr/bin/make
MAKEFILE	= Makefile
LDFLAGS	= /export/dsc/work/LGT_DSC/SMLEG/libsrc/MISALIGN/misalign.o
#LDFLAGS	= /home/june/work/LGT_DSC/SMLEG/libsrc/MISALIGN/misalign.o

OPTIMIZE = -g3
#OPTIMIZE = -O

#MACHINE = -m64
MACHINE = 
#MACHINE = -m32         # for 32-bit App in 64-bit OS

#DEFINES = -DBUFFERING   # Turn on buffering message queue for CAPD, PANA
DEFINES = -Wall

COMMON_FLAGS = $(MACHINE) $(OPTIMIZE) -Wall -DSMALL_SESSION 
