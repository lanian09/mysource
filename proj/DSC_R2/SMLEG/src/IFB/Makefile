include ../../MkConfig.mk

BINDIR		= ../../bin
#C64FLAGS	= -m64

DEFINES		= -D_REENTRANT
CFLAG		= $(COMMON_FLAGS) $(DEFINES)
DEFINES		+=-DDEBUG

INC_PATH	= -I. -I../../include -I../../libsrc/COMM

LIB_PATH	= -L../../lib

LIBS		= -lcomm -lInitShm -lcrypt

COMMSRCS    = ifb_comm.c ifb_shm.c
COMMOBJS	= $(COMMSRCS:.c=.o)

DISPRC		= disprc
STARTPRC	= startprc
KILLPRC		= killprc
RMSYSQM		= rmsysqm
CLRQ		= clrq

BINS		= $(DISPRC) $(STARTPRC) $(KILLPRC) $(CLRQ)
OBJS		= $(BINS:=.o)


DEST		= ../../bin

#
#---------------------------------------------------------------------
#
all: $(BINS) install

$(DISPRC): $(DISPRC).o $(COMMOBJS)
#	$(CC) -m64 $(LIB_PATH) -o $@ $^ $(LIBS)
	$(CC) $(LIB_PATH) -g -o $@ $^ $(LIBS) $(LDFLAGS)

$(STARTPRC): $(STARTPRC).o $(COMMOBJS)
#	$(CC) -m64 $(LIB_PATH) -o $@ $^ $(LIBS)
	$(CC) $(LIB_PATH) -o $@ $^ $(LIBS) $(LDFLAGS)

$(KILLPRC): $(KILLPRC).o $(COMMOBJS)
#	$(CC) -m64 $(LIB_PATH) -o $@ $^ $(LIBS)
	$(CC) $(LIB_PATH) -o $@ $^ $(LIBS) $(LDFLAGS)

$(RMSYSQM): $(RMSYSQM).o $(COMMOBJS)
#	$(CC) -m64 $(LIB_PATH) -o $@ $^ $(LIBS)
	$(CC) $(LIB_PATH) -o $@ $^ $(LIBS) $(LDFLAGS)

$(CLRQ): $(CLRQ).o $(COMMOBJS)
	$(CC) $(LIB_PATH) -o $@ $^ $(LIBS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $<

install:
	$(CP) $(BINS) $(BINDIR)

dep-startprc: ${STARTPRC}.c ${COMMSRCS}
	${DEPEND} -o.o -f ${MAKEFILE} -- ${INC_PATH} -- ${STARTPRC}.c ${COMMSRCS}

dep-killprc: ${KILLPRC}.c ${COMMSRCS}
	${DEPEND} -o.o -f ${MAKEFILE} -- ${INC_PATH} -- ${KILLPRC}.c ${COMMSRCS}

clean:
	$(RM) $(BINS) $(OBJS) $(COMMOBJS) core > /dev/null

# DO NOT DELETE

killprc.o: ifb_proto.h /usr/include/stdio.h /usr/include/sys/feature_tests.h
killprc.o: /usr/include/sys/ccompile.h /usr/include/sys/isa_defs.h
killprc.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
killprc.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
killprc.o: /usr/include/iso/stdio_c99.h /usr/include/stdlib.h
killprc.o: /usr/include/iso/stdlib_iso.h /usr/include/iso/stdlib_c99.h
killprc.o: /usr/include/string.h /usr/include/iso/string_iso.h
killprc.o: /usr/include/strings.h /usr/include/sys/types.h
killprc.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
killprc.o: /usr/include/sys/select.h /usr/include/sys/time_impl.h
killprc.o: /usr/include/sys/time.h /usr/include/sys/time.h
killprc.o: /usr/include/sys/errno.h /usr/include/sys/stat.h
killprc.o: /usr/include/sys/stat_impl.h /usr/include/sys/procfs.h
killprc.o: /usr/include/sys/old_procfs.h /usr/include/sys/signal.h
killprc.o: /usr/include/sys/iso/signal_iso.h /usr/include/sys/unistd.h
killprc.o: /usr/include/sys/siginfo.h /usr/include/sys/machsig.h
killprc.o: /usr/include/sys/fault.h /usr/include/sys/syscall.h
killprc.o: /usr/include/sys/procfs_isa.h /usr/include/sys/regset.h
killprc.o: /usr/include/v7/sys/privregs.h /usr/include/v7/sys/psr.h
killprc.o: /usr/include/sys/fsr.h /usr/include/sys/msg.h
killprc.o: /usr/include/sys/ipc.h /usr/include/dirent.h
killprc.o: /usr/include/sys/dirent.h /usr/include/fcntl.h
killprc.o: /usr/include/sys/fcntl.h /usr/include/ctype.h
killprc.o: /usr/include/iso/ctype_iso.h /usr/include/iso/ctype_c99.h
killprc.o: /usr/include/unistd.h /usr/include/stropts.h
killprc.o: /usr/include/sys/stropts.h /usr/include/sys/conf.h
killprc.o: /usr/include/sys/t_lock.h /usr/include/sys/machlock.h
killprc.o: /usr/include/sys/param.h /usr/include/sys/mutex.h
killprc.o: /usr/include/sys/rwlock.h /usr/include/sys/semaphore.h
killprc.o: /usr/include/sys/condvar.h /usr/include/crypt.h /usr/include/pwd.h
killprc.o: ../../include/commlib.h ../../libsrc/COMM/conflib.h
killprc.o: ../../libsrc/COMM/loglib.h /usr/include/stdarg.h
killprc.o: /usr/include/iso/stdarg_iso.h /usr/include/sys/va_impl.h
killprc.o: /usr/include/iso/stdarg_c99.h ../../libsrc/COMM/socklib.h
killprc.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
killprc.o: /usr/include/sys/socket_impl.h /usr/include/sys/un.h
killprc.o: /usr/include/net/if_dl.h /usr/include/sys/netconfig.h
killprc.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
killprc.o: /usr/include/sys/vnode.h /usr/include/sys/rwstlock.h
killprc.o: /usr/include/sys/ksynch.h /usr/include/sys/cred.h
killprc.o: /usr/include/sys/resource.h /usr/include/vm/seg_enum.h
killprc.o: /usr/include/sys/kstat.h /usr/include/sys/kmem.h
killprc.o: /usr/include/sys/vmem.h /usr/include/sys/poll.h
killprc.o: /usr/include/sys/strmdep.h /usr/include/sys/model.h
killprc.o: /usr/include/sys/strft.h /usr/include/sys/byteorder.h
killprc.o: /usr/include/signal.h /usr/include/iso/signal_iso.h
killprc.o: /usr/include/netinet/tcp.h /usr/include/sys/inttypes.h
killprc.o: /usr/include/sys/int_limits.h /usr/include/sys/int_const.h
killprc.o: /usr/include/sys/int_fmtio.h /usr/include/arpa/inet.h
killprc.o: /usr/include/netdb.h ../../libsrc/COMM/trclib.h
killprc.o: ../../include/comm_msgtypes.h ../../include/define.h
killprc.o: ../../libsrc/COMM/loglib.h ../../libsrc/COMM/comm_util.h
killprc.o: ../../libsrc/COMM/trclib.h ../../libsrc/COMM/keepalivelib.h
killprc.o: ../../include/sysconf.h ../../libsrc/COMM/comm_proc.h
killprc.o: /usr/include/errno.h /usr/include/sys/shm.h
killprc.o: ../../include/sfm_msgtypes.h ../../include/sfmconf.h
killprc.o: ../../include/ipaf_names.h
ifb_comm.o: ifb_proto.h /usr/include/stdio.h /usr/include/sys/feature_tests.h
ifb_comm.o: /usr/include/sys/ccompile.h /usr/include/sys/isa_defs.h
ifb_comm.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
ifb_comm.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ifb_comm.o: /usr/include/iso/stdio_c99.h /usr/include/stdlib.h
ifb_comm.o: /usr/include/iso/stdlib_iso.h /usr/include/iso/stdlib_c99.h
ifb_comm.o: /usr/include/string.h /usr/include/iso/string_iso.h
ifb_comm.o: /usr/include/strings.h /usr/include/sys/types.h
ifb_comm.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ifb_comm.o: /usr/include/sys/select.h /usr/include/sys/time_impl.h
ifb_comm.o: /usr/include/sys/time.h /usr/include/sys/time.h
ifb_comm.o: /usr/include/sys/errno.h /usr/include/sys/stat.h
ifb_comm.o: /usr/include/sys/stat_impl.h /usr/include/sys/procfs.h
ifb_comm.o: /usr/include/sys/old_procfs.h /usr/include/sys/signal.h
ifb_comm.o: /usr/include/sys/iso/signal_iso.h /usr/include/sys/unistd.h
ifb_comm.o: /usr/include/sys/siginfo.h /usr/include/sys/machsig.h
ifb_comm.o: /usr/include/sys/fault.h /usr/include/sys/syscall.h
ifb_comm.o: /usr/include/sys/procfs_isa.h /usr/include/sys/regset.h
ifb_comm.o: /usr/include/v7/sys/privregs.h /usr/include/v7/sys/psr.h
ifb_comm.o: /usr/include/sys/fsr.h /usr/include/sys/msg.h
ifb_comm.o: /usr/include/sys/ipc.h /usr/include/dirent.h
ifb_comm.o: /usr/include/sys/dirent.h /usr/include/fcntl.h
ifb_comm.o: /usr/include/sys/fcntl.h /usr/include/ctype.h
ifb_comm.o: /usr/include/iso/ctype_iso.h /usr/include/iso/ctype_c99.h
ifb_comm.o: /usr/include/unistd.h /usr/include/stropts.h
ifb_comm.o: /usr/include/sys/stropts.h /usr/include/sys/conf.h
ifb_comm.o: /usr/include/sys/t_lock.h /usr/include/sys/machlock.h
ifb_comm.o: /usr/include/sys/param.h /usr/include/sys/mutex.h
ifb_comm.o: /usr/include/sys/rwlock.h /usr/include/sys/semaphore.h
ifb_comm.o: /usr/include/sys/condvar.h /usr/include/crypt.h
ifb_comm.o: /usr/include/pwd.h ../../include/commlib.h
ifb_comm.o: ../../libsrc/COMM/conflib.h ../../libsrc/COMM/loglib.h
ifb_comm.o: /usr/include/stdarg.h /usr/include/iso/stdarg_iso.h
ifb_comm.o: /usr/include/sys/va_impl.h /usr/include/iso/stdarg_c99.h
ifb_comm.o: ../../libsrc/COMM/socklib.h /usr/include/sys/socket.h
ifb_comm.o: /usr/include/sys/uio.h /usr/include/sys/socket_impl.h
ifb_comm.o: /usr/include/sys/un.h /usr/include/net/if_dl.h
ifb_comm.o: /usr/include/sys/netconfig.h /usr/include/netinet/in.h
ifb_comm.o: /usr/include/sys/stream.h /usr/include/sys/vnode.h
ifb_comm.o: /usr/include/sys/rwstlock.h /usr/include/sys/ksynch.h
ifb_comm.o: /usr/include/sys/cred.h /usr/include/sys/resource.h
ifb_comm.o: /usr/include/vm/seg_enum.h /usr/include/sys/kstat.h
ifb_comm.o: /usr/include/sys/kmem.h /usr/include/sys/vmem.h
ifb_comm.o: /usr/include/sys/poll.h /usr/include/sys/strmdep.h
ifb_comm.o: /usr/include/sys/model.h /usr/include/sys/strft.h
ifb_comm.o: /usr/include/sys/byteorder.h /usr/include/signal.h
ifb_comm.o: /usr/include/iso/signal_iso.h /usr/include/netinet/tcp.h
ifb_comm.o: /usr/include/sys/inttypes.h /usr/include/sys/int_limits.h
ifb_comm.o: /usr/include/sys/int_const.h /usr/include/sys/int_fmtio.h
ifb_comm.o: /usr/include/arpa/inet.h /usr/include/netdb.h
ifb_comm.o: ../../libsrc/COMM/trclib.h ../../include/comm_msgtypes.h
ifb_comm.o: ../../include/define.h ../../libsrc/COMM/loglib.h
ifb_comm.o: ../../libsrc/COMM/comm_util.h ../../libsrc/COMM/trclib.h
ifb_comm.o: ../../libsrc/COMM/keepalivelib.h ../../include/sysconf.h
ifb_comm.o: ../../libsrc/COMM/comm_proc.h /usr/include/errno.h
ifb_comm.o: /usr/include/sys/shm.h ../../include/sfm_msgtypes.h
ifb_comm.o: ../../include/sfmconf.h ../../include/ipaf_names.h
ifb_shm.o: ../../include/ipaf_define.h ../../include/ipaf_shm.h
ifb_shm.o: /usr/include/sys/time.h ../../include/ipaf_names.h
ifb_shm.o: ../../include/ipaf_svc.h /usr/include/sys/time.h
ifb_shm.o: /usr/include/sys/feature_tests.h /usr/include/sys/ccompile.h
ifb_shm.o: /usr/include/sys/isa_defs.h /usr/include/sys/types.h
ifb_shm.o: /usr/include/sys/machtypes.h /usr/include/sys/int_types.h
ifb_shm.o: /usr/include/sys/select.h /usr/include/sys/time_impl.h
ifb_shm.o: ../../include/define.h /usr/include/string.h
ifb_shm.o: /usr/include/iso/string_iso.h ../../include/comm_typedef.h
ifb_shm.o: ../../include/comm_msgtypes.h ifb_proto.h /usr/include/stdio.h
ifb_shm.o: /usr/include/iso/stdio_iso.h /usr/include/sys/va_list.h
ifb_shm.o: /usr/include/stdio_tag.h /usr/include/stdio_impl.h
ifb_shm.o: /usr/include/iso/stdio_c99.h /usr/include/stdlib.h
ifb_shm.o: /usr/include/iso/stdlib_iso.h /usr/include/iso/stdlib_c99.h
ifb_shm.o: /usr/include/strings.h /usr/include/sys/errno.h
ifb_shm.o: /usr/include/sys/stat.h /usr/include/sys/stat_impl.h
ifb_shm.o: /usr/include/sys/procfs.h /usr/include/sys/old_procfs.h
ifb_shm.o: /usr/include/sys/signal.h /usr/include/sys/iso/signal_iso.h
ifb_shm.o: /usr/include/sys/unistd.h /usr/include/sys/siginfo.h
ifb_shm.o: /usr/include/sys/machsig.h /usr/include/sys/fault.h
ifb_shm.o: /usr/include/sys/syscall.h /usr/include/sys/procfs_isa.h
ifb_shm.o: /usr/include/sys/regset.h /usr/include/v7/sys/privregs.h
ifb_shm.o: /usr/include/v7/sys/psr.h /usr/include/sys/fsr.h
ifb_shm.o: /usr/include/sys/msg.h /usr/include/sys/ipc.h
ifb_shm.o: /usr/include/dirent.h /usr/include/sys/dirent.h
ifb_shm.o: /usr/include/fcntl.h /usr/include/sys/fcntl.h /usr/include/ctype.h
ifb_shm.o: /usr/include/iso/ctype_iso.h /usr/include/iso/ctype_c99.h
ifb_shm.o: /usr/include/unistd.h /usr/include/stropts.h
ifb_shm.o: /usr/include/sys/stropts.h /usr/include/sys/conf.h
ifb_shm.o: /usr/include/sys/t_lock.h /usr/include/sys/machlock.h
ifb_shm.o: /usr/include/sys/param.h /usr/include/sys/mutex.h
ifb_shm.o: /usr/include/sys/rwlock.h /usr/include/sys/semaphore.h
ifb_shm.o: /usr/include/sys/condvar.h /usr/include/crypt.h /usr/include/pwd.h
ifb_shm.o: ../../include/commlib.h ../../libsrc/COMM/conflib.h
ifb_shm.o: ../../libsrc/COMM/loglib.h /usr/include/stdarg.h
ifb_shm.o: /usr/include/iso/stdarg_iso.h /usr/include/sys/va_impl.h
ifb_shm.o: /usr/include/iso/stdarg_c99.h ../../libsrc/COMM/socklib.h
ifb_shm.o: /usr/include/sys/socket.h /usr/include/sys/uio.h
ifb_shm.o: /usr/include/sys/socket_impl.h /usr/include/sys/un.h
ifb_shm.o: /usr/include/net/if_dl.h /usr/include/sys/netconfig.h
ifb_shm.o: /usr/include/netinet/in.h /usr/include/sys/stream.h
ifb_shm.o: /usr/include/sys/vnode.h /usr/include/sys/rwstlock.h
ifb_shm.o: /usr/include/sys/ksynch.h /usr/include/sys/cred.h
ifb_shm.o: /usr/include/sys/resource.h /usr/include/vm/seg_enum.h
ifb_shm.o: /usr/include/sys/kstat.h /usr/include/sys/kmem.h
ifb_shm.o: /usr/include/sys/vmem.h /usr/include/sys/poll.h
ifb_shm.o: /usr/include/sys/strmdep.h /usr/include/sys/model.h
ifb_shm.o: /usr/include/sys/strft.h /usr/include/sys/byteorder.h
ifb_shm.o: /usr/include/signal.h /usr/include/iso/signal_iso.h
ifb_shm.o: /usr/include/netinet/tcp.h /usr/include/sys/inttypes.h
ifb_shm.o: /usr/include/sys/int_limits.h /usr/include/sys/int_const.h
ifb_shm.o: /usr/include/sys/int_fmtio.h /usr/include/arpa/inet.h
ifb_shm.o: /usr/include/netdb.h ../../libsrc/COMM/trclib.h
ifb_shm.o: ../../libsrc/COMM/loglib.h ../../libsrc/COMM/comm_util.h
ifb_shm.o: ../../libsrc/COMM/trclib.h ../../libsrc/COMM/keepalivelib.h
ifb_shm.o: ../../include/sysconf.h ../../libsrc/COMM/comm_proc.h
ifb_shm.o: /usr/include/errno.h /usr/include/sys/shm.h
ifb_shm.o: ../../include/sfm_msgtypes.h ../../include/sfmconf.h
