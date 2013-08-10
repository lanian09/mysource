#
# Makefile to compile source directories
#

SHELL		= /bin/sh

MAKE		= make

SOURCES		=	\
			COND \
			FIMD \
			IFB \
			IXPC \
			MCDM \
			MMCD \
			NMSIF \
			RMI \
			SAMD \
			STMD \
			CDELAY \
			TOOLS/CDRLOG_VIEW \
			HAMON \
			SCEM \
			CSCM \
			DIRM

#			TOOLS/AAA_SIM \
#			TOOLS/AAA_VIEW \
#			TOOLS/UAWAP_VIEW \
#			TOOLS/UDR_VIEW \


all::
	for i in $(SOURCES) ; \
	do \
		(cd $$i ; echo "making all in ./$$i" ; \
			$(MAKE) all; ) \
	done

clean::
	for i in $(SOURCES) ; \
	do \
		(cd $$i ; echo "cleaning all in ./$$i" ; \
			$(MAKE) clean; ) \
	done
