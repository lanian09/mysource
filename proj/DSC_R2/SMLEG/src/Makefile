#
# Makefile to compile source directories
#

SHELL	= /bin/sh
MAKE	= make
SUBDIRS	= MMCR IXPC SAMD IFB CAPD PANA RANA RLEG SMPP RDRANA RDRCAPD PCUBE_SUBS REANA 

all: $(patsubst %, __ALL__%, $(SUBDIRS))
clean: $(patsubst %, __CLEAN__%, $(SUBDIRS))
install: $(patsubst %, __INSTALL__%, $(SUBDIRS))

$(patsubst %, __ALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __ALL__%, %, $@)

$(patsubst %, __CLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __CLEAN__%, %, $@) clean

$(patsubst %, __INSTALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __INSTALL__%, %, $@) install
