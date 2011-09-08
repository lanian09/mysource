SHELL		=`which sh`
CP			=`which cp`
RM			=`which rm`

# CGA 하위 디렉토리에 있는 LEX를 이용한 라이브러리
SUB_LEX     =
SUB_LEX    += EMS
SUB_LEX    += FB
SUB_LEX    += FV
SUB_LEX    += HTTP
SUB_LEX    += MSRP
SUB_LEX    += PAGE
SUB_LEX    += RTSP
SUB_LEX    += SIP
SUB_LEX    += WIDGET
SUB_LEX    += XCAP

TAF_DIR 	= ../TAF
TAM_DIR		= ../TAM

all : build lex install

build:
	@echo "building CGA library ..."
	${SHELL} ./structg_conf.sh 

install:
	${CP} common_stg.h ../INC
	${CP} *.a ${TAF_DIR}/LIB
	${CP} *.a ${TAM_DIR}/LIB
	

clean: ${patsubst %,__CLEAN__%, ${SUB_LEX}}
	${SHELL} clean_all.sh

${patsubst %,__CLEAN__%, ${SUB_LEX}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
	
lex: ${patsubst %, __ALL__%, ${SUB_LEX}}

${patsubst %,__ALL__%, ${SUB_LEX}}:
	${MAKE} -C ${patsubst __ALL__%,%,$@}

