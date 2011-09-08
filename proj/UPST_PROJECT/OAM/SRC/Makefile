#--------------------- Makefile which have no SubDir ---------------------
MAKE        = `which make`

TAM_SUB		=
TAM_SUB	   += CHSMD
TAM_SUB	   += ALMD
TAM_SUB	   += COND
TAM_SUB	   += SI_SVC
TAM_SUB	   += FSTAT
TAM_SUB	   += MMCD
TAM_SUB	   += RMI

TAF_SUB		=
TAF_SUB	   += CHSMD_F
TAF_SUB	   += ALMD_F
TAF_SUB	   += CI_SVC

SUB_DIRS    =
SUB_DIRS   += QMON
SUB_DIRS   += TIFB

SUB_ALL		= ${TAM_SUB} ${TAF_SUB} ${SUB_DIRS}

all: taf

taf : ${patsubst %, __TAF_ALL__%, ${TAF_SUB}} ${patsubst %, __COMM_ALL__%, ${SUB_DIRS}}

taf_msg :
	@echo
	@echo " ----------------------------"
	@echo " TAF O&M BLOCKs Compiling ..."
	@echo " ----------------------------"
	@echo

${patsubst %,__TAF_ALL__%, ${TAF_SUB}}: taf_msg
	${MAKE} -C ${patsubst __TAF_ALL__%,%,$@}

tam : ${patsubst %, __TAM_ALL__%, ${TAM_SUB}} ${patsubst %, __COMM_ALL__%, ${SUB_DIRS}}

tam_msg :
	@echo
	@echo " ----------------------------"
	@echo " TAM O&M BLOCKs Compiling ..."
	@echo " ----------------------------"
	@echo

${patsubst %,__TAM_ALL__%, ${TAM_SUB}}: tam_msg
	${MAKE} -C ${patsubst __TAM_ALL__%,%,$@}

${patsubst %,__COMM_ALL__%, ${SUB_DIRS}}:
	${MAKE} -C ${patsubst __COMM_ALL__%,%,$@}

dep: ${patsubst %, __DEPEND__%, ${SUB_ALL}}

${patsubst %, __DEPEND__%, ${SUB_ALL}}:
	${MAKE} -C ${patsubst __DEPEND__%,%,$@} dep

clean: ${patsubst %, __CLEAN__%, ${SUB_ALL}}

${patsubst %, __CLEAN__%, ${SUB_ALL}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
