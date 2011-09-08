CC		= gcc
LINT	= lint

INSTALL = /usr/sbin/install

#INCLUDE =  -I. -I../../include -I../../libsrc/COMM -I/opt/mysql/mysql/include
INCLUDE =  -I. -I../../include -I../../libsrc/COMM -I${MYSQL_HOME}/include


BINDIR = ../../bin
#LOC_LIBDIR = -L../../lib -L/opt/mysql/mysql/lib
LOC_LIBDIR = -L../../lib -L${MYSQL_HOME}/lib

PROGRAMS	= stmd 

#20041202.mnpark
#LIBRARIES = -lcomm -lnsl -lsocket -lmysqlclient -lm
LIBRARIES = -lcomm -lnsl -lsocket -lmysqlclient -lz -lm

LDOPTIONS = $(LOC_LIBDIR) $(LIBRARIES)

#####################################################################################
# DEFINES INFO
#   -D_ISSUE : DELETE 기준 : 7일전 DATA 삭제, 
#	-DTEST	 : DELETE 기준 : 현재시간 전 DATA 삭제, 5분 주기 
# EX) DEFINES =  -DISSUE  -DTEST
#####################################################################################

CFLAGS = -Wall -g -DDEBUG -DDELAY $(DEFINES) $(INCLUDE)#-DUNIX_SVR4 -DSYSTEMV -DUNIX -DSOLARIS -DMULTITHREAD -DSOLARIS_SPARC -g -D_TPS_

SRCS	=   stmd_init.c \
		stmd_cron_stat.c \
		stmd_main.c \
		stmd_mmchdl.c \
		stmd_nms_stat.c \
		stmd_statisticrpt.c \
		stmd_ondemand_stat.c \
		stmd_periodic_flt.c \
		stmd_periodic_flow.c \
		stmd_periodic_load.c \
		stmd_periodic_link.c \
		stmd_periodic_stat.c \
		stmd_periodic_ruleset.c \
		stmd_periodic_ruleent.c \
		stmd_periodic_leg.c \
		stmd_periodic_logon.c \
		stmd_periodic_sms.c \
		stmd_periodic_delay.c \
		stmd_tool.c \
		stmd_txmsg.c \
		stmd_update.c \
		stmd_update_flt.c \
		stmd_update_flow.c \
		stmd_update_load.c \
		stmd_update_lur.c \
		stmd_update_tur.c \
		stmd_update_block.c \
		stmd_update_ruleset.c \
		stmd_update_ruleent.c \
		stmd_update_sms.c \
		stmd_update_leg.c \
		stmd_update_logon.c \
		stmd_update_delay.c \
		stmd_make_msg.c

OBJS	= $(SRCS:.c=.o)

#stmd_periodic_rule_cnt.c \
#stmd_update_rule.c \
#stmd_update_pur.c \

#-----------------------------------------------------------

all:: $(PROGRAMS) install

$(PROGRAMS): $(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDOPTIONS)
#	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDOPTIONS)

install:
	cp $(PROGRAMS) $(BINDIR)

clean:
	rm -f $(OBJS) $(PROGRAMS) core

lint:
#	$(LINT) $(INCLUDES) -h -v -a -u -x $(SRCS)
	$(LINT) $(INCLUDES) -h -v -a -u -x -n -l $(SRCS)

# DO NOT DELETE

