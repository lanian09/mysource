
CC      = gcc
CP		= /bin/cp
SHELL	= /bin/sh
MAKE	= make


SUBDIRS		= libsrc\
			  src 

all: $(patsubst %, __ALL__%, $(SUBDIRS)) 

$(patsubst %, __ALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __ALL__%, %, $@)


clean: $(patsubst %, __CLEAN__%, $(SUBDIRS))

$(patsubst %, __CLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __CLEAN__%, %, $@) clean

