PERL			= `which perl`
CP				= `which cp`
MAKE			= `which make`

CMD_FILE		= gifo.pl
PROCESS_LIST 	= process_list.conf
FLOW_LIST		= flow_list.conf
CONFIGURE_FILES = ${PROCESS_LIST} ${FLOW_LIST}
TARGET			= cifo.conf gifo.conf nifo_zone.conf

#-------------------------------------------

all: taf

taf:
	@echo
	@echo "------------------------"
	@echo " TAF GIFO Configuration "
	@echo "------------------------"
	@echo
	@echo ${CP} ${PROCESS_LIST}.taf ${PROCESS_LIST}
	@echo ${CP} ${FLOW_LIST}.taf ${FLOW_LIST}
	${CP} ${PROCESS_LIST}.taf ${PROCESS_LIST}
	${CP} ${FLOW_LIST}.taf ${FLOW_LIST}
	${MAKE} configure
	${CP} ${TARGET} ../../TAF/DATA/
	

tam:
	@echo
	@echo "------------------------"
	@echo " TAM GIFO Configuration "
	@echo "------------------------"
	@echo
	@echo ${CP} ${PROCESS_LIST}.tam ${PROCESS_LIST}
	@echo ${CP} ${FLOW_LIST}.tam ${FLOW_LIST}
	${CP} ${PROCESS_LIST}.tam ${PROCESS_LIST}
	${CP} ${FLOW_LIST}.tam ${FLOW_LIST}
	${MAKE} configure
	${CP} ${TARGET} ../../TAM/DATA/

configure:
	@echo ${PERL} ${CMD_FILE}
	${PERL} ${CMD_FILE}

clean:
	rm -rf ${CONFIGURE_FILES} ${TARGET}
