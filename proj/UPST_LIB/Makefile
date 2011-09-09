MAKE			=`which make`
RM				=`which rm`
SRC_DIRS		=SRC
LIB_DIRS		=LIB

all : ${patsubst %,__ALL__%,${SRC_DIRS}}

${patsubst %,__ALL__%,${SRC_DIRS}}:
	${MAKE} -C ${patsubst __ALL__%,%,$@}

clean: ${patsubst %,__CLEAN__%,${SRC_DIRS}}

${patsubst %,__CLEAN__%,${SRC_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
	${RM} -f ${LIB_DIRS}/*.a



