MAKE        = `which make`

SUB_DIRS    =
SUB_DIRS   += TOOLS

all : ${patsubst %,__ALL__%,${SUB_DIRS}}

${patsubst %,__ALL__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __ALL__%,%,$@}

clean : ${patsubst %,__CLEAN__%,${SUB_DIRS}}

${patsubst %,__CLEAN__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
