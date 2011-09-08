#
# Makefile to compile source directories
#

SHELL		= /bin/sh

# SITE MAKE UTILITY LOCATION
MAKE		= /usr/local/bin/make
# UPRESTO MAKE UTILITY LOCATION
#MAKE		= /usr/bin/make

SUBDIRS		=	\
				ANZ_LIB \
				COMM \
				HASH \
				NIFO \
				UTILLIB \
				IPCLIB \
				SHMUTIL_LIB \
				IPPOOLBIT \
				MMDB_DESTIP \
				MMDB_DESTPORT \
				INITSHM_LIB \
				CGALIB

all: $(patsubst %, __ALL__%, $(SUBDIRS))
clean: $(patsubst %, __CLEAN__%, $(SUBDIRS))

$(patsubst %, __ALL__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __ALL__%, %, $@)

$(patsubst %, __CLEAN__%, $(SUBDIRS)):
	$(MAKE) -C $(patsubst __CLEAN__%, %, $@) clean

