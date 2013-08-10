#
# Makefile to compile source directories
#

SHELL		= /bin/sh

MAKE		= /usr/local/bin/make

SUBDIRS		=	\
				include \
				solaris

all: $(patsubst %, __ALL__%, $(SUBDIRS))
clean: $(patsubst %, __CLEAN__%, $(SUBDIRS))

$(patsubst %, __ALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __ALL__%, %, $@)

$(patsubst %, __CLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __CLEAN__%, %, $@) clean

