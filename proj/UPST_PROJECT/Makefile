MAKE		=`which make`

CGA_DIR		=CGALIB
TAF_SRC		=TAF
TAM_SRC		=TAM
LIB_HOME	=LIB
OAM_HOME	=OAM

SUB_DIRS	= ${CGA_DIR} ${TAF_SRC} ${TAM_SRC}

all : RP

lib :
	${MAKE} -C ${LIB_HOME}

oam :
	${MAKE} -C ${OAM_HOME}

tam : cga
	@echo
	@echo "-----------------------"
	@echo "      BUILDING TAM     "
	@echo "-----------------------"
	${MAKE} -C ${TAM_SRC}


RP: cga
	@echo 
	@echo "-----------------------"
	@echo "      BUILDING RP      "
	@echo "-----------------------"
	$(MAKE) -C ${TAF_SRC} RP

PI: cga
	@echo 
	@echo "-----------------------"
	@echo "      BUILDING PI      "
	@echo "-----------------------"
	$(MAKE) -C ${TAF_SRC} PI

PIOLD: cga
	@echo 
	@echo "-----------------------"
	@echo "     BUILDING PIOLD    "
	@echo "-----------------------"
	$(MAKE) -C ${TAF_SRC} PIOLD

cga:
	@echo
	@echo "-----------------------------"
	@echo "     BUILDING CGA Library"
	@echo "-----------------------------"
	$(MAKE) -C ${CGA_DIR}

complete:
	@echo "compile complete"

clean: ${patsubst %,__CLEAN__%, ${SUB_DIRS}}

${patsubst %,__CLEAN__%, ${SUB_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean

cleanall: clean
	${MAKE} -C ${LIB_HOME} clean
	${MAKE} -C ${OAM_HOME} clean

