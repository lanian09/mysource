
prefix = /usr/local
exec_prefix = ${prefix}
include= ${prefix}/include

INSTALL= /usr/bin/install -c

# install libdag header files (API)
HEADERS  = dagapi.h dagcrc.h daginf.h dag37t_api.h dagnew.h dagpbm.h dagreg.h dagpci.h\
	dagutil.h dagclarg.h dag_platform.h dag_platform_linux.h \
	dag_platform_freebsd.h dag_platform_solaris.h dag_platform_win32.h \
	dag_platform_macosx.h

.PHONY: all clean distclean install depend

all:

clean:

distclean:
	$(RM) Makefile

install: ${HEADERS}
	@echo installing headers in ${include}/
	$(INSTALL) -d ${include}
	$(INSTALL) ${HEADERS} ${include}

depend:
