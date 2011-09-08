MAKE			=`which make`
RM				=`which rm`
SRC_DIRS		=SRC
BIN_DIRS		=BIN

all : taf

taf : ${patsubst %,__TAF_ALL__%,${SRC_DIRS}}

${patsubst %,__TAF_ALL__%,${SRC_DIRS}}:
	${MAKE} -C ${patsubst __TAF_ALL__%,%,$@} taf

tam : ${patsubst %,__TAM_ALL__%,${SRC_DIRS}}

${patsubst %,__TAM_ALL__%,${SRC_DIRS}}:
	${MAKE} -C ${patsubst __TAM_ALL__%,%,$@} tam



clean: ${patsubst %,__CLEAN__%,${SRC_DIRS}}

${patsubst %,__CLEAN__%,${SRC_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
	${RM} -f ${BIN_DIRS}/*.a



