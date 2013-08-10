include ../../MkConfig.mk

CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
C64FLAGS	= 
#CFLAG		= -g -DINTEL -Wall #-DDEBUG1 #-DMMDB_DEBUG #-Wall -D_REENTRANT #-DALONE_TEST

#INC_PATH	= -I. -I../../include -I../../libsrc/COMM -I../../libsrc/IPPOOLBIT -I../../libsrc/SM_API/include -I../../libsrc/SM_API/include/system -I../RLEG
INC_PATH	= -I. -I../../include -I../../libsrc/COMM -I../../libsrc/SM_API/include -I../../libsrc/SM_API/include/system 

LIB_PATH	= -L../../lib

CFLAGS		= -g -Wall -I$(INC_PATH) #-DDEBUG1 #-DMMDB_DEBUG #-Wall -D_REENTRANT #-DALONE_TEST

#LIBS 		= -lcomm -lsmapi -lpthread -lInitShm -lIPPOOLBIT -lHASHO  -lsocket  -lnsl -lLogUtil
LIBS 		= -lcomm -lsmapi -lIPC -lpthread -lnifo -lInitShm -lHASHO  -lsocket  -lnsl -lLogUtil

MAKEFILE    = Makefile

SRCS		=	mmcr_init.c \
				mmcr_main.c \
				mmcr_rxtxmsg.c \
				mmcr_mmchdl.c \
				mmcr_file.c \
                mmcr_mmc_setinfo.c \
                mmcr_shmprc.c \
                mmcr_msgformat.c \
                mmcr_util.c \
				mmc_hld.c \
				mmc_proc.c \
                mmcr_mmc_disinfo.c 
#                mmcr_mmc_addinfo.c \
#                mmcr_mmc_chginfo.c \
#                mmcr_mmc_delinfo.c \
##
#                mmcr_sem.c \
#                mmcr_mmc_dissvc.c \
#                mmcr_shm_svctype.c \
#                mmcr_mmc_svctype.c \
#                mmcr_mdb.c \
#                mmcr_mdbload.c \
#                mmcr_destipapi.c \
#                mmcr_destportapi.c \
#                mmcr_ipbtree.c \
#                mmcr_portbtree.c \
#                mmcr_destip.c \
#                mmcr_destport.c \
#                mmcr_ipalloc.c \
#                mmcr_portalloc.c \
#                mmcr_shm_pdsninfo.c \
#                mmcr_shm_ippoolinfo.c \
#                mmcr_svcopt.c \
#                mmcr_svcoptapi.c \
#                mmcr_svcoptalloc.c \
#                mmcr_svcoptbtree.c \
#                mmcr_shm_svcopt.c \

OBJS		= $(SRCS:.c=.o)

AOUT		= MMCR

DEST		= ../../bin

all:: $(AOUT) install

#$(OBJS): $(@:.o=.c)
#	$(CC) $(C64FLAGS) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(C64FLAGS) $(OBJS) $(LIB_PATH) $(LIBS) $(LDFLAGS)

shmmb_print:
	$(CC) -c  $(CFLAG)  $(INC_PATH) shmmb_print.c
	$(CC) -o shmmb_print shmmb_print.o $(OBJS) $(LIB_PATH) $(LIBS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INC_PATH) --$(LIB_PATH) -- $(SRCS)

install:
	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core
