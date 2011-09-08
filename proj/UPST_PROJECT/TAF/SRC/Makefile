MAKE		=`which make`

COMMON		= A_CALL CI_LOG S_MNG PRE_A
PIPROC		= A_TCP A_SCTP A_HTTP A_WAP20 A_WIPI A_2G
PIPROC	   += A_JAVA A_MMS A_JNC A_VOD A_FV A_EMS A_FB A_IV A_XCAP
PIPROC     += A_MSRPM A_MSRPT A_SIPM A_SIPT A_SIPD A_ONLINE A_WIDGET A_DIAMETER A_RADIUS
PIPROC	   += A_VT A_IM A_FTP A_DNS A_IHTTP A_ITCP A_INET
RPPROC		= A_RP A_GRE A_L2TP

SUB_DIRS    = ${COMMON} ${PIPROC} ${RPPROC}


all: RP

rp : RP
pi : PI

# RP (default) ----------------------------------------
RP: 
	@echo "-------------------------------------------"
	@echo "BUILDING RP TAF BLOCKS"
	@echo "-------------------------------------------"
	$(foreach PROC, ${COMMON}, ${MAKE} "SYSTYPE=$@" -C $(PROC);)
	$(foreach PROC, ${RPPROC}, ${MAKE} "SYSTYPE=$@" -C $(PROC);)

# PI ---------------------------------------------------
PI: 
	@echo "-------------------------------------------"
	@echo "BUILDING PI TAF BLOCKS"
	@echo "-------------------------------------------"
	@echo
	$(foreach PROC, ${COMMON}, ${MAKE} "SYSTYPE=$@" -C $(PROC);)
	$(foreach PROC, ${PIPROC}, ${MAKE} "SYSTYPE=$@" -C $(PROC);)


# PIOLD ------------------------------------------------
PIOLD: 
	@echo "-------------------------------------------"
	@echo "BUILDING PIOLD TAF BLOCKS"
	@echo "-------------------------------------------"
	@echo
	${MAKE} "PLATFORM=PI" "SYSTYPE=$@" -C ${PLAT1}
	${MAKE} "PLATFORM=PI" "SYSTYPE=$@" -C ${PLAT2}
	${MAKE} "PLATFORM=PI" "SYSTYPE=$@" -C ${PLAT3}
	${MAKE} "PLATFORM=PI" "SYSTYPE=$@" -C ${PLAT4}
	$(foreach PROC, ${COMMON}, ${MAKE} "SYSTYPE=$@" -C $(PROC);)
	$(foreach PROC, ${PIPROC}, ${MAKE} "SYSTYPE=$@" -C $(PROC);)


${patsubst %,__ALL__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __ALL__%,%,$@}

dep: ${patsubst %,__DEPEND__%,${SUB_DIRS}}
${patsubst %,__DEPEND__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __DEPEND__%,%,$@} dep

clean: ${patsubst %,__CLEAN__%,${SUB_DIRS}}

${patsubst %,__CLEAN__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean

