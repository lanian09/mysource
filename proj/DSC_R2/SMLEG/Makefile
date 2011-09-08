#
# Makefile to compile source directories
#

SHELL		= /bin/sh

MAKE		= make

SUBDIRS		= libsrc src bin lib

all: $(patsubst %, __ALL__%, $(SUBDIRS))
clean: $(patsubst %, __CLEAN__%, $(SUBDIRS))
binclean: $(patsubst %, __BINCLEAN__%, $(SUBDIRS))

$(patsubst %, __ALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __ALL__%, %, $@)

$(patsubst %, __CLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __CLEAN__%, %, $@) clean

$(patsubst %, __BINCLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __BINCLEAN__%, %, $@) clean
#	find ./bin -type f -mindepth 1 -maxdepth 1 | xargs rm -f
#	find ./lib -type f -mindepth 1 -maxdepth 1 | xargs rm -f
	find ./bin -type f | xargs rm -f
	find ./lib -type f | xargs rm -f

