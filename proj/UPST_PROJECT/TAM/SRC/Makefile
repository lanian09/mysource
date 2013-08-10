MAKE		= `which make`

SUB_DIRS	=
SUB_DIRS	+= A_ROAM
SUB_DIRS	+= A_RPPI
SUB_DIRS	+= CHGSVCM
SUB_DIRS	+= MOND
SUB_DIRS	+= M_LOG
SUB_DIRS	+= M_SVCMON
SUB_DIRS	+= M_TRACE_PCAP
SUB_DIRS	+= O_SVCMON
SUB_DIRS	+= SI_DB
SUB_DIRS	+= SI_LOG
SUB_DIRS	+= SI_NMS
SUB_DIRS	+= SI_SVCMON
SUB_DIRS	+= SNMPIF
SUB_DIRS	+= S_MNG

all : ${patsubst %,__ALL__%,${SUB_DIRS}}

${patsubst %,__ALL__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __ALL__%,%,$@}

clean : ${patsubst %,__CLEAN__%,${SUB_DIRS}}

${patsubst %,__CLEAN__%,${SUB_DIRS}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
