MAKE		=`which make`
SOURCE		=SRC
LIBRARY		=LIBSRC
SUB_DIRS	=${SOURCE} ${LIBRARY}

all : lib src

lib : ${patsubst %,__LIB_ALL__%, ${LIBRARY}}

${patsubst %,__LIB_ALL__%, ${LIBRARY}}:
	${MAKE} -C ${patsubst __LIB_ALL__%,%,$@}

src : ${patsubst %,__SRC_ALL__%,${SOURCE}}

${patsubst %,__SRC_ALL__%,${SOURCE}}:
	${MAKE} -C ${patsubst __SRC_ALL__%,%,$@}

clean : ${patsubst %,__CLEAN__%,${SUB_DIRS}}

${patsubst %,__CLEAN__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean

cleanall : ${patsubst %,__CLEAN__%,${SUB_DIRS}}
	@echo
	@echo "---------------------------"
	@echo "Remove BIN, LIB"
	@echo "---------------------------"
	find BIN -maxdepth 1 -type f -exec rm -f '{}' \;
