#--------------------- Makefile which have no SubDir ---------------------
MAKE        = `which make`

SUB_DIRS   += NSOCK
SUB_DIRS   += IPC
SUB_DIRS   += LOG
SUB_DIRS   += UTIL
SUB_DIRS   += DB_MYSQL
SUB_DIRS   += FILE 
SUB_DIRS   += FIDB 
SUB_DIRS   += VERSION 
SUB_DIRS   += GIFO 
SUB_DIRS   += NIFO 
SUB_DIRS   += CIFO 
SUB_DIRS   += HASHO 
SUB_DIRS   += TIMERN 
SUB_DIRS   += ANZ_LIB 

SUB_ALL	    = ${SUB_DIRS}

all: ${patsubst %, __ALL__%, ${SUB_ALL}}

${patsubst %,__ALL__%, ${SUB_ALL}}:
	${MAKE} -C ${patsubst __ALL__%,%,$@}

clean: ${patsubst %, __CLEAN__%, ${SUB_ALL}}

${patsubst %, __CLEAN__%, ${SUB_ALL}}:
	${MAKE} -C ${patsubst __CLEAN__%,%,$@} clean
