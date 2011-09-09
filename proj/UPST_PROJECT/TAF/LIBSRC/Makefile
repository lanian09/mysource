MAKE		=`which make`

COMMON		= TOOLS MMDB_GREENTRY MMDB_PSESS IPPOOLBIT
COMMON_DIRS = ${COMMON}

all: RP

rp : RP
pi : PI

RP: CGA_RP ${patsubst %,__ALL__%,${COMMON_DIRS}}
PI: CGA_PI ${patsubst %,__ALL__%,${COMMON_DIRS}}
PIOLD: CGA_PIOLD ${patsubst %,__ALL__%,${COMMON_DIRS}}

CGA_RP:
	@echo -n "BUILDING $@ LIBSRC '${COMMON_DIRS}'"
	#${MAKE} -C ${CGALIB} CGA_RP
	#${MAKE} -C CGA_RP

CGA_PI:
	@echo -n "BUILDING PI LIBSRC '${COMMON_DIRS}'"
	#${MAKE} -C ${CGALIB} CGA_PI
	#${MAKE} -C CGA_PI

CGA_PIOLD:
	@echo -n "BUILDING PIOLD LIBSRC '${COMMON_DIRS}'"
	#${MAKE} -C ${CGALIB} CGA_PIOLD
	#${MAKE} -C CGA_PIOLD

${patsubst %,__ALL__%,${COMMON_DIRS}}:
	${MAKE} -C ${patsubst __ALL__%, %, $@}

${patsubst %,__DEPEND__%,${COMMON_DIRS}}:
	${MAKE} -C ${patsubst __DEPEND__%, %, $@} dep

${patsubst %,__CLEAN__%,${COMMON_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%, %, $@} clean

dep: ${patsubst %,__DEPEND__%,${COMMON_DIRS}}
clean: ${patsubst %,__CLEAN__%,${COMMON_DIRS}}


