
CC      = gcc
CP		= /bin/cp
SHELL	= /bin/sh
MAKE	= make

TOP_LIB	= ../../lib
TOP_INC = ../../inc
#TOP_INC = /home/june/cssim/inc

SUBDIRS		= hashg \
				hasho \
				memg \
				timerN

all: $(patsubst %, __ALL__%, $(SUBDIRS)) 
#	install -d $(TOP_LIB)
#	install -d $(TOP_INC)

$(patsubst %, __ALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __ALL__%, %, $@)


clean: $(patsubst %, __CLEAN__%, $(SUBDIRS))

$(patsubst %, __CLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __CLEAN__%, %, $@) clean


