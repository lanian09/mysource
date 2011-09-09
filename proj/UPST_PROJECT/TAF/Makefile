MAKE		=`which make`

LIB_SOURCE	=LIBSRC
SOURCE		=SRC
SUB_DIRS	=${LIB_SOURCE} ${SOURCE}

all: RP

rp: RP
pi: PI
RP:
	@echo
	@echo "-----------------------"
	@echo "BUILDING RP TAF PACKAGE"
	@echo "-----------------------"
	$(MAKE) -C $(LIB_SOURCE) RP
	$(MAKE) -C $(SOURCE) RP
PI:
	@echo
	@echo "-----------------------"
	@echo "BUILDING PI TAF PACKAGE"
	@echo "-----------------------"
	$(MAKE) -C $(LIB_SOURCE) PI
	$(MAKE) -C $(SOURCE) PI

PIOLD:
	@echo
	@echo "-----------------------"
	@echo "BUILDING PIOLD TAF PACKAGE"
	@echo "-----------------------"
	$(MAKE) -C $(LIB_SOURCE) PIOLD
	$(MAKE) -C $(SOURCE) PIOLD


${patsubst %,__DEPEND__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __DEPEND__%,%,$@} dep

${patsubst %,__CLEAN__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean

dep: ${patsubst %,__DEPEND__%,${SUB_DIRS}}
clean: ${patsubst %,__CLEAN__%,${SUB_DIRS}}
cleanall: ${patsubst %,__CLEAN__%,${SUB_DIRS}}
	@echo
	@echo "-----------------------"
	@echo "Remove BIN, LIB"
	@echo "-----------------------"
	find BIN -maxdepth 1 -type f -exec rm -f '{}' \;
	find LIB -maxdepth 1 -type f -exec rm -f '{}' \;
