# 1 "shmmb_print.c"
# 1 "<built-in>"
#define __VERSION__ "3.2.3 20030502 (Red Hat Linux 3.2.3-54)"
#define __USER_LABEL_PREFIX__ 
#define __REGISTER_PREFIX__ 
#define __HAVE_BUILTIN_SETJMP__ 1
#define __SIZE_TYPE__ unsigned int
#define __PTRDIFF_TYPE__ int
#define __WCHAR_TYPE__ long int
#define __WINT_TYPE__ unsigned int
#define __STDC__ 1
# 1 "<command line>"
#define __GNUC__ 3
# 1 "<command line>"
#define __GNUC_MINOR__ 2
# 1 "<command line>"
#define __GNUC_PATCHLEVEL__ 3
# 1 "<command line>"
#define __GNUC_RH_RELEASE__ 54
# 1 "<command line>"
#define __GXX_ABI_VERSION 102
# 1 "<command line>"
#define __ELF__ 1
# 1 "<command line>"
#define unix 1
# 1 "<command line>"
#define __gnu_linux__ 1
# 1 "<command line>"
#define linux 1
# 1 "<command line>"
#define __ELF__ 1
# 1 "<command line>"
#define __unix__ 1
# 1 "<command line>"
#define __gnu_linux__ 1
# 1 "<command line>"
#define __linux__ 1
# 1 "<command line>"
#define __unix 1
# 1 "<command line>"
#define __linux 1
# 1 "<command line>"
#define __OPTIMIZE__ 1
# 1 "<command line>"
#define __STDC_HOSTED__ 1
# 1 "<command line>"
#define i386 1
# 1 "<command line>"
#define __i386 1
# 1 "<command line>"
#define __i386__ 1
# 1 "<command line>"
#define __tune_i386__ 1
# 1 "<command line>"
#define PRINT 1
# 1 "<command line>"
#define DEBUG 1
# 1 "shmmb_print.c"
# 1 "/usr/include/stdio.h" 1 3
# 27 "/usr/include/stdio.h" 3
#define _STDIO_H 1
# 1 "/usr/include/features.h" 1 3
# 20 "/usr/include/features.h" 3
#define _FEATURES_H 1
# 110 "/usr/include/features.h" 3
#define __KERNEL_STRICT_NAMES 



#define __USE_ANSI 1
# 151 "/usr/include/features.h" 3
#define _BSD_SOURCE 1
#define _SVID_SOURCE 1
# 168 "/usr/include/features.h" 3
#define _POSIX_SOURCE 1



#define _POSIX_C_SOURCE 199506L




#define __USE_POSIX 1



#define __USE_POSIX2 1



#define __USE_POSIX199309 1



#define __USE_POSIX199506 1
# 228 "/usr/include/features.h" 3
#define __USE_MISC 1



#define __USE_BSD 1



#define __USE_SVID 1
# 248 "/usr/include/features.h" 3
#define __STDC_IEC_559__ 1
#define __STDC_IEC_559_COMPLEX__ 1


#define __STDC_ISO_10646__ 200009L
# 261 "/usr/include/features.h" 3
#define __GNU_LIBRARY__ 6



#define __GLIBC__ 2
#define __GLIBC_MINOR__ 3
# 276 "/usr/include/features.h" 3
#define __GNUC_PREREQ(maj,min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))





#define __GLIBC_PREREQ(maj,min) ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((maj) << 16) + (min))







#define __GLIBC_HAVE_LONG_LONG 1





# 1 "/usr/include/sys/cdefs.h" 1 3
# 20 "/usr/include/sys/cdefs.h" 3
#define _SYS_CDEFS_H 1
# 46 "/usr/include/sys/cdefs.h" 3
#define __THROW 

#define __P(args) args __THROW


#define __PMT(args) args
# 70 "/usr/include/sys/cdefs.h" 3
#define __CONCAT(x,y) x ## y
#define __STRING(x) #x


#define __ptr_t void *
#define __long_double_t long double







#define __BEGIN_DECLS 
#define __END_DECLS 
# 104 "/usr/include/sys/cdefs.h" 3
#define __BEGIN_NAMESPACE_STD 
#define __END_NAMESPACE_STD 
#define __USING_NAMESPACE_STD(name) 
#define __BEGIN_NAMESPACE_C99 
#define __END_NAMESPACE_C99 
#define __USING_NAMESPACE_C99(name) 





#define __bounded 
#define __unbounded 
#define __ptrvalue 






#define __flexarr []
# 151 "/usr/include/sys/cdefs.h" 3
#define __REDIRECT(name,proto,alias) name proto __asm__ (__ASMNAME (#alias))
#define __ASMNAME(cname) __ASMNAME2 (__USER_LABEL_PREFIX__, cname)
#define __ASMNAME2(prefix,cname) __STRING (prefix) cname
# 174 "/usr/include/sys/cdefs.h" 3
#define __attribute_malloc__ __attribute__ ((__malloc__))
# 183 "/usr/include/sys/cdefs.h" 3
#define __attribute_pure__ __attribute__ ((__pure__))
# 192 "/usr/include/sys/cdefs.h" 3
#define __attribute_used__ __attribute__ ((__used__))
#define __attribute_noinline__ __attribute__ ((__noinline__))







#define __attribute_deprecated__ __attribute__ ((__deprecated__))
# 213 "/usr/include/sys/cdefs.h" 3
#define __attribute_format_arg__(x) __attribute__ ((__format_arg__ (x)))
# 223 "/usr/include/sys/cdefs.h" 3
#define __attribute_format_strfmon__(a,b) __attribute__ ((__format__ (__strfmon__, a, b)))
# 246 "/usr/include/sys/cdefs.h" 3
#define __restrict_arr __restrict
# 297 "/usr/include/features.h" 2 3
# 311 "/usr/include/features.h" 3
#define __USE_EXTERN_INLINES 1







# 1 "/usr/include/gnu/stubs.h" 1 3
# 10 "/usr/include/gnu/stubs.h" 3
#define __stub___kernel_cosl 
#define __stub___kernel_sinl 
#define __stub___kernel_tanl 
#define __stub_chflags 
#define __stub_fattach 
#define __stub_fchflags 
#define __stub_fdetach 
#define __stub_gtty 
#define __stub_lchmod 
#define __stub_lutimes 
#define __stub_revoke 
#define __stub_setlogin 
#define __stub_sstk 
#define __stub_stty 
# 320 "/usr/include/features.h" 2 3
# 29 "/usr/include/stdio.h" 2 3



#define __need_size_t 
#define __need_NULL 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 188 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#define __size_t__ 
#define __SIZE_T__ 
#define _SIZE_T 
#define _SYS_SIZE_T_H 
#define _T_SIZE_ 
#define _T_SIZE 
#define __SIZE_T 
#define _SIZE_T_ 
#define _BSD_SIZE_T_ 
#define _SIZE_T_DEFINED_ 
#define _SIZE_T_DEFINED 
#define _BSD_SIZE_T_DEFINED_ 
#define _SIZE_T_DECLARED 
#define ___int_size_t_h 
#define _GCC_SIZE_T 
#define _SIZET_ 



#define __size_t 





typedef unsigned int size_t;
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 402 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#define NULL ((void *)0)





#undef __need_NULL
# 35 "/usr/include/stdio.h" 2 3

# 1 "/usr/include/bits/types.h" 1 3
# 25 "/usr/include/bits/types.h" 3
#define _BITS_TYPES_H 1


# 1 "/usr/include/bits/wordsize.h" 1 3
# 19 "/usr/include/bits/wordsize.h" 3
#define __WORDSIZE 32
# 29 "/usr/include/bits/types.h" 2 3

#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 32 "/usr/include/bits/types.h" 2 3


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;




__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;







__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
# 102 "/usr/include/bits/types.h" 3
#define __S16_TYPE short int
#define __U16_TYPE unsigned short int
#define __S32_TYPE int
#define __U32_TYPE unsigned int
#define __SLONGWORD_TYPE long int
#define __ULONGWORD_TYPE unsigned long int

#define __SQUAD_TYPE long long int
#define __UQUAD_TYPE unsigned long long int
#define __SWORD_TYPE int
#define __UWORD_TYPE unsigned int
#define __SLONG32_TYPE long int
#define __ULONG32_TYPE unsigned long int
#define __S64_TYPE __quad_t
#define __U64_TYPE __u_quad_t
# 129 "/usr/include/bits/types.h" 3
# 1 "/usr/include/bits/typesizes.h" 1 3
# 25 "/usr/include/bits/typesizes.h" 3
#define _BITS_TYPESIZES_H 1




#define __DEV_T_TYPE __UQUAD_TYPE
#define __UID_T_TYPE __U32_TYPE
#define __GID_T_TYPE __U32_TYPE
#define __INO_T_TYPE __ULONGWORD_TYPE
#define __INO64_T_TYPE __UQUAD_TYPE
#define __MODE_T_TYPE __U32_TYPE
#define __NLINK_T_TYPE __UWORD_TYPE
#define __OFF_T_TYPE __SLONGWORD_TYPE
#define __OFF64_T_TYPE __SQUAD_TYPE
#define __PID_T_TYPE __S32_TYPE
#define __RLIM_T_TYPE __ULONGWORD_TYPE
#define __RLIM64_T_TYPE __UQUAD_TYPE
#define __BLKCNT_T_TYPE __SLONGWORD_TYPE
#define __BLKCNT64_T_TYPE __SQUAD_TYPE
#define __FSBLKCNT_T_TYPE __ULONGWORD_TYPE
#define __FSBLKCNT64_T_TYPE __UQUAD_TYPE
#define __FSFILCNT_T_TYPE __ULONGWORD_TYPE
#define __FSFILCNT64_T_TYPE __UQUAD_TYPE
#define __ID_T_TYPE __U32_TYPE
#define __CLOCK_T_TYPE __SLONGWORD_TYPE
#define __TIME_T_TYPE __SLONGWORD_TYPE
#define __USECONDS_T_TYPE __U32_TYPE
#define __SUSECONDS_T_TYPE __SLONGWORD_TYPE
#define __DADDR_T_TYPE __S32_TYPE
#define __SWBLK_T_TYPE __SLONGWORD_TYPE
#define __KEY_T_TYPE __S32_TYPE
#define __CLOCKID_T_TYPE __S32_TYPE
#define __TIMER_T_TYPE __S32_TYPE
#define __BLKSIZE_T_TYPE __SLONGWORD_TYPE
#define __FSID_T_TYPE struct { int __val[2]; }
#define __SSIZE_T_TYPE __SWORD_TYPE


#define __FD_SETSIZE 1024
# 130 "/usr/include/bits/types.h" 2 3



#define __STD_TYPE __extension__ typedef


__extension__ typedef unsigned long long int __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef unsigned long long int __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef long long int __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef unsigned long long int __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;

__extension__ typedef int __daddr_t;
__extension__ typedef long int __swblk_t;
__extension__ typedef int __key_t;


__extension__ typedef int __clockid_t;


__extension__ typedef int __timer_t;


__extension__ typedef long int __blksize_t;




__extension__ typedef long int __blkcnt_t;
__extension__ typedef long long int __blkcnt64_t;


__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef unsigned long long int __fsblkcnt64_t;


__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef unsigned long long int __fsfilcnt64_t;

__extension__ typedef int __ssize_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


__extension__ typedef int __intptr_t;


__extension__ typedef unsigned int __socklen_t;


#undef __STD_TYPE
# 37 "/usr/include/stdio.h" 2 3
#define __need_FILE 
#define __need___FILE 







typedef struct _IO_FILE FILE;







#define __FILE_defined 1

#undef __need_FILE





typedef struct _IO_FILE __FILE;

#define ____FILE_defined 1

#undef __need___FILE



#define _STDIO_USES_IOSTREAM 

# 1 "/usr/include/libio.h" 1 3
# 30 "/usr/include/libio.h" 3
#define _IO_STDIO_H 

# 1 "/usr/include/_G_config.h" 1 3




#define _G_config_h 1




#define __need_size_t 
#define __need_wchar_t 
#define __need_wint_t 
#define __need_NULL 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 264 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#define __wchar_t__ 
#define __WCHAR_T__ 
#define _WCHAR_T 
#define _T_WCHAR_ 
#define _T_WCHAR 
#define __WCHAR_T 
#define _WCHAR_T_ 
#define _BSD_WCHAR_T_ 
#define _WCHAR_T_DEFINED_ 
#define _WCHAR_T_DEFINED 
#define _WCHAR_T_H 
#define ___int_wchar_t_h 
#define __INT_WCHAR_T_H 
#define _GCC_WCHAR_T 
#define _WCHAR_T_DECLARED 
# 291 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef _BSD_WCHAR_T_
# 325 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
typedef long int wchar_t;
# 344 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_wchar_t




#define _WINT_T 




typedef unsigned int wint_t;

#undef __need_wint_t
# 397 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 15 "/usr/include/_G_config.h" 2 3
# 23 "/usr/include/_G_config.h" 3
#define __need_mbstate_t 
# 1 "/usr/include/wchar.h" 1 3
# 47 "/usr/include/wchar.h" 3
#define __need_wint_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 356 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_wint_t
# 49 "/usr/include/wchar.h" 2 3

# 1 "/usr/include/bits/wchar.h" 1 3
# 21 "/usr/include/bits/wchar.h" 3
#define _BITS_WCHAR_H 1

#define __WCHAR_MIN (-2147483647l - 1l)
#define __WCHAR_MAX (2147483647l)
# 51 "/usr/include/wchar.h" 2 3
# 74 "/usr/include/wchar.h" 3
#define __mbstate_t_defined 1

typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;

#undef __need_mbstate_t
# 25 "/usr/include/_G_config.h" 2 3
#define _G_size_t size_t
typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;
#define _G_ssize_t __ssize_t
#define _G_off_t __off_t
#define _G_off64_t __off64_t
#define _G_pid_t __pid_t
#define _G_uid_t __uid_t
#define _G_wchar_t wchar_t
#define _G_wint_t wint_t
#define _G_stat64 stat64
# 1 "/usr/include/gconv.h" 1 3
# 24 "/usr/include/gconv.h" 3
#define _GCONV_H 1


#define __need_mbstate_t 
# 1 "/usr/include/wchar.h" 1 3
# 47 "/usr/include/wchar.h" 3
#define __need_wint_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 356 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_wint_t
# 49 "/usr/include/wchar.h" 2 3
# 86 "/usr/include/wchar.h" 3
#undef __need_mbstate_t
# 29 "/usr/include/gconv.h" 2 3
#define __need_size_t 
#define __need_wchar_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 344 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_wchar_t
# 32 "/usr/include/gconv.h" 2 3


#define __UNKNOWN_10646_CHAR ((wchar_t) 0xfffd)


enum
{
  __GCONV_OK = 0,
  __GCONV_NOCONV,
  __GCONV_NODB,
  __GCONV_NOMEM,

  __GCONV_EMPTY_INPUT,
  __GCONV_FULL_OUTPUT,
  __GCONV_ILLEGAL_INPUT,
  __GCONV_INCOMPLETE_INPUT,

  __GCONV_ILLEGAL_DESCRIPTOR,
  __GCONV_INTERNAL_ERROR
};



enum
{
  __GCONV_IS_LAST = 0x0001,
  __GCONV_IGNORE_ERRORS = 0x0002
};



struct __gconv_step;
struct __gconv_step_data;
struct __gconv_loaded_object;
struct __gconv_trans_data;



typedef int (*__gconv_fct) (struct __gconv_step *, struct __gconv_step_data *,
                            __const unsigned char **, __const unsigned char *,
                            unsigned char **, size_t *, int, int);


typedef wint_t (*__gconv_btowc_fct) (struct __gconv_step *, unsigned char);


typedef int (*__gconv_init_fct) (struct __gconv_step *);
typedef void (*__gconv_end_fct) (struct __gconv_step *);



typedef int (*__gconv_trans_fct) (struct __gconv_step *,
                                  struct __gconv_step_data *, void *,
                                  __const unsigned char *,
                                  __const unsigned char **,
                                  __const unsigned char *, unsigned char **,
                                  size_t *);


typedef int (*__gconv_trans_context_fct) (void *, __const unsigned char *,
                                          __const unsigned char *,
                                          unsigned char *, unsigned char *);


typedef int (*__gconv_trans_query_fct) (__const char *, __const char ***,
                                        size_t *);


typedef int (*__gconv_trans_init_fct) (void **, const char *);
typedef void (*__gconv_trans_end_fct) (void *);

struct __gconv_trans_data
{

  __gconv_trans_fct __trans_fct;
  __gconv_trans_context_fct __trans_context_fct;
  __gconv_trans_end_fct __trans_end_fct;
  void *__data;
  struct __gconv_trans_data *__next;
};



struct __gconv_step
{
  struct __gconv_loaded_object *__shlib_handle;
  __const char *__modname;

  int __counter;

  char *__from_name;
  char *__to_name;

  __gconv_fct __fct;
  __gconv_btowc_fct __btowc_fct;
  __gconv_init_fct __init_fct;
  __gconv_end_fct __end_fct;



  int __min_needed_from;
  int __max_needed_from;
  int __min_needed_to;
  int __max_needed_to;


  int __stateful;

  void *__data;
};



struct __gconv_step_data
{
  unsigned char *__outbuf;
  unsigned char *__outbufend;



  int __flags;



  int __invocation_counter;



  int __internal_use;

  __mbstate_t *__statep;
  __mbstate_t __state;



  struct __gconv_trans_data *__trans;
};



typedef struct __gconv_info
{
  size_t __nsteps;
  struct __gconv_step *__steps;
  __extension__ struct __gconv_step_data __data [];
} *__gconv_t;
# 45 "/usr/include/_G_config.h" 2 3
typedef union
{
  struct __gconv_info __cd;
  struct
  {
    struct __gconv_info __cd;
    struct __gconv_step_data __data;
  } __combined;
} _G_iconv_t;

typedef int _G_int16_t __attribute__ ((__mode__ (__HI__)));
typedef int _G_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int _G_uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int _G_uint32_t __attribute__ ((__mode__ (__SI__)));

#define _G_HAVE_BOOL 1



#define _G_HAVE_ATEXIT 1
#define _G_HAVE_SYS_CDEFS 1
#define _G_HAVE_SYS_WAIT 1
#define _G_NEED_STDARG_H 1
#define _G_va_list __gnuc_va_list

#define _G_HAVE_PRINTF_FP 1
#define _G_HAVE_MMAP 1
#define _G_HAVE_LONG_DOUBLE_IO 1
#define _G_HAVE_IO_FILE_OPEN 1
#define _G_HAVE_IO_GETLINE_INFO 1

#define _G_IO_IO_FILE_VERSION 0x20001

#define _G_OPEN64 __open64
#define _G_LSEEK64 __lseek64
#define _G_MMAP64 __mmap64
#define _G_FSTAT64(fd,buf) __fxstat64 (_STAT_VER, fd, buf)


#define _G_HAVE_ST_BLKSIZE defined (_STATBUF_ST_BLKSIZE)

#define _G_BUFSIZ 8192


#define _G_NAMES_HAVE_UNDERSCORE 0
#define _G_VTABLE_LABEL_HAS_LENGTH 1
#define _G_USING_THUNKS 1
#define _G_VTABLE_LABEL_PREFIX "__vt_"
#define _G_VTABLE_LABEL_PREFIX_ID __vt_



#define _G_ARGS(ARGLIST) ARGLIST
# 33 "/usr/include/libio.h" 2 3

#define _IO_pos_t _G_fpos_t
#define _IO_fpos_t _G_fpos_t
#define _IO_fpos64_t _G_fpos64_t
#define _IO_size_t _G_size_t
#define _IO_ssize_t _G_ssize_t
#define _IO_off_t _G_off_t
#define _IO_off64_t _G_off64_t
#define _IO_pid_t _G_pid_t
#define _IO_uid_t _G_uid_t
#define _IO_iconv_t _G_iconv_t
#define _IO_HAVE_SYS_WAIT _G_HAVE_SYS_WAIT
#define _IO_HAVE_ST_BLKSIZE _G_HAVE_ST_BLKSIZE
#define _IO_BUFSIZ _G_BUFSIZ
#define _IO_va_list _G_va_list
#define _IO_wint_t _G_wint_t



#define __need___va_list 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stdarg.h" 1 3
# 37 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stdarg.h" 3
#undef __need___va_list




#define __GNUC_VA_LIST 
typedef __builtin_va_list __gnuc_va_list;
# 54 "/usr/include/libio.h" 2 3

#undef _IO_va_list
#define _IO_va_list __gnuc_va_list
# 76 "/usr/include/libio.h" 3
#define _PARAMS(protos) __P(protos)







#define _IO_UNIFIED_JUMPTABLES 1





#define EOF (-1)
# 105 "/usr/include/libio.h" 3
#define _IOS_INPUT 1
#define _IOS_OUTPUT 2
#define _IOS_ATEND 4
#define _IOS_APPEND 8
#define _IOS_TRUNC 16
#define _IOS_NOCREATE 32
#define _IOS_NOREPLACE 64
#define _IOS_BIN 128







#define _IO_MAGIC 0xFBAD0000
#define _OLD_STDIO_MAGIC 0xFABC0000
#define _IO_MAGIC_MASK 0xFFFF0000
#define _IO_USER_BUF 1
#define _IO_UNBUFFERED 2
#define _IO_NO_READS 4
#define _IO_NO_WRITES 8
#define _IO_EOF_SEEN 0x10
#define _IO_ERR_SEEN 0x20
#define _IO_DELETE_DONT_CLOSE 0x40
#define _IO_LINKED 0x80
#define _IO_IN_BACKUP 0x100
#define _IO_LINE_BUF 0x200
#define _IO_TIED_PUT_GET 0x400
#define _IO_CURRENTLY_PUTTING 0x800
#define _IO_IS_APPENDING 0x1000
#define _IO_IS_FILEBUF 0x2000
#define _IO_BAD_SEEN 0x4000
#define _IO_USER_LOCK 0x8000

#define _IO_FLAGS2_MMAP 1
#define _IO_FLAGS2_NOTCANCEL 2


#define _IO_SKIPWS 01
#define _IO_LEFT 02
#define _IO_RIGHT 04
#define _IO_INTERNAL 010
#define _IO_DEC 020
#define _IO_OCT 040
#define _IO_HEX 0100
#define _IO_SHOWBASE 0200
#define _IO_SHOWPOINT 0400
#define _IO_UPPERCASE 01000
#define _IO_SHOWPOS 02000
#define _IO_SCIENTIFIC 04000
#define _IO_FIXED 010000
#define _IO_UNITBUF 020000
#define _IO_STDIO 040000
#define _IO_DONT_CLOSE 0100000
#define _IO_BOOLALPHA 0200000


struct _IO_jump_t; struct _IO_FILE;
# 173 "/usr/include/libio.h" 3
typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;
# 196 "/usr/include/libio.h" 3
};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};
# 264 "/usr/include/libio.h" 3
struct _IO_FILE {
  int _flags;
#define _IO_file_flags _flags



  char* _IO_read_ptr;
  char* _IO_read_end;
  char* _IO_read_base;
  char* _IO_write_base;
  char* _IO_write_ptr;
  char* _IO_write_end;
  char* _IO_buf_base;
  char* _IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;



  int _flags2;

  __off_t _old_offset;

#define __HAVE_COLUMN 

  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];



  _IO_lock_t *_lock;
# 312 "/usr/include/libio.h" 3
  __off64_t _offset;





  void *__pad1;
  void *__pad2;

  int _mode;

  char _unused2[15 * sizeof (int) - 2 * sizeof (void *)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;

#define _IO_stdin ((_IO_FILE*)(&_IO_2_1_stdin_))
#define _IO_stdout ((_IO_FILE*)(&_IO_2_1_stdout_))
#define _IO_stderr ((_IO_FILE*)(&_IO_2_1_stderr_))
# 351 "/usr/include/libio.h" 3
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, __const char *__buf,
                                 size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);
# 403 "/usr/include/libio.h" 3
extern int __underflow (_IO_FILE *) ;
extern int __uflow (_IO_FILE *) ;
extern int __overflow (_IO_FILE *, int) ;
extern wint_t __wunderflow (_IO_FILE *) ;
extern wint_t __wuflow (_IO_FILE *) ;
extern wint_t __woverflow (_IO_FILE *, wint_t) ;

#define _IO_getc_unlocked(_fp) ((_fp)->_IO_read_ptr >= (_fp)->_IO_read_end ? __uflow (_fp) : *(unsigned char *) (_fp)->_IO_read_ptr++)


#define _IO_peekc_unlocked(_fp) ((_fp)->_IO_read_ptr >= (_fp)->_IO_read_end && __underflow (_fp) == EOF ? EOF : *(unsigned char *) (_fp)->_IO_read_ptr)



#define _IO_putc_unlocked(_ch,_fp) (((_fp)->_IO_write_ptr >= (_fp)->_IO_write_end) ? __overflow (_fp, (unsigned char) (_ch)) : (unsigned char) (*(_fp)->_IO_write_ptr++ = (_ch)))




#define _IO_getwc_unlocked(_fp) ((_fp)->_wide_data->_IO_read_ptr >= (_fp)->_wide_data->_IO_read_end ? __wuflow (_fp) : (_IO_wint_t) *(_fp)->_wide_data->_IO_read_ptr++)


#define _IO_putwc_unlocked(_wch,_fp) ((_fp)->_wide_data->_IO_write_ptr >= (_fp)->_wide_data->_IO_write_end ? __woverflow (_fp, _wch) : (_IO_wint_t) (*(_fp)->_wide_data->_IO_write_ptr++ = (_wch)))




#define _IO_feof_unlocked(__fp) (((__fp)->_flags & _IO_EOF_SEEN) != 0)
#define _IO_ferror_unlocked(__fp) (((__fp)->_flags & _IO_ERR_SEEN) != 0)

extern int _IO_getc (_IO_FILE *__fp) ;
extern int _IO_putc (int __c, _IO_FILE *__fp) ;
extern int _IO_feof (_IO_FILE *__fp) ;
extern int _IO_ferror (_IO_FILE *__fp) ;

extern int _IO_peekc_locked (_IO_FILE *__fp) ;


#define _IO_PENDING_OUTPUT_COUNT(_fp) ((_fp)->_IO_write_ptr - (_fp)->_IO_write_base)


extern void _IO_flockfile (_IO_FILE *) ;
extern void _IO_funlockfile (_IO_FILE *) ;
extern int _IO_ftrylockfile (_IO_FILE *) ;
# 455 "/usr/include/libio.h" 3
#define _IO_peekc(_fp) _IO_peekc_unlocked (_fp)
#define _IO_flockfile(_fp) 
#define _IO_funlockfile(_fp) 
#define _IO_ftrylockfile(_fp) 
#define _IO_cleanup_region_start(_fct,_fp) 
#define _IO_cleanup_region_end(_Doit) 


extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
                        __gnuc_va_list, int *__restrict) ;
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
                         __gnuc_va_list) ;
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t) ;
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t) ;

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int) ;
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int) ;

extern void _IO_free_backup_area (_IO_FILE *) ;
# 73 "/usr/include/stdio.h" 2 3
# 86 "/usr/include/stdio.h" 3


typedef _G_fpos_t fpos_t;









#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2




#define BUFSIZ _IO_BUFSIZ
# 118 "/usr/include/stdio.h" 3
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2




#define P_tmpdir "/tmp"
# 138 "/usr/include/stdio.h" 3
# 1 "/usr/include/bits/stdio_lim.h" 1 3
# 24 "/usr/include/bits/stdio_lim.h" 3
#define L_tmpnam 20
#define TMP_MAX 238328
#define FILENAME_MAX 4096


#define L_ctermid 9
#define L_cuserid 9





#define FOPEN_MAX 16
# 139 "/usr/include/stdio.h" 2 3



extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;


#define stdin stdin
#define stdout stdout
#define stderr stderr




extern int remove (__const char *__filename) ;

extern int rename (__const char *__old, __const char *__new) ;









extern FILE *tmpfile (void);
# 180 "/usr/include/stdio.h" 3
extern char *tmpnam (char *__s) ;





extern char *tmpnam_r (char *__s) ;
# 198 "/usr/include/stdio.h" 3
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__malloc__));








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

# 223 "/usr/include/stdio.h" 3
extern int fflush_unlocked (FILE *__stream);
# 237 "/usr/include/stdio.h" 3






extern FILE *fopen (__const char *__restrict __filename,
                    __const char *__restrict __modes);




extern FILE *freopen (__const char *__restrict __filename,
                      __const char *__restrict __modes,
                      FILE *__restrict __stream);
# 264 "/usr/include/stdio.h" 3

# 275 "/usr/include/stdio.h" 3
extern FILE *fdopen (int __fd, __const char *__modes) ;
# 296 "/usr/include/stdio.h" 3



extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) ;



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
                    int __modes, size_t __n) ;





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
                       size_t __size) ;


extern void setlinebuf (FILE *__stream) ;








extern int fprintf (FILE *__restrict __stream,
                    __const char *__restrict __format, ...);




extern int printf (__const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
                    __const char *__restrict __format, ...) ;





extern int vfprintf (FILE *__restrict __s, __const char *__restrict __format,
                     __gnuc_va_list __arg);




extern int vprintf (__const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, __const char *__restrict __format,
                     __gnuc_va_list __arg) ;





extern int snprintf (char *__restrict __s, size_t __maxlen,
                     __const char *__restrict __format, ...)
     __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
                      __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 3, 0)));

# 390 "/usr/include/stdio.h" 3





extern int fscanf (FILE *__restrict __stream,
                   __const char *__restrict __format, ...);




extern int scanf (__const char *__restrict __format, ...);

extern int sscanf (__const char *__restrict __s,
                   __const char *__restrict __format, ...) ;

# 432 "/usr/include/stdio.h" 3





extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);




#define getc(_fp) _IO_getc (_fp)






extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
# 467 "/usr/include/stdio.h" 3
extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);




#define putc(_ch,_fp) _IO_putc (_ch, _fp)
# 500 "/usr/include/stdio.h" 3
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream);






extern char *gets (char *__s);

# 580 "/usr/include/stdio.h" 3





extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
                     size_t __n, FILE *__restrict __stream);




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
                      size_t __n, FILE *__restrict __s);

# 633 "/usr/include/stdio.h" 3
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
                              size_t __n, FILE *__restrict __stream);
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
                               size_t __n, FILE *__restrict __stream);








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream);




extern void rewind (FILE *__stream);

# 688 "/usr/include/stdio.h" 3






extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, __const fpos_t *__pos);
# 711 "/usr/include/stdio.h" 3

# 720 "/usr/include/stdio.h" 3


extern void clearerr (FILE *__stream) ;

extern int feof (FILE *__stream) ;

extern int ferror (FILE *__stream) ;




extern void clearerr_unlocked (FILE *__stream) ;
extern int feof_unlocked (FILE *__stream) ;
extern int ferror_unlocked (FILE *__stream) ;








extern void perror (__const char *__s);






# 1 "/usr/include/bits/sys_errlist.h" 1 3
# 27 "/usr/include/bits/sys_errlist.h" 3
extern int sys_nerr;
extern __const char *__const sys_errlist[];
# 750 "/usr/include/stdio.h" 2 3




extern int fileno (FILE *__stream) ;




extern int fileno_unlocked (FILE *__stream) ;
# 769 "/usr/include/stdio.h" 3
extern FILE *popen (__const char *__command, __const char *__modes);





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) ;
# 809 "/usr/include/stdio.h" 3
extern void flockfile (FILE *__stream) ;



extern int ftrylockfile (FILE *__stream) ;


extern void funlockfile (FILE *__stream) ;
# 830 "/usr/include/stdio.h" 3
# 1 "/usr/include/bits/stdio.h" 1 3
# 27 "/usr/include/bits/stdio.h" 3
#define __STDIO_INLINE extern __inline





extern __inline int
vprintf (__const char *__restrict __fmt, __gnuc_va_list __arg)
{
  return vfprintf (stdout, __fmt, __arg);
}


extern __inline int
getchar (void)
{
  return _IO_getc (stdin);
}




extern __inline int
getc_unlocked (FILE *__fp)
{
  return ((__fp)->_IO_read_ptr >= (__fp)->_IO_read_end ? __uflow (__fp) : *(unsigned char *) (__fp)->_IO_read_ptr++);
}


extern __inline int
getchar_unlocked (void)
{
  return ((stdin)->_IO_read_ptr >= (stdin)->_IO_read_end ? __uflow (stdin) : *(unsigned char *) (stdin)->_IO_read_ptr++);
}




extern __inline int
putchar (int __c)
{
  return _IO_putc (__c, stdout);
}




extern __inline int
fputc_unlocked (int __c, FILE *__stream)
{
  return (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}





extern __inline int
putc_unlocked (int __c, FILE *__stream)
{
  return (((__stream)->_IO_write_ptr >= (__stream)->_IO_write_end) ? __overflow (__stream, (unsigned char) (__c)) : (unsigned char) (*(__stream)->_IO_write_ptr++ = (__c)));
}


extern __inline int
putchar_unlocked (int __c)
{
  return (((stdout)->_IO_write_ptr >= (stdout)->_IO_write_end) ? __overflow (stdout, (unsigned char) (__c)) : (unsigned char) (*(stdout)->_IO_write_ptr++ = (__c)));
}
# 111 "/usr/include/bits/stdio.h" 3
extern __inline int
feof_unlocked (FILE *__stream)
{
  return (((__stream)->_flags & 0x10) != 0);
}


extern __inline int
ferror_unlocked (FILE *__stream)
{
  return (((__stream)->_flags & 0x20) != 0);
}







#define fread_unlocked(ptr,size,n,stream) (__extension__ ((__builtin_constant_p (size) && __builtin_constant_p (n) && (size_t) (size) * (size_t) (n) <= 8 && (size_t) (size) != 0) ? ({ char *__ptr = (char *) (ptr); FILE *__stream = (stream); size_t __cnt; for (__cnt = (size_t) (size) * (size_t) (n); __cnt > 0; --__cnt) { int __c = _IO_getc_unlocked (__stream); if (__c == EOF) break; *__ptr++ = __c; } ((size_t) (size) * (size_t) (n) - __cnt) / (size_t) (size); }) : (((__builtin_constant_p (size) && (size_t) (size) == 0) || (__builtin_constant_p (n) && (size_t) (n) == 0)) ? ((void) (ptr), (void) (stream), (void) (size), (void) (n), (size_t) 0) : fread_unlocked (ptr, size, n, stream))))
# 154 "/usr/include/bits/stdio.h" 3
#define fwrite_unlocked(ptr,size,n,stream) (__extension__ ((__builtin_constant_p (size) && __builtin_constant_p (n) && (size_t) (size) * (size_t) (n) <= 8 && (size_t) (size) != 0) ? ({ const char *__ptr = (const char *) (ptr); FILE *__stream = (stream); size_t __cnt; for (__cnt = (size_t) (size) * (size_t) (n); __cnt > 0; --__cnt) if (_IO_putc_unlocked (*__ptr++, __stream) == EOF) break; ((size_t) (size) * (size_t) (n) - __cnt) / (size_t) (size); }) : (((__builtin_constant_p (size) && (size_t) (size) == 0) || (__builtin_constant_p (n) && (size_t) (n) == 0)) ? ((void) (ptr), (void) (stream), (void) (size), (void) (n), (size_t) 0) : fwrite_unlocked (ptr, size, n, stream))))
# 176 "/usr/include/bits/stdio.h" 3
#undef __STDIO_INLINE
# 831 "/usr/include/stdio.h" 2 3



# 2 "shmmb_print.c" 2
# 1 "/usr/include/sys/socket.h" 1 3
# 21 "/usr/include/sys/socket.h" 3
#define _SYS_SOCKET_H 1





# 1 "/usr/include/sys/uio.h" 1 3
# 20 "/usr/include/sys/uio.h" 3
#define _SYS_UIO_H 1



# 1 "/usr/include/sys/types.h" 1 3
# 25 "/usr/include/sys/types.h" 3
#define _SYS_TYPES_H 1









typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;
#define __u_char_defined 



typedef __loff_t loff_t;



typedef __ino_t ino_t;



#define __ino_t_defined 







typedef __dev_t dev_t;
#define __dev_t_defined 



typedef __gid_t gid_t;
#define __gid_t_defined 



typedef __mode_t mode_t;
#define __mode_t_defined 



typedef __nlink_t nlink_t;
#define __nlink_t_defined 



typedef __uid_t uid_t;
#define __uid_t_defined 




typedef __off_t off_t;



#define __off_t_defined 







typedef __pid_t pid_t;
#define __pid_t_defined 



typedef __id_t id_t;
#define __id_t_defined 



typedef __ssize_t ssize_t;
#define __ssize_t_defined 




typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;
#define __daddr_t_defined 




typedef __key_t key_t;
#define __key_t_defined 





#define __need_time_t 
#define __need_timer_t 
#define __need_clockid_t 
# 1 "/usr/include/time.h" 1 3
# 70 "/usr/include/time.h" 3
#define __time_t_defined 1





typedef __time_t time_t;






#undef __need_time_t



#define __clockid_t_defined 1




typedef __clockid_t clockid_t;






#define __timer_t_defined 1




typedef __timer_t timer_t;


#undef __need_timer_t
# 134 "/usr/include/sys/types.h" 2 3
# 146 "/usr/include/sys/types.h" 3
#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 148 "/usr/include/sys/types.h" 2 3



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
# 184 "/usr/include/sys/types.h" 3
#define __intN_t(N,MODE) typedef int int ##N ##_t __attribute__ ((__mode__ (MODE)))

#define __u_intN_t(N,MODE) typedef unsigned int u_int ##N ##_t __attribute__ ((__mode__ (MODE)))



#define __int8_t_defined 
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));





#define __BIT_TYPES_DEFINED__ 1




# 1 "/usr/include/endian.h" 1 3
# 20 "/usr/include/endian.h" 3
#define _ENDIAN_H 1
# 32 "/usr/include/endian.h" 3
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __PDP_ENDIAN 3412


# 1 "/usr/include/bits/endian.h" 1 3






#define __BYTE_ORDER __LITTLE_ENDIAN
# 38 "/usr/include/endian.h" 2 3




#define __FLOAT_WORD_ORDER __BYTE_ORDER



#define LITTLE_ENDIAN __LITTLE_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#define PDP_ENDIAN __PDP_ENDIAN
#define BYTE_ORDER __BYTE_ORDER



#define __LONG_LONG_PAIR(HI,LO) LO, HI
# 214 "/usr/include/sys/types.h" 2 3


# 1 "/usr/include/sys/select.h" 1 3
# 23 "/usr/include/sys/select.h" 3
#define _SYS_SELECT_H 1







# 1 "/usr/include/bits/select.h" 1 3
# 26 "/usr/include/bits/select.h" 3
#define __FD_ZERO(fdsp) do { int __d0, __d1; __asm__ __volatile__ ("cld; rep; stosl" : "=c" (__d0), "=D" (__d1) : "a" (0), "0" (sizeof (fd_set) / sizeof (__fd_mask)), "1" (&__FDS_BITS (fdsp)[0]) : "memory"); } while (0)
# 37 "/usr/include/bits/select.h" 3
#define __FD_SET(fd,fdsp) __asm__ __volatile__ ("btsl %1,%0" : "=m" (__FDS_BITS (fdsp)[__FDELT (fd)]) : "r" (((int) (fd)) % __NFDBITS) : "cc","memory")




#define __FD_CLR(fd,fdsp) __asm__ __volatile__ ("btrl %1,%0" : "=m" (__FDS_BITS (fdsp)[__FDELT (fd)]) : "r" (((int) (fd)) % __NFDBITS) : "cc","memory")




#define __FD_ISSET(fd,fdsp) (__extension__ ({register char __result; __asm__ __volatile__ ("btl %1,%2 ; setcb %b0" : "=q" (__result) : "r" (((int) (fd)) % __NFDBITS), "m" (__FDS_BITS (fdsp)[__FDELT (fd)]) : "cc"); __result; }))
# 32 "/usr/include/sys/select.h" 2 3


# 1 "/usr/include/bits/sigset.h" 1 3
# 21 "/usr/include/bits/sigset.h" 3
#define _SIGSET_H_types 1

typedef int __sig_atomic_t;



#define _SIGSET_NWORDS (1024 / (8 * sizeof (unsigned long int)))
typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
# 35 "/usr/include/sys/select.h" 2 3


#define __sigset_t_defined 
typedef __sigset_t sigset_t;



#define __need_time_t 
#define __need_timespec 
# 1 "/usr/include/time.h" 1 3
# 83 "/usr/include/time.h" 3
#undef __need_time_t
# 114 "/usr/include/time.h" 3
#define __timespec_defined 1



struct timespec
  {
    __time_t tv_sec;
    long int tv_nsec;
  };


#undef __need_timespec
# 45 "/usr/include/sys/select.h" 2 3
#define __need_timeval 
# 1 "/usr/include/bits/time.h" 1 3
# 62 "/usr/include/bits/time.h" 3
#undef __need_timeval

#define _STRUCT_TIMEVAL 1




struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
# 47 "/usr/include/sys/select.h" 2 3


typedef __suseconds_t suseconds_t;
#define __suseconds_t_defined 




typedef long int __fd_mask;






#define __NFDBITS (8 * sizeof (__fd_mask))
#define __FDELT(d) ((d) / __NFDBITS)
#define __FDMASK(d) ((__fd_mask) 1 << ((d) % __NFDBITS))


typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * sizeof (__fd_mask))];
#define __FDS_BITS(set) ((set)->__fds_bits)

  } fd_set;


#define FD_SETSIZE __FD_SETSIZE



typedef __fd_mask fd_mask;


#define NFDBITS __NFDBITS




#define FD_SET(fd,fdsetp) __FD_SET (fd, fdsetp)
#define FD_CLR(fd,fdsetp) __FD_CLR (fd, fdsetp)
#define FD_ISSET(fd,fdsetp) __FD_ISSET (fd, fdsetp)
#define FD_ZERO(fdsetp) __FD_ZERO (fdsetp)



# 109 "/usr/include/sys/select.h" 3
extern int select (int __nfds, fd_set *__restrict __readfds,
                   fd_set *__restrict __writefds,
                   fd_set *__restrict __exceptfds,
                   struct timeval *__restrict __timeout);
# 128 "/usr/include/sys/select.h" 3

# 217 "/usr/include/sys/types.h" 2 3


# 1 "/usr/include/sys/sysmacros.h" 1 3
# 21 "/usr/include/sys/sysmacros.h" 3
#define _SYS_SYSMACROS_H 1







__extension__
extern __inline unsigned int gnu_dev_major (unsigned long long int __dev)
     ;
__extension__
extern __inline unsigned int gnu_dev_minor (unsigned long long int __dev)
     ;
__extension__
extern __inline unsigned long long int gnu_dev_makedev (unsigned int __major,
                                                        unsigned int __minor)
     ;


__extension__ extern __inline unsigned int
gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}

__extension__ extern __inline unsigned int
gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}

__extension__ extern __inline unsigned long long int
gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
          | (((unsigned long long int) (__minor & ~0xff)) << 12)
          | (((unsigned long long int) (__major & ~0xfff)) << 32));
}




#define major(dev) gnu_dev_major (dev)
#define minor(dev) gnu_dev_minor (dev)
#define makedev(maj,min) gnu_dev_makedev (maj, min)
# 220 "/usr/include/sys/types.h" 2 3
# 231 "/usr/include/sys/types.h" 3
typedef __blkcnt_t blkcnt_t;
#define __blkcnt_t_defined 


typedef __fsblkcnt_t fsblkcnt_t;
#define __fsblkcnt_t_defined 


typedef __fsfilcnt_t fsfilcnt_t;
#define __fsfilcnt_t_defined 
# 266 "/usr/include/sys/types.h" 3
# 1 "/usr/include/bits/pthreadtypes.h" 1 3
# 20 "/usr/include/bits/pthreadtypes.h" 3
#define _BITS_PTHREADTYPES_H 1

#define __need_schedparam 
# 1 "/usr/include/bits/sched.h" 1 3
# 81 "/usr/include/bits/sched.h" 3
#define __defined_schedparam 1

struct __sched_param
  {
    int __sched_priority;
  };
#undef __need_schedparam
# 24 "/usr/include/bits/pthreadtypes.h" 2 3


struct _pthread_fastlock
{
  long int __status;
  int __spinlock;

};



typedef struct _pthread_descr_struct *_pthread_descr;
#define _PTHREAD_DESCR_DEFINED 




typedef struct __pthread_attr_s
{
  int __detachstate;
  int __schedpolicy;
  struct __sched_param __schedparam;
  int __inheritsched;
  int __scope;
  size_t __guardsize;
  int __stackaddr_set;
  void *__stackaddr;
  size_t __stacksize;
} pthread_attr_t;





__extension__ typedef long long __pthread_cond_align_t;




typedef struct
{
  struct _pthread_fastlock __c_lock;
  _pthread_descr __c_waiting;
  char __padding[48 - sizeof (struct _pthread_fastlock)
                 - sizeof (_pthread_descr) - sizeof (__pthread_cond_align_t)];
  __pthread_cond_align_t __align;
} pthread_cond_t;



typedef struct
{
  int __dummy;
} pthread_condattr_t;


typedef unsigned int pthread_key_t;





typedef struct
{
  int __m_reserved;
  int __m_count;
  _pthread_descr __m_owner;
  int __m_kind;
  struct _pthread_fastlock __m_lock;
} pthread_mutex_t;



typedef struct
{
  int __mutexkind;
} pthread_mutexattr_t;



typedef int pthread_once_t;
# 150 "/usr/include/bits/pthreadtypes.h" 3
typedef unsigned long int pthread_t;
# 267 "/usr/include/sys/types.h" 2 3



# 25 "/usr/include/sys/uio.h" 2 3




# 1 "/usr/include/bits/uio.h" 1 3
# 38 "/usr/include/bits/uio.h" 3
#define UIO_MAXIOV 1024



struct iovec
  {
    void *iov_base;
    size_t iov_len;
  };
# 30 "/usr/include/sys/uio.h" 2 3
# 40 "/usr/include/sys/uio.h" 3
extern ssize_t readv (int __fd, __const struct iovec *__vector, int __count);
# 50 "/usr/include/sys/uio.h" 3
extern ssize_t writev (int __fd, __const struct iovec *__vector, int __count);


# 28 "/usr/include/sys/socket.h" 2 3
#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 30 "/usr/include/sys/socket.h" 2 3





# 1 "/usr/include/bits/socket.h" 1 3
# 21 "/usr/include/bits/socket.h" 3
#define __BITS_SOCKET_H 





#define __need_size_t 
#define __need_NULL 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 397 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 30 "/usr/include/bits/socket.h" 2 3

# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 1 3






#define _GCC_LIMITS_H_ 



# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/syslimits.h" 1 3





#define _GCC_NEXT_LIMITS_H 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 1 3
# 132 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 3
# 1 "/usr/include/limits.h" 1 3
# 24 "/usr/include/limits.h" 3
#define _LIBC_LIMITS_H_ 1







#define MB_LEN_MAX 16
# 144 "/usr/include/limits.h" 3
# 1 "/usr/include/bits/posix1_lim.h" 1 3
# 26 "/usr/include/bits/posix1_lim.h" 3
#define _BITS_POSIX1_LIM_H 1





#define _POSIX_AIO_LISTIO_MAX 2


#define _POSIX_AIO_MAX 1


#define _POSIX_ARG_MAX 4096





#define _POSIX_CHILD_MAX 6



#define _POSIX_DELAYTIMER_MAX 32



#define _POSIX_HOST_NAME_MAX 255


#define _POSIX_LINK_MAX 8


#define _POSIX_LOGIN_NAME_MAX 9


#define _POSIX_MAX_CANON 255



#define _POSIX_MAX_INPUT 255


#define _POSIX_MQ_OPEN_MAX 8


#define _POSIX_MQ_PRIO_MAX 32


#define _POSIX_NAME_MAX 14





#define _POSIX_NGROUPS_MAX 0






#define _POSIX_OPEN_MAX 16




#define _POSIX_FD_SETSIZE _POSIX_OPEN_MAX


#define _POSIX_PATH_MAX 256


#define _POSIX_PIPE_BUF 512



#define _POSIX_RE_DUP_MAX 255


#define _POSIX_RTSIG_MAX 8


#define _POSIX_SEM_NSEMS_MAX 256


#define _POSIX_SEM_VALUE_MAX 32767


#define _POSIX_SIGQUEUE_MAX 32


#define _POSIX_SSIZE_MAX 32767


#define _POSIX_STREAM_MAX 8


#define _POSIX_SYMLINK_MAX 255



#define _POSIX_SYMLOOP_MAX 8


#define _POSIX_TIMER_MAX 32


#define _POSIX_TTY_NAME_MAX 9


#define _POSIX_TZNAME_MAX 6


#define _POSIX_QLIMIT 1



#define _POSIX_HIWAT _POSIX_PIPE_BUF


#define _POSIX_UIO_MAXIOV 16


#define _POSIX_CLOCKRES_MIN 20000000



# 1 "/usr/include/bits/local_lim.h" 1 3
# 26 "/usr/include/bits/local_lim.h" 3
#define __undef_NR_OPEN 


#define __undef_LINK_MAX 


#define __undef_OPEN_MAX 



# 1 "/usr/include/linux/limits.h" 1 3

#define _LINUX_LIMITS_H 

#define NR_OPEN 1024

#define NGROUPS_MAX 32
#define ARG_MAX 131072
#define CHILD_MAX 999
#define OPEN_MAX 256
#define LINK_MAX 127
#define MAX_CANON 255
#define MAX_INPUT 255
#define NAME_MAX 255
#define PATH_MAX 4096
#define PIPE_BUF 4096

#define RTSIG_MAX 32
# 37 "/usr/include/bits/local_lim.h" 2 3



#undef NR_OPEN
#undef __undef_NR_OPEN



#undef LINK_MAX
#undef __undef_LINK_MAX



#undef OPEN_MAX
#undef __undef_OPEN_MAX



#define _POSIX_THREAD_KEYS_MAX 128

#define PTHREAD_KEYS_MAX 1024


#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS 4

#define PTHREAD_DESTRUCTOR_ITERATIONS _POSIX_THREAD_DESTRUCTOR_ITERATIONS


#define _POSIX_THREAD_THREADS_MAX 64

#define PTHREAD_THREADS_MAX 16384



#define AIO_PRIO_DELTA_MAX 20


#define PTHREAD_STACK_MIN 16384


#define TIMER_MAX 256


#define DELAYTIMER_MAX 2147483647


#define TTY_NAME_MAX 32


#define LOGIN_NAME_MAX 256
# 154 "/usr/include/bits/posix1_lim.h" 2 3



#define SSIZE_MAX LONG_MAX
# 145 "/usr/include/limits.h" 2 3



# 1 "/usr/include/bits/posix2_lim.h" 1 3
# 24 "/usr/include/bits/posix2_lim.h" 3
#define _BITS_POSIX2_LIM_H 1



#define _POSIX2_BC_BASE_MAX 99


#define _POSIX2_BC_DIM_MAX 2048


#define _POSIX2_BC_SCALE_MAX 99


#define _POSIX2_BC_STRING_MAX 1000



#define _POSIX2_COLL_WEIGHTS_MAX 2



#define _POSIX2_EXPR_NEST_MAX 32


#define _POSIX2_LINE_MAX 2048



#define _POSIX2_RE_DUP_MAX 255



#define _POSIX2_CHARCLASS_NAME_MAX 14







#define BC_BASE_MAX _POSIX2_BC_BASE_MAX


#define BC_DIM_MAX _POSIX2_BC_DIM_MAX


#define BC_SCALE_MAX _POSIX2_BC_SCALE_MAX


#define BC_STRING_MAX _POSIX2_BC_STRING_MAX


#define COLL_WEIGHTS_MAX 255


#define EXPR_NEST_MAX _POSIX2_EXPR_NEST_MAX


#define LINE_MAX _POSIX2_LINE_MAX


#define CHARCLASS_NAME_MAX 2048



#define RE_DUP_MAX (0x7fff)
# 149 "/usr/include/limits.h" 2 3
# 133 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 2 3
# 8 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/syslimits.h" 2 3
#undef _GCC_NEXT_LIMITS_H
# 12 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 2 3


#define _LIMITS_H___ 



#define CHAR_BIT 8
# 27 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 3
#define SCHAR_MIN (-128)

#define SCHAR_MAX 127



#define UCHAR_MAX 255
# 43 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 3
#define CHAR_MIN (-128)

#define CHAR_MAX 127



#define __SHRT_MAX__ 32767




#define SHRT_MIN (-SHRT_MAX-1)

#define SHRT_MAX __SHRT_MAX__



#define __INT_MAX__ 2147483647


#define INT_MIN (-INT_MAX-1)

#define INT_MAX __INT_MAX__






#define USHRT_MAX (SHRT_MAX * 2 + 1)




#define UINT_MAX (INT_MAX * 2U + 1)







#define __LONG_MAX__ 2147483647L



#define LONG_MIN (-LONG_MAX-1)

#define LONG_MAX __LONG_MAX__



#define ULONG_MAX (LONG_MAX * 2UL + 1)


#define __LONG_LONG_MAX__ 9223372036854775807LL
# 32 "/usr/include/bits/socket.h" 2 3




typedef __socklen_t socklen_t;
#define __socklen_t_defined 



enum __socket_type
{
  SOCK_STREAM = 1,

#define SOCK_STREAM SOCK_STREAM
  SOCK_DGRAM = 2,

#define SOCK_DGRAM SOCK_DGRAM
  SOCK_RAW = 3,
#define SOCK_RAW SOCK_RAW
  SOCK_RDM = 4,
#define SOCK_RDM SOCK_RDM
  SOCK_SEQPACKET = 5,

#define SOCK_SEQPACKET SOCK_SEQPACKET
  SOCK_PACKET = 10


#define SOCK_PACKET SOCK_PACKET
};


#define PF_UNSPEC 0
#define PF_LOCAL 1
#define PF_UNIX PF_LOCAL
#define PF_FILE PF_LOCAL
#define PF_INET 2
#define PF_AX25 3
#define PF_IPX 4
#define PF_APPLETALK 5
#define PF_NETROM 6
#define PF_BRIDGE 7
#define PF_ATMPVC 8
#define PF_X25 9
#define PF_INET6 10
#define PF_ROSE 11
#define PF_DECnet 12
#define PF_NETBEUI 13
#define PF_SECURITY 14
#define PF_KEY 15
#define PF_NETLINK 16
#define PF_ROUTE PF_NETLINK
#define PF_PACKET 17
#define PF_ASH 18
#define PF_ECONET 19
#define PF_ATMSVC 20
#define PF_SNA 22
#define PF_IRDA 23
#define PF_PPPOX 24
#define PF_WANPIPE 25
#define PF_BLUETOOTH 31
#define PF_MAX 32


#define AF_UNSPEC PF_UNSPEC
#define AF_LOCAL PF_LOCAL
#define AF_UNIX PF_UNIX
#define AF_FILE PF_FILE
#define AF_INET PF_INET
#define AF_AX25 PF_AX25
#define AF_IPX PF_IPX
#define AF_APPLETALK PF_APPLETALK
#define AF_NETROM PF_NETROM
#define AF_BRIDGE PF_BRIDGE
#define AF_ATMPVC PF_ATMPVC
#define AF_X25 PF_X25
#define AF_INET6 PF_INET6
#define AF_ROSE PF_ROSE
#define AF_DECnet PF_DECnet
#define AF_NETBEUI PF_NETBEUI
#define AF_SECURITY PF_SECURITY
#define AF_KEY PF_KEY
#define AF_NETLINK PF_NETLINK
#define AF_ROUTE PF_ROUTE
#define AF_PACKET PF_PACKET
#define AF_ASH PF_ASH
#define AF_ECONET PF_ECONET
#define AF_ATMSVC PF_ATMSVC
#define AF_SNA PF_SNA
#define AF_IRDA PF_IRDA
#define AF_PPPOX PF_PPPOX
#define AF_WANPIPE PF_WANPIPE
#define AF_BLUETOOTH PF_BLUETOOTH
#define AF_MAX PF_MAX





#define SOL_RAW 255
#define SOL_DECNET 261
#define SOL_X25 262
#define SOL_PACKET 263
#define SOL_ATM 264
#define SOL_AAL 265
#define SOL_IRDA 266


#define SOMAXCONN 128


# 1 "/usr/include/bits/sockaddr.h" 1 3
# 25 "/usr/include/bits/sockaddr.h" 3
#define _BITS_SOCKADDR_H 1



typedef unsigned short int sa_family_t;





#define __SOCKADDR_COMMON(sa_prefix) sa_family_t sa_prefix ##family


#define __SOCKADDR_COMMON_SIZE (sizeof (unsigned short int))
# 143 "/usr/include/bits/socket.h" 2 3


struct sockaddr
  {
    sa_family_t sa_family;
    char sa_data[14];
  };







#define __ss_aligntype __uint32_t

#define _SS_SIZE 128
#define _SS_PADSIZE (_SS_SIZE - (2 * sizeof (__ss_aligntype)))

struct sockaddr_storage
  {
    sa_family_t ss_family;
    __uint32_t __ss_align;
    char __ss_padding[(128 - (2 * sizeof (__uint32_t)))];
  };



enum
  {
    MSG_OOB = 0x01,
#define MSG_OOB MSG_OOB
    MSG_PEEK = 0x02,
#define MSG_PEEK MSG_PEEK
    MSG_DONTROUTE = 0x04,
#define MSG_DONTROUTE MSG_DONTROUTE





    MSG_CTRUNC = 0x08,
#define MSG_CTRUNC MSG_CTRUNC
    MSG_PROXY = 0x10,
#define MSG_PROXY MSG_PROXY
    MSG_TRUNC = 0x20,
#define MSG_TRUNC MSG_TRUNC
    MSG_DONTWAIT = 0x40,
#define MSG_DONTWAIT MSG_DONTWAIT
    MSG_EOR = 0x80,
#define MSG_EOR MSG_EOR
    MSG_WAITALL = 0x100,
#define MSG_WAITALL MSG_WAITALL
    MSG_FIN = 0x200,
#define MSG_FIN MSG_FIN
    MSG_SYN = 0x400,
#define MSG_SYN MSG_SYN
    MSG_CONFIRM = 0x800,
#define MSG_CONFIRM MSG_CONFIRM
    MSG_RST = 0x1000,
#define MSG_RST MSG_RST
    MSG_ERRQUEUE = 0x2000,
#define MSG_ERRQUEUE MSG_ERRQUEUE
    MSG_NOSIGNAL = 0x4000,
#define MSG_NOSIGNAL MSG_NOSIGNAL
    MSG_MORE = 0x8000
#define MSG_MORE MSG_MORE
  };




struct msghdr
  {
    void *msg_name;
    socklen_t msg_namelen;

    struct iovec *msg_iov;
    size_t msg_iovlen;

    void *msg_control;
    size_t msg_controllen;

    int msg_flags;
  };


struct cmsghdr
  {
    size_t cmsg_len;

    int cmsg_level;
    int cmsg_type;

    __extension__ unsigned char __cmsg_data [];

  };



#define CMSG_DATA(cmsg) ((cmsg)->__cmsg_data)



#define CMSG_NXTHDR(mhdr,cmsg) __cmsg_nxthdr (mhdr, cmsg)
#define CMSG_FIRSTHDR(mhdr) ((size_t) (mhdr)->msg_controllen >= sizeof (struct cmsghdr) ? (struct cmsghdr *) (mhdr)->msg_control : (struct cmsghdr *) NULL)


#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1))

#define CMSG_SPACE(len) (CMSG_ALIGN (len) + CMSG_ALIGN (sizeof (struct cmsghdr)))

#define CMSG_LEN(len) (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))

extern struct cmsghdr *__cmsg_nxthdr (struct msghdr *__mhdr,
                                      struct cmsghdr *__cmsg) ;


#define _EXTERN_INLINE extern __inline

extern __inline struct cmsghdr *
__cmsg_nxthdr (struct msghdr *__mhdr, struct cmsghdr *__cmsg)
{
  if ((size_t) __cmsg->cmsg_len < sizeof (struct cmsghdr))

    return 0;

  __cmsg = (struct cmsghdr *) ((unsigned char *) __cmsg
                               + (((__cmsg->cmsg_len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1)));
  if ((unsigned char *) (__cmsg + 1) > ((unsigned char *) __mhdr->msg_control
                                        + __mhdr->msg_controllen)
      || ((unsigned char *) __cmsg + (((__cmsg->cmsg_len) + sizeof (size_t) - 1) & (size_t) ~(sizeof (size_t) - 1))
          > ((unsigned char *) __mhdr->msg_control + __mhdr->msg_controllen)))

    return 0;
  return __cmsg;
}




enum
  {
    SCM_RIGHTS = 0x01,
#define SCM_RIGHTS SCM_RIGHTS

    SCM_CREDENTIALS = 0x02,
#define SCM_CREDENTIALS SCM_CREDENTIALS

    __SCM_CONNECT = 0x03
  };



struct ucred
{
  pid_t pid;
  uid_t uid;
  gid_t gid;
};


# 1 "/usr/include/asm/socket.h" 1 3

#define _ASM_SOCKET_H 

# 1 "/usr/include/asm/sockios.h" 1 3

#define __ARCH_I386_SOCKIOS__ 


#define FIOSETOWN 0x8901
#define SIOCSPGRP 0x8902
#define FIOGETOWN 0x8903
#define SIOCGPGRP 0x8904
#define SIOCATMARK 0x8905
#define SIOCGSTAMP 0x8906
# 5 "/usr/include/asm/socket.h" 2 3


#define SOL_SOCKET 1

#define SO_DEBUG 1
#define SO_REUSEADDR 2
#define SO_TYPE 3
#define SO_ERROR 4
#define SO_DONTROUTE 5
#define SO_BROADCAST 6
#define SO_SNDBUF 7
#define SO_RCVBUF 8
#define SO_KEEPALIVE 9
#define SO_OOBINLINE 10
#define SO_NO_CHECK 11
#define SO_PRIORITY 12
#define SO_LINGER 13
#define SO_BSDCOMPAT 14

#define SO_PASSCRED 16
#define SO_PEERCRED 17
#define SO_RCVLOWAT 18
#define SO_SNDLOWAT 19
#define SO_RCVTIMEO 20
#define SO_SNDTIMEO 21


#define SO_SECURITY_AUTHENTICATION 22
#define SO_SECURITY_ENCRYPTION_TRANSPORT 23
#define SO_SECURITY_ENCRYPTION_NETWORK 24

#define SO_BINDTODEVICE 25


#define SO_ATTACH_FILTER 26
#define SO_DETACH_FILTER 27

#define SO_PEERNAME 28
#define SO_TIMESTAMP 29
#define SCM_TIMESTAMP SO_TIMESTAMP

#define SO_ACCEPTCONN 30
# 306 "/usr/include/bits/socket.h" 2 3



struct linger
  {
    int l_onoff;
    int l_linger;
  };
# 36 "/usr/include/sys/socket.h" 2 3




struct osockaddr
  {
    unsigned short int sa_family;
    unsigned char sa_data[14];
  };




enum
{
  SHUT_RD = 0,
#define SHUT_RD SHUT_RD
  SHUT_WR,
#define SHUT_WR SHUT_WR
  SHUT_RDWR
#define SHUT_RDWR SHUT_RDWR
};
# 66 "/usr/include/sys/socket.h" 3
#define __SOCKADDR_ARG struct sockaddr *__restrict
#define __CONST_SOCKADDR_ARG __const struct sockaddr *
# 100 "/usr/include/sys/socket.h" 3
extern int socket (int __domain, int __type, int __protocol) ;





extern int socketpair (int __domain, int __type, int __protocol,
                       int __fds[2]) ;


extern int bind (int __fd, __const struct sockaddr * __addr, socklen_t __len)
     ;


extern int getsockname (int __fd, struct sockaddr *__restrict __addr,
                        socklen_t *__restrict __len) ;
# 124 "/usr/include/sys/socket.h" 3
extern int connect (int __fd, __const struct sockaddr * __addr, socklen_t __len);



extern int getpeername (int __fd, struct sockaddr *__restrict __addr,
                        socklen_t *__restrict __len) ;






extern ssize_t send (int __fd, __const void *__buf, size_t __n, int __flags);






extern ssize_t recv (int __fd, void *__buf, size_t __n, int __flags);






extern ssize_t sendto (int __fd, __const void *__buf, size_t __n,
                       int __flags, __const struct sockaddr * __addr,
                       socklen_t __addr_len);
# 161 "/usr/include/sys/socket.h" 3
extern ssize_t recvfrom (int __fd, void *__restrict __buf, size_t __n,
                         int __flags, struct sockaddr *__restrict __addr,
                         socklen_t *__restrict __addr_len);







extern ssize_t sendmsg (int __fd, __const struct msghdr *__message,
                        int __flags);






extern ssize_t recvmsg (int __fd, struct msghdr *__message, int __flags);





extern int getsockopt (int __fd, int __level, int __optname,
                       void *__restrict __optval,
                       socklen_t *__restrict __optlen) ;




extern int setsockopt (int __fd, int __level, int __optname,
                       __const void *__optval, socklen_t __optlen) ;





extern int listen (int __fd, int __n) ;
# 209 "/usr/include/sys/socket.h" 3
extern int accept (int __fd, struct sockaddr *__restrict __addr,
                   socklen_t *__restrict __addr_len);







extern int shutdown (int __fd, int __how) ;
# 231 "/usr/include/sys/socket.h" 3
extern int isfdtype (int __fd, int __fdtype) ;



# 3 "shmmb_print.c" 2
# 1 "/usr/include/netinet/in.h" 1 3
# 20 "/usr/include/netinet/in.h" 3
#define _NETINET_IN_H 1


# 1 "/usr/include/stdint.h" 1 3
# 24 "/usr/include/stdint.h" 3
#define _STDINT_H 1



# 1 "/usr/include/bits/wordsize.h" 1 3
# 19 "/usr/include/bits/wordsize.h" 3
#define __WORDSIZE 32
# 29 "/usr/include/stdint.h" 2 3
# 49 "/usr/include/stdint.h" 3
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;

typedef unsigned int uint32_t;
#define __uint32_t_defined 




__extension__
typedef unsigned long long int uint64_t;






typedef signed char int_least8_t;
typedef short int int_least16_t;
typedef int int_least32_t;



__extension__
typedef long long int int_least64_t;



typedef unsigned char uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int uint_least32_t;



__extension__
typedef unsigned long long int uint_least64_t;






typedef signed char int_fast8_t;





typedef int int_fast16_t;
typedef int int_fast32_t;
__extension__
typedef long long int int_fast64_t;



typedef unsigned char uint_fast8_t;





typedef unsigned int uint_fast16_t;
typedef unsigned int uint_fast32_t;
__extension__
typedef unsigned long long int uint_fast64_t;
# 126 "/usr/include/stdint.h" 3
typedef int intptr_t;
#define __intptr_t_defined 

typedef unsigned int uintptr_t;
# 138 "/usr/include/stdint.h" 3
__extension__
typedef long long int intmax_t;
__extension__
typedef unsigned long long int uintmax_t;
# 153 "/usr/include/stdint.h" 3
#define __INT64_C(c) c ## LL
#define __UINT64_C(c) c ## ULL





#define INT8_MIN (-128)
#define INT16_MIN (-32767-1)
#define INT32_MIN (-2147483647-1)
#define INT64_MIN (-__INT64_C(9223372036854775807)-1)

#define INT8_MAX (127)
#define INT16_MAX (32767)
#define INT32_MAX (2147483647)
#define INT64_MAX (__INT64_C(9223372036854775807))


#define UINT8_MAX (255)
#define UINT16_MAX (65535)
#define UINT32_MAX (4294967295U)
#define UINT64_MAX (__UINT64_C(18446744073709551615))



#define INT_LEAST8_MIN (-128)
#define INT_LEAST16_MIN (-32767-1)
#define INT_LEAST32_MIN (-2147483647-1)
#define INT_LEAST64_MIN (-__INT64_C(9223372036854775807)-1)

#define INT_LEAST8_MAX (127)
#define INT_LEAST16_MAX (32767)
#define INT_LEAST32_MAX (2147483647)
#define INT_LEAST64_MAX (__INT64_C(9223372036854775807))


#define UINT_LEAST8_MAX (255)
#define UINT_LEAST16_MAX (65535)
#define UINT_LEAST32_MAX (4294967295U)
#define UINT_LEAST64_MAX (__UINT64_C(18446744073709551615))



#define INT_FAST8_MIN (-128)




#define INT_FAST16_MIN (-2147483647-1)
#define INT_FAST32_MIN (-2147483647-1)

#define INT_FAST64_MIN (-__INT64_C(9223372036854775807)-1)

#define INT_FAST8_MAX (127)




#define INT_FAST16_MAX (2147483647)
#define INT_FAST32_MAX (2147483647)

#define INT_FAST64_MAX (__INT64_C(9223372036854775807))


#define UINT_FAST8_MAX (255)




#define UINT_FAST16_MAX (4294967295U)
#define UINT_FAST32_MAX (4294967295U)

#define UINT_FAST64_MAX (__UINT64_C(18446744073709551615))
# 234 "/usr/include/stdint.h" 3
#define INTPTR_MIN (-2147483647-1)
#define INTPTR_MAX (2147483647)
#define UINTPTR_MAX (4294967295U)




#define INTMAX_MIN (-__INT64_C(9223372036854775807)-1)

#define INTMAX_MAX (__INT64_C(9223372036854775807))


#define UINTMAX_MAX (__UINT64_C(18446744073709551615))
# 256 "/usr/include/stdint.h" 3
#define PTRDIFF_MIN (-2147483647-1)
#define PTRDIFF_MAX (2147483647)



#define SIG_ATOMIC_MIN (-2147483647-1)
#define SIG_ATOMIC_MAX (2147483647)





#define SIZE_MAX (4294967295U)





#define WCHAR_MIN __WCHAR_MIN
#define WCHAR_MAX __WCHAR_MAX



#define WINT_MIN (0u)
#define WINT_MAX (4294967295u)
# 290 "/usr/include/stdint.h" 3
#define INT8_C(c) c
#define INT16_C(c) c
#define INT32_C(c) c



#define INT64_C(c) c ## LL



#define UINT8_C(c) c ## U
#define UINT16_C(c) c ## U
#define UINT32_C(c) c ## U



#define UINT64_C(c) c ## ULL







#define INTMAX_C(c) c ## LL
#define UINTMAX_C(c) c ## ULL
# 24 "/usr/include/netinet/in.h" 2 3







enum
  {
    IPPROTO_IP = 0,
#define IPPROTO_IP IPPROTO_IP
    IPPROTO_HOPOPTS = 0,
#define IPPROTO_HOPOPTS IPPROTO_HOPOPTS
    IPPROTO_ICMP = 1,
#define IPPROTO_ICMP IPPROTO_ICMP
    IPPROTO_IGMP = 2,
#define IPPROTO_IGMP IPPROTO_IGMP
    IPPROTO_IPIP = 4,
#define IPPROTO_IPIP IPPROTO_IPIP
    IPPROTO_TCP = 6,
#define IPPROTO_TCP IPPROTO_TCP
    IPPROTO_EGP = 8,
#define IPPROTO_EGP IPPROTO_EGP
    IPPROTO_PUP = 12,
#define IPPROTO_PUP IPPROTO_PUP
    IPPROTO_UDP = 17,
#define IPPROTO_UDP IPPROTO_UDP
    IPPROTO_IDP = 22,
#define IPPROTO_IDP IPPROTO_IDP
    IPPROTO_TP = 29,
#define IPPROTO_TP IPPROTO_TP
    IPPROTO_IPV6 = 41,
#define IPPROTO_IPV6 IPPROTO_IPV6
    IPPROTO_ROUTING = 43,
#define IPPROTO_ROUTING IPPROTO_ROUTING
    IPPROTO_FRAGMENT = 44,
#define IPPROTO_FRAGMENT IPPROTO_FRAGMENT
    IPPROTO_RSVP = 46,
#define IPPROTO_RSVP IPPROTO_RSVP
    IPPROTO_GRE = 47,
#define IPPROTO_GRE IPPROTO_GRE
    IPPROTO_ESP = 50,
#define IPPROTO_ESP IPPROTO_ESP
    IPPROTO_AH = 51,
#define IPPROTO_AH IPPROTO_AH
    IPPROTO_ICMPV6 = 58,
#define IPPROTO_ICMPV6 IPPROTO_ICMPV6
    IPPROTO_NONE = 59,
#define IPPROTO_NONE IPPROTO_NONE
    IPPROTO_DSTOPTS = 60,
#define IPPROTO_DSTOPTS IPPROTO_DSTOPTS
    IPPROTO_MTP = 92,
#define IPPROTO_MTP IPPROTO_MTP
    IPPROTO_ENCAP = 98,
#define IPPROTO_ENCAP IPPROTO_ENCAP
    IPPROTO_PIM = 103,
#define IPPROTO_PIM IPPROTO_PIM
    IPPROTO_COMP = 108,
#define IPPROTO_COMP IPPROTO_COMP
    IPPROTO_SCTP = 132,
#define IPPROTO_SCTP IPPROTO_SCTP
    IPPROTO_RAW = 255,
#define IPPROTO_RAW IPPROTO_RAW
    IPPROTO_MAX
  };



typedef uint16_t in_port_t;


enum
  {
    IPPORT_ECHO = 7,
    IPPORT_DISCARD = 9,
    IPPORT_SYSTAT = 11,
    IPPORT_DAYTIME = 13,
    IPPORT_NETSTAT = 15,
    IPPORT_FTP = 21,
    IPPORT_TELNET = 23,
    IPPORT_SMTP = 25,
    IPPORT_TIMESERVER = 37,
    IPPORT_NAMESERVER = 42,
    IPPORT_WHOIS = 43,
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,


    IPPORT_EXECSERVER = 512,
    IPPORT_LOGINSERVER = 513,
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,


    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,


    IPPORT_RESERVED = 1024,


    IPPORT_USERRESERVED = 5000
  };



typedef uint32_t in_addr_t;
struct in_addr
  {
    in_addr_t s_addr;
  };







#define IN_CLASSA(a) ((((in_addr_t)(a)) & 0x80000000) == 0)
#define IN_CLASSA_NET 0xff000000
#define IN_CLASSA_NSHIFT 24
#define IN_CLASSA_HOST (0xffffffff & ~IN_CLASSA_NET)
#define IN_CLASSA_MAX 128

#define IN_CLASSB(a) ((((in_addr_t)(a)) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET 0xffff0000
#define IN_CLASSB_NSHIFT 16
#define IN_CLASSB_HOST (0xffffffff & ~IN_CLASSB_NET)
#define IN_CLASSB_MAX 65536

#define IN_CLASSC(a) ((((in_addr_t)(a)) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET 0xffffff00
#define IN_CLASSC_NSHIFT 8
#define IN_CLASSC_HOST (0xffffffff & ~IN_CLASSC_NET)

#define IN_CLASSD(a) ((((in_addr_t)(a)) & 0xf0000000) == 0xe0000000)
#define IN_MULTICAST(a) IN_CLASSD(a)

#define IN_EXPERIMENTAL(a) ((((in_addr_t)(a)) & 0xe0000000) == 0xe0000000)
#define IN_BADCLASS(a) ((((in_addr_t)(a)) & 0xf0000000) == 0xf0000000)


#define INADDR_ANY ((in_addr_t) 0x00000000)

#define INADDR_BROADCAST ((in_addr_t) 0xffffffff)

#define INADDR_NONE ((in_addr_t) 0xffffffff)


#define IN_LOOPBACKNET 127


#define INADDR_LOOPBACK ((in_addr_t) 0x7f000001)



#define INADDR_UNSPEC_GROUP ((in_addr_t) 0xe0000000)
#define INADDR_ALLHOSTS_GROUP ((in_addr_t) 0xe0000001)
#define INADDR_ALLRTRS_GROUP ((in_addr_t) 0xe0000002)
#define INADDR_MAX_LOCAL_GROUP ((in_addr_t) 0xe00000ff)



struct in6_addr
  {
    union
      {
        uint8_t u6_addr8[16];
        uint16_t u6_addr16[8];
        uint32_t u6_addr32[4];
      } in6_u;
#define s6_addr in6_u.u6_addr8
#define s6_addr16 in6_u.u6_addr16
#define s6_addr32 in6_u.u6_addr32
  };

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;
#define IN6ADDR_ANY_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } }
#define IN6ADDR_LOOPBACK_INIT { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } }

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46






struct sockaddr_in
  {
    sa_family_t sin_family;
    in_port_t sin_port;
    struct in_addr sin_addr;


    unsigned char sin_zero[sizeof (struct sockaddr) -
                           (sizeof (unsigned short int)) -
                           sizeof (in_port_t) -
                           sizeof (struct in_addr)];
  };


struct sockaddr_in6
  {
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
  };


struct ipv6_mreq
  {

    struct in6_addr ipv6mr_multiaddr;


    unsigned int ipv6mr_interface;
  };


# 1 "/usr/include/bits/in.h" 1 3
# 28 "/usr/include/bits/in.h" 3
#define IP_TOS 1
#define IP_TTL 2
#define IP_HDRINCL 3
#define IP_OPTIONS 4
#define IP_ROUTER_ALERT 5
#define IP_RECVOPTS 6
#define IP_RETOPTS 7
#define IP_PKTINFO 8
#define IP_PKTOPTIONS 9
#define IP_PMTUDISC 10
#define IP_MTU_DISCOVER 10
#define IP_RECVERR 11
#define IP_RECVTTL 12
#define IP_RECVTOS 13
#define IP_MULTICAST_IF 32
#define IP_MULTICAST_TTL 33
#define IP_MULTICAST_LOOP 34
#define IP_ADD_MEMBERSHIP 35
#define IP_DROP_MEMBERSHIP 36


#define IP_RECVRETOPTS IP_RETOPTS


#define IP_PMTUDISC_DONT 0
#define IP_PMTUDISC_WANT 1
#define IP_PMTUDISC_DO 2


#define SOL_IP 0

#define IP_DEFAULT_MULTICAST_TTL 1
#define IP_DEFAULT_MULTICAST_LOOP 1
#define IP_MAX_MEMBERSHIPS 20




struct ip_opts
  {
    struct in_addr ip_dst;
    char ip_opts[40];
  };


struct ip_mreq
  {
    struct in_addr imr_multiaddr;
    struct in_addr imr_interface;
  };


struct ip_mreqn
  {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
  };


struct in_pktinfo
  {
    int ipi_ifindex;
    struct in_addr ipi_spec_dst;
    struct in_addr ipi_addr;
  };




#define IPV6_ADDRFORM 1
#define IPV6_PKTINFO 2
#define IPV6_HOPOPTS 3
#define IPV6_DSTOPTS 4
#define IPV6_RTHDR 5
#define IPV6_PKTOPTIONS 6
#define IPV6_CHECKSUM 7
#define IPV6_HOPLIMIT 8
#define IPV6_NEXTHOP 9
#define IPV6_AUTHHDR 10
#define IPV6_UNICAST_HOPS 16
#define IPV6_MULTICAST_IF 17
#define IPV6_MULTICAST_HOPS 18
#define IPV6_MULTICAST_LOOP 19
#define IPV6_JOIN_GROUP 20
#define IPV6_LEAVE_GROUP 21
#define IPV6_ROUTER_ALERT 22
#define IPV6_MTU_DISCOVER 23
#define IPV6_MTU 24
#define IPV6_RECVERR 25
#define IPV6_V6ONLY 26
#define IPV6_JOIN_ANYCAST 27
#define IPV6_LEAVE_ANYCAST 28
#define IPV6_IPSEC_POLICY 34
#define IPV6_XFRM_POLICY 35

#define SCM_SRCRT IPV6_RXSRCRT


#define IPV6_RXHOPOPTS IPV6_HOPOPTS
#define IPV6_RXDSTOPTS IPV6_DSTOPTS
#define IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP



#define IPV6_PMTUDISC_DONT 0
#define IPV6_PMTUDISC_WANT 1
#define IPV6_PMTUDISC_DO 2


#define SOL_IPV6 41
#define SOL_ICMPV6 58


#define IPV6_RTHDR_LOOSE 0
#define IPV6_RTHDR_STRICT 1

#define IPV6_RTHDR_TYPE_0 0
# 254 "/usr/include/netinet/in.h" 2 3
# 262 "/usr/include/netinet/in.h" 3
extern uint32_t ntohl (uint32_t __netlong) __attribute__ ((__const__));
extern uint16_t ntohs (uint16_t __netshort)
     __attribute__ ((__const__));
extern uint32_t htonl (uint32_t __hostlong)
     __attribute__ ((__const__));
extern uint16_t htons (uint16_t __hostshort)
     __attribute__ ((__const__));




# 1 "/usr/include/bits/byteswap.h" 1 3
# 25 "/usr/include/bits/byteswap.h" 3
#define _BITS_BYTESWAP_H 1


#define __bswap_constant_16(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))




#define __bswap_16(x) (__extension__ ({ register unsigned short int __v, __x = (x); if (__builtin_constant_p (__x)) __v = __bswap_constant_16 (__x); else __asm__ ("rorw $8, %w0" : "=r" (__v) : "0" (__x) : "cc"); __v; }))
# 59 "/usr/include/bits/byteswap.h" 3
#define __bswap_constant_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
# 69 "/usr/include/bits/byteswap.h" 3
#define __bswap_32(x) (__extension__ ({ register unsigned int __v, __x = (x); if (__builtin_constant_p (__x)) __v = __bswap_constant_32 (__x); else __asm__ ("rorw $8, %w0;" "rorl $16, %0;" "rorw $8, %w0" : "=r" (__v) : "0" (__x) : "cc"); __v; }))
# 108 "/usr/include/bits/byteswap.h" 3
#define __bswap_constant_64(x) ((((x) & 0xff00000000000000ull) >> 56) | (((x) & 0x00ff000000000000ull) >> 40) | (((x) & 0x0000ff0000000000ull) >> 24) | (((x) & 0x000000ff00000000ull) >> 8) | (((x) & 0x00000000ff000000ull) << 8) | (((x) & 0x0000000000ff0000ull) << 24) | (((x) & 0x000000000000ff00ull) << 40) | (((x) & 0x00000000000000ffull) << 56))
# 118 "/usr/include/bits/byteswap.h" 3
#define __bswap_64(x) (__extension__ ({ union { __extension__ unsigned long long int __ll; unsigned long int __l[2]; } __w, __r; if (__builtin_constant_p (x)) __r.__ll = __bswap_constant_64 (x); else { __w.__ll = (x); __r.__l[0] = __bswap_32 (__w.__l[1]); __r.__l[1] = __bswap_32 (__w.__l[0]); } __r.__ll; }))
# 274 "/usr/include/netinet/in.h" 2 3
# 288 "/usr/include/netinet/in.h" 3
#define ntohl(x) __bswap_32 (x)
#define ntohs(x) __bswap_16 (x)
#define htonl(x) __bswap_32 (x)
#define htons(x) __bswap_16 (x)




#define IN6_IS_ADDR_UNSPECIFIED(a) (((__const uint32_t *) (a))[0] == 0 && ((__const uint32_t *) (a))[1] == 0 && ((__const uint32_t *) (a))[2] == 0 && ((__const uint32_t *) (a))[3] == 0)





#define IN6_IS_ADDR_LOOPBACK(a) (((__const uint32_t *) (a))[0] == 0 && ((__const uint32_t *) (a))[1] == 0 && ((__const uint32_t *) (a))[2] == 0 && ((__const uint32_t *) (a))[3] == htonl (1))





#define IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)

#define IN6_IS_ADDR_LINKLOCAL(a) ((((__const uint32_t *) (a))[0] & htonl (0xffc00000)) == htonl (0xfe800000))



#define IN6_IS_ADDR_SITELOCAL(a) ((((__const uint32_t *) (a))[0] & htonl (0xffc00000)) == htonl (0xfec00000))



#define IN6_IS_ADDR_V4MAPPED(a) ((((__const uint32_t *) (a))[0] == 0) && (((__const uint32_t *) (a))[1] == 0) && (((__const uint32_t *) (a))[2] == htonl (0xffff)))




#define IN6_IS_ADDR_V4COMPAT(a) ((((__const uint32_t *) (a))[0] == 0) && (((__const uint32_t *) (a))[1] == 0) && (((__const uint32_t *) (a))[2] == 0) && (ntohl (((__const uint32_t *) (a))[3]) > 1))





#define IN6_ARE_ADDR_EQUAL(a,b) ((((__const uint32_t *) (a))[0] == ((__const uint32_t *) (b))[0]) && (((__const uint32_t *) (a))[1] == ((__const uint32_t *) (b))[1]) && (((__const uint32_t *) (a))[2] == ((__const uint32_t *) (b))[2]) && (((__const uint32_t *) (a))[3] == ((__const uint32_t *) (b))[3]))






extern int bindresvport (int __sockfd, struct sockaddr_in *__sock_in) ;


extern int bindresvport6 (int __sockfd, struct sockaddr_in6 *__sock_in)
     ;


#define IN6_IS_ADDR_MC_NODELOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((__const uint8_t *) (a))[1] & 0xf) == 0x1))



#define IN6_IS_ADDR_MC_LINKLOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((__const uint8_t *) (a))[1] & 0xf) == 0x2))



#define IN6_IS_ADDR_MC_SITELOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((__const uint8_t *) (a))[1] & 0xf) == 0x5))



#define IN6_IS_ADDR_MC_ORGLOCAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((__const uint8_t *) (a))[1] & 0xf) == 0x8))



#define IN6_IS_ADDR_MC_GLOBAL(a) (IN6_IS_ADDR_MULTICAST(a) && ((((__const uint8_t *) (a))[1] & 0xf) == 0xe))




struct in6_pktinfo
  {
    struct in6_addr ipi6_addr;
    unsigned int ipi6_ifindex;
  };
# 387 "/usr/include/netinet/in.h" 3

# 4 "shmmb_print.c" 2
# 1 "/usr/include/arpa/inet.h" 1 3
# 20 "/usr/include/arpa/inet.h" 3
#define _ARPA_INET_H 1
# 31 "/usr/include/arpa/inet.h" 3




extern in_addr_t inet_addr (__const char *__cp) ;


extern in_addr_t inet_lnaof (struct in_addr __in) ;



extern struct in_addr inet_makeaddr (in_addr_t __net, in_addr_t __host)
     ;


extern in_addr_t inet_netof (struct in_addr __in) ;



extern in_addr_t inet_network (__const char *__cp) ;



extern char *inet_ntoa (struct in_addr __in) ;




extern int inet_pton (int __af, __const char *__restrict __cp,
                      void *__restrict __buf) ;




extern __const char *inet_ntop (int __af, __const void *__restrict __cp,
                                char *__restrict __buf, socklen_t __len)
     ;






extern int inet_aton (__const char *__cp, struct in_addr *__inp) ;



extern char *inet_neta (in_addr_t __net, char *__buf, size_t __len) ;




extern char *inet_net_ntop (int __af, __const void *__cp, int __bits,
                            char *__buf, size_t __len) ;




extern int inet_net_pton (int __af, __const char *__cp,
                          void *__buf, size_t __len) ;




extern unsigned int inet_nsap_addr (__const char *__cp,
                                    unsigned char *__buf, int __len) ;



extern char *inet_nsap_ntoa (int __len, __const unsigned char *__cp,
                             char *__buf) ;



# 5 "shmmb_print.c" 2

# 1 "/usr/include/sys/ipc.h" 1 3
# 20 "/usr/include/sys/ipc.h" 3
#define _SYS_IPC_H 1
# 29 "/usr/include/sys/ipc.h" 3
# 1 "/usr/include/bits/ipctypes.h" 1 3
# 25 "/usr/include/bits/ipctypes.h" 3
#define _BITS_IPCTYPES_H 1





typedef unsigned short int __ipc_pid_t;
# 30 "/usr/include/sys/ipc.h" 2 3
# 1 "/usr/include/bits/ipc.h" 1 3
# 26 "/usr/include/bits/ipc.h" 3
#define IPC_CREAT 01000
#define IPC_EXCL 02000
#define IPC_NOWAIT 04000


#define IPC_RMID 0
#define IPC_SET 1
#define IPC_STAT 2





#define IPC_PRIVATE ((__key_t) 0)



struct ipc_perm
  {
    __key_t __key;
    __uid_t uid;
    __gid_t gid;
    __uid_t cuid;
    __gid_t cgid;
    unsigned short int mode;
    unsigned short int __pad1;
    unsigned short int __seq;
    unsigned short int __pad2;
    unsigned long int __unused1;
    unsigned long int __unused2;
  };
# 31 "/usr/include/sys/ipc.h" 2 3
# 52 "/usr/include/sys/ipc.h" 3



extern key_t ftok (__const char *__pathname, int __proj_id) ;


# 7 "shmmb_print.c" 2
# 1 "/usr/include/sys/shm.h" 1 3
# 20 "/usr/include/sys/shm.h" 3
#define _SYS_SHM_H 1



#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 26 "/usr/include/sys/shm.h" 2 3





# 1 "/usr/include/bits/shm.h" 1 3
# 26 "/usr/include/bits/shm.h" 3
#define SHM_R 0400
#define SHM_W 0200


#define SHM_RDONLY 010000
#define SHM_RND 020000
#define SHM_REMAP 040000


#define SHM_LOCK 11
#define SHM_UNLOCK 12




#define SHMLBA (__getpagesize ())
extern int __getpagesize (void) __attribute__ ((__const__));



typedef unsigned long int shmatt_t;


struct shmid_ds
  {
    struct ipc_perm shm_perm;
    size_t shm_segsz;
    __time_t shm_atime;
    unsigned long int __unused1;
    __time_t shm_dtime;
    unsigned long int __unused2;
    __time_t shm_ctime;
    unsigned long int __unused3;
    __pid_t shm_cpid;
    __pid_t shm_lpid;
    shmatt_t shm_nattch;
    unsigned long int __unused4;
    unsigned long int __unused5;
  };




#define SHM_STAT 13
#define SHM_INFO 14


#define SHM_DEST 01000
#define SHM_LOCKED 02000
#define SHM_HUGETLB 04000

struct shminfo
  {
    unsigned long int shmmax;
    unsigned long int shmmin;
    unsigned long int shmmni;
    unsigned long int shmseg;
    unsigned long int shmall;
    unsigned long int __unused1;
    unsigned long int __unused2;
    unsigned long int __unused3;
    unsigned long int __unused4;
  };

struct shm_info
  {
    int used_ids;
    unsigned long int shm_tot;
    unsigned long int shm_rss;
    unsigned long int shm_swp;
    unsigned long int swap_attempts;
    unsigned long int swap_successes;
  };




# 32 "/usr/include/sys/shm.h" 2 3


#define __need_time_t 
# 1 "/usr/include/time.h" 1 3
# 83 "/usr/include/time.h" 3
#undef __need_time_t
# 36 "/usr/include/sys/shm.h" 2 3
# 45 "/usr/include/sys/shm.h" 3






extern int shmctl (int __shmid, int __cmd, struct shmid_ds *__buf) ;


extern int shmget (key_t __key, size_t __size, int __shmflg) ;


extern void *shmat (int __shmid, __const void *__shmaddr, int __shmflg)
     ;


extern int shmdt (__const void *__shmaddr) ;


# 8 "shmmb_print.c" 2
# 1 "../../include/ipaf_svc.h" 1
# 18 "../../include/ipaf_svc.h"
#define _IPAF_SVC_________________H 

# 1 "/usr/include/time.h" 1 3
# 27 "/usr/include/time.h" 3
#define _TIME_H 1








#define __need_size_t 
#define __need_NULL 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 397 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 39 "/usr/include/time.h" 2 3



# 1 "/usr/include/bits/time.h" 1 3
# 26 "/usr/include/bits/time.h" 3
#define _BITS_TIME_H 1







#define CLOCKS_PER_SEC 1000000l





extern long int __sysconf (int);
#define CLK_TCK ((__clock_t) __sysconf (2))




#define CLOCK_REALTIME 0

#define CLOCK_MONOTONIC 1

#define CLOCK_PROCESS_CPUTIME_ID 2

#define CLOCK_THREAD_CPUTIME_ID 3


#define TIMER_ABSTIME 1
# 43 "/usr/include/time.h" 2 3
# 54 "/usr/include/time.h" 3
#define __clock_t_defined 1





typedef __clock_t clock_t;



# 129 "/usr/include/time.h" 3


struct tm
{
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;


  long int tm_gmtoff;
  __const char *tm_zone;




};








struct itimerspec
  {
    struct timespec it_interval;
    struct timespec it_value;
  };


struct sigevent;
# 178 "/usr/include/time.h" 3



extern clock_t clock (void) ;


extern time_t time (time_t *__timer) ;


extern double difftime (time_t __time1, time_t __time0)
     __attribute__ ((__const__));


extern time_t mktime (struct tm *__tp) ;





extern size_t strftime (char *__restrict __s, size_t __maxsize,
                        __const char *__restrict __format,
                        __const struct tm *__restrict __tp) ;

# 226 "/usr/include/time.h" 3



extern struct tm *gmtime (__const time_t *__timer) ;



extern struct tm *localtime (__const time_t *__timer) ;





extern struct tm *gmtime_r (__const time_t *__restrict __timer,
                            struct tm *__restrict __tp) ;



extern struct tm *localtime_r (__const time_t *__restrict __timer,
                               struct tm *__restrict __tp) ;





extern char *asctime (__const struct tm *__tp) ;


extern char *ctime (__const time_t *__timer) ;







extern char *asctime_r (__const struct tm *__restrict __tp,
                        char *__restrict __buf) ;


extern char *ctime_r (__const time_t *__restrict __timer,
                      char *__restrict __buf) ;




extern char *__tzname[2];
extern int __daylight;
extern long int __timezone;




extern char *tzname[2];



extern void tzset (void) ;



extern int daylight;
extern long int timezone;





extern int stime (__const time_t *__when) ;





#define __isleap(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
# 309 "/usr/include/time.h" 3
extern time_t timegm (struct tm *__tp) ;


extern time_t timelocal (struct tm *__tp) ;


extern int dysize (int __year) __attribute__ ((__const__));
# 324 "/usr/include/time.h" 3
extern int nanosleep (__const struct timespec *__requested_time,
                      struct timespec *__remaining);



extern int clock_getres (clockid_t __clock_id, struct timespec *__res) ;


extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) ;


extern int clock_settime (clockid_t __clock_id, __const struct timespec *__tp)
     ;
# 353 "/usr/include/time.h" 3
extern int timer_create (clockid_t __clock_id,
                         struct sigevent *__restrict __evp,
                         timer_t *__restrict __timerid) ;


extern int timer_delete (timer_t __timerid) ;


extern int timer_settime (timer_t __timerid, int __flags,
                          __const struct itimerspec *__restrict __value,
                          struct itimerspec *__restrict __ovalue) ;


extern int timer_gettime (timer_t __timerid, struct itimerspec *__value)
     ;


extern int timer_getoverrun (timer_t __timerid) ;
# 413 "/usr/include/time.h" 3

# 21 "../../include/ipaf_svc.h" 2
# 1 "/usr/include/sys/time.h" 1 3
# 20 "/usr/include/sys/time.h" 3
#define _SYS_TIME_H 1




#define __need_time_t 

#define __need_timeval 
# 1 "/usr/include/bits/time.h" 1 3
# 62 "/usr/include/bits/time.h" 3
#undef __need_timeval
# 29 "/usr/include/sys/time.h" 2 3
# 38 "/usr/include/sys/time.h" 3

# 56 "/usr/include/sys/time.h" 3
struct timezone
  {
    int tz_minuteswest;
    int tz_dsttime;
  };

typedef struct timezone *__restrict __timezone_ptr_t;
# 72 "/usr/include/sys/time.h" 3
extern int gettimeofday (struct timeval *__restrict __tv,
                         __timezone_ptr_t __tz) ;




extern int settimeofday (__const struct timeval *__tv,
                         __const struct timezone *__tz) ;





extern int adjtime (__const struct timeval *__delta,
                    struct timeval *__olddelta) ;




enum __itimer_which
  {

    ITIMER_REAL = 0,
#define ITIMER_REAL ITIMER_REAL

    ITIMER_VIRTUAL = 1,
#define ITIMER_VIRTUAL ITIMER_VIRTUAL


    ITIMER_PROF = 2
#define ITIMER_PROF ITIMER_PROF
  };



struct itimerval
  {

    struct timeval it_interval;

    struct timeval it_value;
  };






typedef int __itimer_which_t;




extern int getitimer (__itimer_which_t __which,
                      struct itimerval *__value) ;




extern int setitimer (__itimer_which_t __which,
                      __const struct itimerval *__restrict __new,
                      struct itimerval *__restrict __old) ;




extern int utimes (__const char *__file, __const struct timeval __tvp[2])
     ;



extern int lutimes (__const char *__file, __const struct timeval __tvp[2])
     ;


extern int futimes (int __fd, __const struct timeval __tvp[2]) ;






#define timerisset(tvp) ((tvp)->tv_sec || (tvp)->tv_usec)
#define timerclear(tvp) ((tvp)->tv_sec = (tvp)->tv_usec = 0)
#define timercmp(a,b,CMP) (((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))



#define timeradd(a,b,result) do { (result)->tv_sec = (a)->tv_sec + (b)->tv_sec; (result)->tv_usec = (a)->tv_usec + (b)->tv_usec; if ((result)->tv_usec >= 1000000) { ++(result)->tv_sec; (result)->tv_usec -= 1000000; } } while (0)
# 170 "/usr/include/sys/time.h" 3
#define timersub(a,b,result) do { (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; if ((result)->tv_usec < 0) { --(result)->tv_sec; (result)->tv_usec += 1000000; } } while (0)
# 181 "/usr/include/sys/time.h" 3

# 22 "../../include/ipaf_svc.h" 2
# 1 "../../include/define.h" 1
# 16 "../../include/define.h"
#define _DEFINE____________H 

# 1 "/usr/include/string.h" 1 3
# 24 "/usr/include/string.h" 3
#define _STRING_H 1






#define __need_size_t 
#define __need_NULL 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 397 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 34 "/usr/include/string.h" 2 3




extern void *memcpy (void *__restrict __dest,
                     __const void *__restrict __src, size_t __n) ;


extern void *memmove (void *__dest, __const void *__src, size_t __n)
     ;






extern void *memccpy (void *__restrict __dest, __const void *__restrict __src,
                      int __c, size_t __n)
     ;





extern void *memset (void *__s, int __c, size_t __n) ;


extern int memcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__pure__));


extern void *memchr (__const void *__s, int __c, size_t __n)
      __attribute__ ((__pure__));

# 80 "/usr/include/string.h" 3


extern char *strcpy (char *__restrict __dest, __const char *__restrict __src)
     ;

extern char *strncpy (char *__restrict __dest,
                      __const char *__restrict __src, size_t __n) ;


extern char *strcat (char *__restrict __dest, __const char *__restrict __src)
     ;

extern char *strncat (char *__restrict __dest, __const char *__restrict __src,
                      size_t __n) ;


extern int strcmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__pure__));

extern int strncmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__pure__));


extern int strcoll (__const char *__s1, __const char *__s2)
     __attribute__ ((__pure__));

extern size_t strxfrm (char *__restrict __dest,
                       __const char *__restrict __src, size_t __n) ;

# 126 "/usr/include/string.h" 3
extern char *strdup (__const char *__s) __attribute__ ((__malloc__));
# 160 "/usr/include/string.h" 3


extern char *strchr (__const char *__s, int __c) __attribute__ ((__pure__));

extern char *strrchr (__const char *__s, int __c) __attribute__ ((__pure__));











extern size_t strcspn (__const char *__s, __const char *__reject)
     __attribute__ ((__pure__));


extern size_t strspn (__const char *__s, __const char *__accept)
     __attribute__ ((__pure__));

extern char *strpbrk (__const char *__s, __const char *__accept)
     __attribute__ ((__pure__));

extern char *strstr (__const char *__haystack, __const char *__needle)
     __attribute__ ((__pure__));



extern char *strtok (char *__restrict __s, __const char *__restrict __delim)
     ;




extern char *__strtok_r (char *__restrict __s,
                         __const char *__restrict __delim,
                         char **__restrict __save_ptr) ;

extern char *strtok_r (char *__restrict __s, __const char *__restrict __delim,
                       char **__restrict __save_ptr) ;
# 228 "/usr/include/string.h" 3


extern size_t strlen (__const char *__s) __attribute__ ((__pure__));

# 241 "/usr/include/string.h" 3


extern char *strerror (int __errnum) ;




extern char *strerror_r (int __errnum, char *__buf, size_t __buflen) ;




extern void __bzero (void *__s, size_t __n) ;



extern void bcopy (__const void *__src, void *__dest, size_t __n) ;


extern void bzero (void *__s, size_t __n) ;


extern int bcmp (__const void *__s1, __const void *__s2, size_t __n)
     __attribute__ ((__pure__));


extern char *index (__const char *__s, int __c) __attribute__ ((__pure__));


extern char *rindex (__const char *__s, int __c) __attribute__ ((__pure__));



extern int ffs (int __i) __attribute__ ((__const__));
# 287 "/usr/include/string.h" 3
extern int strcasecmp (__const char *__s1, __const char *__s2)
     __attribute__ ((__pure__));


extern int strncasecmp (__const char *__s1, __const char *__s2, size_t __n)
     __attribute__ ((__pure__));
# 309 "/usr/include/string.h" 3
extern char *strsep (char **__restrict __stringp,
                     __const char *__restrict __delim) ;
# 372 "/usr/include/string.h" 3
# 1 "/usr/include/bits/string.h" 1 3
# 25 "/usr/include/bits/string.h" 3
#define _STRING_ARCH_unaligned 1
# 373 "/usr/include/string.h" 2 3


# 1 "/usr/include/bits/string2.h" 1 3
# 46 "/usr/include/bits/string2.h" 3
#define __STRING_INLINE extern __inline
# 56 "/usr/include/bits/string2.h" 3
#define __STRING2_SMALL_GET16(src,idx) (((__const unsigned char *) (__const char *) (src))[idx + 1] << 8 | ((__const unsigned char *) (__const char *) (src))[idx])


#define __STRING2_SMALL_GET32(src,idx) (((((__const unsigned char *) (__const char *) (src))[idx + 3] << 8 | ((__const unsigned char *) (__const char *) (src))[idx + 2]) << 8 | ((__const unsigned char *) (__const char *) (src))[idx + 1]) << 8 | ((__const unsigned char *) (__const char *) (src))[idx])
# 93 "/usr/include/bits/string2.h" 3
#define __string2_1bptr_p(__x) ((size_t)(const void *)((__x) + 1) - (size_t)(const void *)(__x) == 1)





#define memset(s,c,n) (__extension__ (__builtin_constant_p (n) && (n) <= 16 ? ((n) == 1 ? __memset_1 (s, c) : __memset_gc (s, c, n)) : (__builtin_constant_p (c) && (c) == '\0' ? ({ void *__s = (s); __bzero (__s, n); __s; }) : memset (s, c, n))))
# 108 "/usr/include/bits/string2.h" 3
#define __memset_1(s,c) ({ void *__s = (s); *((__uint8_t *) __s) = (__uint8_t) c; __s; })


#define __memset_gc(s,c,n) ({ void *__s = (s); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (c); switch ((unsigned int) (n)) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })
# 192 "/usr/include/bits/string2.h" 3
#define __bzero(s,n) __builtin_memset (s, '\0', n)
# 389 "/usr/include/bits/string2.h" 3
extern void *__rawmemchr (const void *__s, int __c);
#define strchr(s,c) (__extension__ (__builtin_constant_p (c) && (c) == '\0' ? (char *) __rawmemchr (s, c) : strchr (s, c)))
# 756 "/usr/include/bits/string2.h" 3
#define strncpy(dest,src,n) (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n) ? (strlen (src) + 1 >= ((size_t) (n)) ? (char *) memcpy (dest, src, n) : strncpy (dest, src, n)) : strncpy (dest, src, n)))
# 778 "/usr/include/bits/string2.h" 3
#define strncat(dest,src,n) (__extension__ (__builtin_constant_p (src) && __builtin_constant_p (n) ? (strlen (src) < ((size_t) (n)) ? strcat (dest, src) : strncat (dest, src, n)) : strncat (dest, src, n)))
# 790 "/usr/include/bits/string2.h" 3
#define strcmp(s1,s2) __extension__ ({ size_t __s1_len, __s2_len; (__builtin_constant_p (s1) && __builtin_constant_p (s2) && (__s1_len = strlen (s1), __s2_len = strlen (s2), (!__string2_1bptr_p (s1) || __s1_len >= 4) && (!__string2_1bptr_p (s2) || __s2_len >= 4)) ? memcmp ((__const char *) (s1), (__const char *) (s2), (__s1_len < __s2_len ? __s1_len : __s2_len) + 1) : (__builtin_constant_p (s1) && __string2_1bptr_p (s1) && (__s1_len = strlen (s1), __s1_len < 4) ? (__builtin_constant_p (s2) && __string2_1bptr_p (s2) ? __strcmp_cc (s1, s2, __s1_len) : __strcmp_cg (s1, s2, __s1_len)) : (__builtin_constant_p (s2) && __string2_1bptr_p (s2) && (__s2_len = strlen (s2), __s2_len < 4) ? (__builtin_constant_p (s1) && __string2_1bptr_p (s1) ? __strcmp_cc (s1, s2, __s2_len) : __strcmp_gc (s1, s2, __s2_len)) : strcmp (s1, s2)))); })
# 811 "/usr/include/bits/string2.h" 3
#define __strcmp_cc(s1,s2,l) (__extension__ ({ register int __result = (((__const unsigned char *) (__const char *) (s1))[0] - ((__const unsigned char *) (__const char *)(s2))[0]); if (l > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (s1))[1] - ((__const unsigned char *) (__const char *) (s2))[1]); if (l > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (s1))[2] - ((__const unsigned char *) (__const char *) (s2))[2]); if (l > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (s1))[3] - ((__const unsigned char *) (__const char *) (s2))[3]); } } __result; }))
# 838 "/usr/include/bits/string2.h" 3
#define __strcmp_cg(s1,s2,l1) (__extension__ ({ __const unsigned char *__s2 = (__const unsigned char *) (__const char *) (s2); register int __result = (((__const unsigned char *) (__const char *) (s1))[0] - __s2[0]); if (l1 > 0 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (s1))[1] - __s2[1]); if (l1 > 1 && __result == 0) { __result = (((__const unsigned char *) (__const char *) (s1))[2] - __s2[2]); if (l1 > 2 && __result == 0) __result = (((__const unsigned char *) (__const char *) (s1))[3] - __s2[3]); } } __result; }))
# 860 "/usr/include/bits/string2.h" 3
#define __strcmp_gc(s1,s2,l2) (__extension__ ({ __const unsigned char *__s1 = (__const unsigned char *) (__const char *) (s1); register int __result = __s1[0] - ((__const unsigned char *) (__const char *) (s2))[0]; if (l2 > 0 && __result == 0) { __result = (__s1[1] - ((__const unsigned char *) (__const char *) (s2))[1]); if (l2 > 1 && __result == 0) { __result = (__s1[2] - ((__const unsigned char *) (__const char *) (s2))[2]); if (l2 > 2 && __result == 0) __result = (__s1[3] - ((__const unsigned char *) (__const char *) (s2))[3]); } } __result; }))
# 889 "/usr/include/bits/string2.h" 3
#define strncmp(s1,s2,n) (__extension__ (__builtin_constant_p (n) && ((__builtin_constant_p (s1) && strlen (s1) < ((size_t) (n))) || (__builtin_constant_p (s2) && strlen (s2) < ((size_t) (n)))) ? strcmp (s1, s2) : strncmp (s1, s2, n)))
# 903 "/usr/include/bits/string2.h" 3
#define strcspn(s,reject) __extension__ ({ char __r0, __r1, __r2; (__builtin_constant_p (reject) && __string2_1bptr_p (reject) ? ((__r0 = ((__const char *) (reject))[0], __r0 == '\0') ? strlen (s) : ((__r1 = ((__const char *) (reject))[1], __r1 == '\0') ? __strcspn_c1 (s, __r0) : ((__r2 = ((__const char *) (reject))[2], __r2 == '\0') ? __strcspn_c2 (s, __r0, __r1) : (((__const char *) (reject))[3] == '\0' ? __strcspn_c3 (s, __r0, __r1, __r2) : strcspn (s, reject))))) : strcspn (s, reject)); })
# 919 "/usr/include/bits/string2.h" 3
extern __inline size_t __strcspn_c1 (__const char *__s, int __reject);
extern __inline size_t
__strcspn_c1 (__const char *__s, int __reject)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject)
    ++__result;
  return __result;
}

extern __inline size_t __strcspn_c2 (__const char *__s, int __reject1,
                                     int __reject2);
extern __inline size_t
__strcspn_c2 (__const char *__s, int __reject1, int __reject2)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject1
         && __s[__result] != __reject2)
    ++__result;
  return __result;
}

extern __inline size_t __strcspn_c3 (__const char *__s, int __reject1,
                                     int __reject2, int __reject3);
extern __inline size_t
__strcspn_c3 (__const char *__s, int __reject1, int __reject2,
              int __reject3)
{
  register size_t __result = 0;
  while (__s[__result] != '\0' && __s[__result] != __reject1
         && __s[__result] != __reject2 && __s[__result] != __reject3)
    ++__result;
  return __result;
}







#define strspn(s,accept) __extension__ ({ char __a0, __a1, __a2; (__builtin_constant_p (accept) && __string2_1bptr_p (accept) ? ((__a0 = ((__const char *) (accept))[0], __a0 == '\0') ? ((void) (s), 0) : ((__a1 = ((__const char *) (accept))[1], __a1 == '\0') ? __strspn_c1 (s, __a0) : ((__a2 = ((__const char *) (accept))[2], __a2 == '\0') ? __strspn_c2 (s, __a0, __a1) : (((__const char *) (accept))[3] == '\0' ? __strspn_c3 (s, __a0, __a1, __a2) : strspn (s, accept))))) : strspn (s, accept)); })
# 976 "/usr/include/bits/string2.h" 3
extern __inline size_t __strspn_c1 (__const char *__s, int __accept);
extern __inline size_t
__strspn_c1 (__const char *__s, int __accept)
{
  register size_t __result = 0;

  while (__s[__result] == __accept)
    ++__result;
  return __result;
}

extern __inline size_t __strspn_c2 (__const char *__s, int __accept1,
                                    int __accept2);
extern __inline size_t
__strspn_c2 (__const char *__s, int __accept1, int __accept2)
{
  register size_t __result = 0;

  while (__s[__result] == __accept1 || __s[__result] == __accept2)
    ++__result;
  return __result;
}

extern __inline size_t __strspn_c3 (__const char *__s, int __accept1,
                                    int __accept2, int __accept3);
extern __inline size_t
__strspn_c3 (__const char *__s, int __accept1, int __accept2, int __accept3)
{
  register size_t __result = 0;

  while (__s[__result] == __accept1 || __s[__result] == __accept2
         || __s[__result] == __accept3)
    ++__result;
  return __result;
}






#define strpbrk(s,accept) __extension__ ({ char __a0, __a1, __a2; (__builtin_constant_p (accept) && __string2_1bptr_p (accept) ? ((__a0 = ((__const char *) (accept))[0], __a0 == '\0') ? ((void) (s), (char *) NULL) : ((__a1 = ((__const char *) (accept))[1], __a1 == '\0') ? strchr (s, __a0) : ((__a2 = ((__const char *) (accept))[2], __a2 == '\0') ? __strpbrk_c2 (s, __a0, __a1) : (((__const char *) (accept))[3] == '\0' ? __strpbrk_c3 (s, __a0, __a1, __a2) : strpbrk (s, accept))))) : strpbrk (s, accept)); })
# 1033 "/usr/include/bits/string2.h" 3
extern __inline char *__strpbrk_c2 (__const char *__s, int __accept1,
                                     int __accept2);
extern __inline char *
__strpbrk_c2 (__const char *__s, int __accept1, int __accept2)
{

  while (*__s != '\0' && *__s != __accept1 && *__s != __accept2)
    ++__s;
  return *__s == '\0' ? ((void *)0) : (char *) (size_t) __s;
}

extern __inline char *__strpbrk_c3 (__const char *__s, int __accept1,
                                     int __accept2, int __accept3);
extern __inline char *
__strpbrk_c3 (__const char *__s, int __accept1, int __accept2,
              int __accept3)
{

  while (*__s != '\0' && *__s != __accept1 && *__s != __accept2
         && *__s != __accept3)
    ++__s;
  return *__s == '\0' ? ((void *)0) : (char *) (size_t) __s;
}
# 1076 "/usr/include/bits/string2.h" 3
#define __strtok_r(s,sep,nextp) (__extension__ (__builtin_constant_p (sep) && __string2_1bptr_p (sep) ? (((__const char *) (sep))[0] != '\0' && ((__const char *) (sep))[1] == '\0' ? __strtok_r_1c (s, ((__const char *) (sep))[0], nextp) : __strtok_r (s, sep, nextp)) : __strtok_r (s, sep, nextp)))
# 1085 "/usr/include/bits/string2.h" 3
extern __inline char *__strtok_r_1c (char *__s, char __sep, char **__nextp);
extern __inline char *
__strtok_r_1c (char *__s, char __sep, char **__nextp)
{
  char *__result;
  if (__s == ((void *)0))
    __s = *__nextp;
  while (*__s == __sep)
    ++__s;
  __result = ((void *)0);
  if (*__s != '\0')
    {
      __result = __s++;
      while (*__s != '\0')
        if (*__s++ == __sep)
          {
            __s[-1] = '\0';
            break;
          }
      *__nextp = __s;
    }
  return __result;
}

#define strtok_r(s,sep,nextp) __strtok_r (s, sep, nextp)







extern char *__strsep_g (char **__stringp, __const char *__delim);
#define __strsep(s,reject) __extension__ ({ char __r0, __r1, __r2; (__builtin_constant_p (reject) && __string2_1bptr_p (reject) && (__r0 = ((__const char *) (reject))[0], ((__const char *) (reject))[0] != '\0') ? ((__r1 = ((__const char *) (reject))[1], ((__const char *) (reject))[1] == '\0') ? __strsep_1c (s, __r0) : ((__r2 = ((__const char *) (reject))[2], __r2 == '\0') ? __strsep_2c (s, __r0, __r1) : (((__const char *) (reject))[3] == '\0' ? __strsep_3c (s, __r0, __r1, __r2) : __strsep_g (s, reject)))) : __strsep_g (s, reject)); })
# 1135 "/usr/include/bits/string2.h" 3
extern __inline char *__strsep_1c (char **__s, char __reject);
extern __inline char *
__strsep_1c (char **__s, char __reject)
{
  register char *__retval = *__s;
  if (__retval != ((void *)0) && (*__s = (__extension__ (__builtin_constant_p (__reject) && (__reject) == '\0' ? (char *) __rawmemchr (__retval, __reject) : strchr (__retval, __reject)))) != ((void *)0))
    *(*__s)++ = '\0';
  return __retval;
}

extern __inline char *__strsep_2c (char **__s, char __reject1, char __reject2);
extern __inline char *
__strsep_2c (char **__s, char __reject1, char __reject2)
{
  register char *__retval = *__s;
  if (__retval != ((void *)0))
    {
      register char *__cp = __retval;
      while (1)
        {
          if (*__cp == '\0')
            {
              __cp = ((void *)0);
          break;
            }
          if (*__cp == __reject1 || *__cp == __reject2)
            {
              *__cp++ = '\0';
              break;
            }
          ++__cp;
        }
      *__s = __cp;
    }
  return __retval;
}

extern __inline char *__strsep_3c (char **__s, char __reject1, char __reject2,
                                   char __reject3);
extern __inline char *
__strsep_3c (char **__s, char __reject1, char __reject2, char __reject3)
{
  register char *__retval = *__s;
  if (__retval != ((void *)0))
    {
      register char *__cp = __retval;
      while (1)
        {
          if (*__cp == '\0')
            {
              __cp = ((void *)0);
          break;
            }
          if (*__cp == __reject1 || *__cp == __reject2 || *__cp == __reject3)
            {
              *__cp++ = '\0';
              break;
            }
          ++__cp;
        }
      *__s = __cp;
    }
  return __retval;
}

#define strsep(s,reject) __strsep (s, reject)
# 1210 "/usr/include/bits/string2.h" 3
#define __need_malloc_and_calloc 
# 1 "/usr/include/stdlib.h" 1 3
# 28 "/usr/include/stdlib.h" 3
#define __need_size_t 




# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 34 "/usr/include/stdlib.h" 2 3


# 553 "/usr/include/stdlib.h" 3
#define __malloc_and_calloc_defined 


extern void *malloc (size_t __size) __attribute__ ((__malloc__));

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__malloc__));

# 920 "/usr/include/stdlib.h" 3
#undef __need_malloc_and_calloc


# 1212 "/usr/include/bits/string2.h" 2 3




extern char *__strdup (__const char *__string) __attribute__ ((__malloc__));
#define __strdup(s) (__extension__ (__builtin_constant_p (s) && __string2_1bptr_p (s) ? (((__const char *) (s))[0] == '\0' ? (char *) calloc (1, 1) : ({ size_t __len = strlen (s) + 1; char *__retval = (char *) malloc (__len); if (__retval != NULL) __retval = (char *) memcpy (__retval, s, __len); __retval; })) : __strdup (s)))
# 1229 "/usr/include/bits/string2.h" 3
#define strdup(s) __strdup (s)





extern char *__strndup (__const char *__string, size_t __n)
     __attribute__ ((__malloc__));
#define __strndup(s,n) (__extension__ (__builtin_constant_p (s) && __string2_1bptr_p (s) ? (((__const char *) (s))[0] == '\0' ? (char *) calloc (1, 1) : ({ size_t __len = strlen (s) + 1; size_t __n = (n); char *__retval; if (__n < __len) __len = __n + 1; __retval = (char *) malloc (__len); if (__retval != NULL) { __retval[__len - 1] = '\0'; __retval = (char *) memcpy (__retval, s, __len - 1); } __retval; })) : __strndup (s, n)))
# 1264 "/usr/include/bits/string2.h" 3
#undef __STRING_INLINE
# 376 "/usr/include/string.h" 2 3




# 19 "../../include/define.h" 2

#define DEF_PREPAID_CONF_FILE "/DSC/NEW/DATA/PPS.conf"
#define DEF_URLMATCH_FILE "/DSC/NEW/DATA/UDR_CATEGORY.conf"
#define DEF_UDR_DUMPCONF_FILE "/DSC/NEW/DATA/UDR_DUMP.conf"
#define DEF_UDR_TXCCONF_FILE "/DSC/NEW/DATA/UDR_TXC.conf"
#define DEF_CDR_SVCTYPE_FILE "/DSC/NEW/DATA/UDR_CDRINFO.conf"
#define UDRGEN_DUMP_PREFIX "/DSC/UDR"

#define DEF_INIT_FILE START_PATH"/NEW/DATA/INIT_IPAF.dat"
#define DEF_VER_FILE START_PATH"/NEW/DATA/SW_VER.dat"
#define DEF_SVC_TIMEOUT START_PATH"/NEW/DATA/SVC_TIMEOUT.dat"

#define DEF_MACS_TIMEOUT_FILE START_PATH"/NEW/DATA/MACS_TIMEOUT.dat"
#define DEF_WICGS_TIMEOUT_FILE START_PATH"/NEW/DATA/WICGS_TIMEOUT.dat"

#define MAX_MIN_SIZE 17
#define MAX_MIN_LEN 16
#define MAX_MSISDN_SIZE 17

#define MAX_IPAF_MULTI 2
#define MAX_IPAF_PAIR 2

#define MAX_CATESVC_COUNT 21
#define MAX_WATCH_COUNT 10


#define MAX_IMSI_SIZE 17
#define MAX_WINSVCNAME_SIZE 6
#define MAX_CELLINFO_SIZE 17

#define MAX_FILENAME_SIZE 256

#define MAX_CON_SIZE 64
#define MAX_CON_COUNT 100


#define LOG_NOPRINT 0
#define LOG_CRI 1
#define LOG_WARN 2
#define LOG_DEBUG 3
#define LOG_INFO 4

#define LOG_TYPE_DEBUG 1
#define LOG_TYPE_WRITE 2

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10
#define TCP_SYNACK 0x12


#define DEF_FILLTEROUT_OFF 0x00
#define DEF_FILLTEROUT_ON 0x01
#define DEF_FILLTEROUT_RPASS 0x02
#define DEF_QUD_SND_TIME 5

#define DEF_FLAG_OFF 0x00
#define DEF_FLAG_ON 0x01

#define DEF_CALL_CLEAR_NOREMAIN 1000

#define MAX_AAA_SOCK 1
#define MAX_UDR_COUNT 3

//#define SYS_BSDA "DSCA"
//#define SYS_BSDB "DSCB"
#define SYS_BSDA "SCMA"
#define SYS_BSDB "SCMB"

#define MAX_RADIUS_PKTSIZE 5000
#define TRACE_IMSI 1
#define TRACE_MSISDN 2

#define MAX_TRACE_LEN 32
#define MAX_TRCMSG_SIZE 2700


#define DEF_INDEX_INIT 100
#define MAX_INDEX_VALUE 60000
# 23 "../../include/ipaf_svc.h" 2
# 1 "../../include/comm_typedef.h" 1

#define COMM_TYPEDEF_H 

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef int INT;
typedef unsigned int UINT;
typedef long long INT64;
typedef long LONG;
typedef unsigned long ULONG;
typedef short SHORT;
typedef char CHAR;
typedef struct timeval st_TimeVal;
# 24 "../../include/ipaf_svc.h" 2
# 1 "../../include/comm_msgtypes.h" 1

#define __COMM_MSGTYPES_H__ 





#define COMM_MAX_NAME_LEN 16
#define COMM_MAX_VALUE_LEN 100




#define MTYPE_SETPRINT 1
#define MTYPE_IXPC_CONNECTION_CHECK 2
#define MTYPE_MMC_REQUEST 3
#define MTYPE_MMC_RESPONSE 4
#define MTYPE_STATISTICS_REQUEST 5
#define MTYPE_STATISTICS_REPORT 6
#define MTYPE_STATUS_REQUEST 7
#define MTYPE_STATUS_REPORT 8
#define MTYPE_ALARM_REPORT 9
#define MTYPE_STAT_REPORT_SHORT_TERM 10
#define MTYPE_STAT_REPORT_LONG_TERM 11
#define MTYPE_MMC_SYNTAX_RESPONSE 12
#define MTYPE_DB_DISCONNECT 13
#define MTYPE_TRC_CONSOLE 14
#define MTYPE_MAP_NOTIFICATION 15

#define MTYPE_BSD_ALARM_REPORT 16


#define MTYPE_UDR_GET 31
#define MTYPE_UDR_GET_RESP 32


#define MTYPE_BSD_CONFIG 41


#define MTYPE_DUP_STATUS_REQUEST 50
#define MTYPE_DUP_STATUS_RESPONSE 51
# 52 "../../include/comm_msgtypes.h"
#define MSGID_LOAD_STATISTICS_REPORT 100
#define MSGID_FAULT_STATISTICS_REPORT 101
#define MSGID_IPAF_STATISTICS_REPORT 104
#define MSGID_AAA_STATISTICS_REPORT 105
#define MSGID_OB_STATISTICS_REPORT 110
#define MSGID_WISE_STATISTICS_REPORT 111
#define MSGID_SCIB_STATISTICS_REPORT 112
#define MSGID_SCPIF_STATISTICS_REPORT 113
#define MSGID_RCIF_STATISTICS_REPORT 114
#define MSGID_DB_STATISTICS_REPORT 115
#define MSGID_OBC_STATISTICS_REPORT 116

#define MSGID_ALTIBASELOG_REQUEST 210
#define MSGID_ALTIBASELOG_RESPONSE 211

#define MSGID_SYS_COMM_STATUS_REPORT 300
#define MSGID_SYS_SPEC_CONN_STATUS_REPORT 301
#define MSGID_SYS_SPEC_HW_STATUS_REPORT 302
#define MSGID_SYS_SPEC_NMDB_STATUS_REPORT 303

#define MSGID_WATCHDOG_STATUS_REPORT 310
#define MSGID_SYS_TCPCON_STATUS_REPORT 320

typedef struct {
        unsigned char pres;
        unsigned char octet;
} SingleOctetType;

typedef struct {
        unsigned char pres;
#define MAX_OCTET_STRING_LEN 32
        unsigned char octet[32];
} OctetStringType;


typedef struct {
        long mtype;
#define MAX_GEN_QMSG_LEN 8192-(sizeof(long))
        char body[8192-(sizeof(long))];
} GeneralQMsgType;


typedef struct {
        int msgId;
        char segFlag;
        char seqNo;
        char dummy[2];
#define BYTE_ORDER_TAG 0x1234
        short byteOrderFlag;
        short bodyLen;
        char srcSysName[16];
        char srcAppName[16];
        char dstSysName[16];
        char dstAppName[16];
} IxpcQMsgHeadType;

typedef struct {
        IxpcQMsgHeadType head;
#define MAX_IXPC_QMSG_LEN (MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType)
        char body[(8192-(sizeof(long)))-sizeof(IxpcQMsgHeadType)];
} IxpcQMsgType;

#define MML_MAX_CMD_NAME_LEN 32
#define MML_MAX_PARA_CNT 50
#define MML_MAX_PARA_VALUE_LEN 32



typedef struct {
    char paraName[16];
    char paraVal[100];
} CommPara;

typedef struct {
        unsigned short mmcdJobNo;
        unsigned short paraCnt;
        char cmdName[32];
        CommPara para[50];




} MMLReqMsgHeadType;

typedef struct {
        MMLReqMsgHeadType head;
} MMLReqMsgType;




typedef struct {
        unsigned short mmcdJobNo;
        unsigned short extendTime;
        char resCode;
        char contFlag;
        char cmdName[32];
} MMLResMsgHeadType;

typedef struct {
        MMLResMsgHeadType head;
#define MAX_MML_RESULT_LEN ((MAX_GEN_QMSG_LEN)-sizeof(IxpcQMsgHeadType))-sizeof(MMLResMsgHeadType)
        char body[((8192-(sizeof(long)))-sizeof(IxpcQMsgHeadType))-sizeof(MMLResMsgHeadType)];
} MMLResMsgType;





typedef struct {
        int cliReqId;
        int confirm;
        int batchFlag;
        int clientType;
} MMLClientReqMsgHeadType;

#pragma pack(1)
typedef struct {
        MMLClientReqMsgHeadType head;
        char body[4000];
} MMLClientReqMsgType;
#pragma pack()



typedef struct {
        int cliReqId;
        int confirm;
        int batchFlag;
    int errCode;
        char resCode;
        char contFlag;
        char segFlag;
        char seqNo;
} MMLClientResMsgHeadType;

#pragma pack(1)
typedef struct {
        MMLClientResMsgHeadType head;
        char body[4000];
} MMLClientResMsgType;
#pragma pack()

typedef struct {
    char processName[16];
    int Pid;
        int type;
} IFB_KillPrcNotiMsgType;


typedef struct {
        char dup_status;
        char samd_status;
        char response_result;
} dup_status_res;
# 25 "../../include/ipaf_svc.h" 2

#define FATAL_ERROR 5
#define MAJOR_ERROR 4
#define MINOR_ERROR 3
#define WARNING 2
#define NOTIFICATION 1




#define POST_PAID 1
#define PRE_PAID 2

#define NO_REMAIN 4


#define CDR_UNKNOWN 0
#define CDR_END 1
#define CDR_INT 3


#define OK 0
#define DECTED 1
#define NOT_OK 2
#define DUP 3


#define TIME 1
#define BYTE 2
#define TIME_PLUS_BYTE 3


#define NORMAL_END 1
#define ABNORMAL_END 2


#define DEF_SVC 1000
#define DEF_SYS 5000

#define MAX_EXTRA_SIZE 95
#define MAX_MSGBODY_SIZE 4000
#define DEF_CONTENT_SIZE 53
#define DEF_SER_ALIAS_SIZE 24+6

#define BILLCOM_HEAD_SIZE 96
#define MACS_HEAD_SIZE 48
#define WICGS_HEAD_SIZE 56
#define WICGS_BILLCOM_SIZE 108


typedef struct _st_CondCount
{
        USHORT usTotPage;
        USHORT usCurPage;
        USHORT usSerial;
        USHORT usReserved;
}st_CondCount, *pst_CondCount;



typedef struct _st_MsgQ
{
    long int llMType;
        int uiReserved;
        INT64 llNID;
        UCHAR szIPAFID[2];
        UCHAR ucProID;
        char szReserved[5];
    INT64 llIndex;
        INT dMsgQID;
        UINT uiNaIP;
        UCHAR ucNaType;
        UCHAR szReserved2;
        USHORT usCatID;
        USHORT usBodyLen;
        USHORT usRetCode;
        UCHAR szMIN[17];
        UCHAR szExtra[95];
    UCHAR szBody[4000];
} st_MsgQ, *pst_MsgQ;

#define DEF_MSGQ_SIZE sizeof(st_MsgQ)
#define DEF_MSGHEAD_LEN (sizeof(st_MsgQ) - MAX_MSGBODY_SIZE)


typedef struct _st_MsgQSub
{
    USHORT usType;
        UCHAR usSvcID;
        UCHAR usMsgID;
} st_MsgQSub, *pst_MsgQSub;


typedef struct _st_NID
{
    char ucReserved;
    UCHAR ucSysType;
    USHORT usSerial;
    time_t stBuild;
} st_NID, *pst_NID;

typedef union _un_NID
{
        INT64 llNID;
        st_NID stNID;
} un_NID, *pun_NID;


typedef union _un_CRN
{
        INT64 llCRN;
        struct {
                INT64 ucReserved:8;
                INT64 ucSide:1;
                INT64 ucIPAFID:7;
                INT64 usSerial:16;
                INT64 dTime:32;
        } stBody;
} un_CRN, *pun_CRN;



typedef struct _st_STOPMsgKey
{
    INT64 llAcctSessID;
    UINT uiSrcIP;
    USHORT usSrcPort;
        UCHAR ucRDRSndType;
    UCHAR ucPayType;
    struct timeval tSndTime;
        UCHAR szMDN[17 -1];
        USHORT usCatID;
        UCHAR ucURLChar;
        char szReserved[5];
}st_STOPMsgKey, *pst_STOPMsgKey;




#define SID_ACC_S 11
#define SID_ACC_E 21
#define SID_QUD 31
#define SID_CDR 41
#define SID_PPS 51
#define SID_CALL 61
#define SID_CLEAR 71


#define MID_SESS_CLEAR 101

#define MID_GEN_SREQ 11
#define MID_GEN_SRES 12
#define MID_GEN_EREQ 21
#define MID_GEN_ERES 22
#define MID_PPS_SREQ 31
#define MID_PPS_SRES 32
#define MID_PPS_EREQ 41
#define MID_PPS_ERES 42
#define MID_CDR_REQ 51
#define MID_CDR_RES 52
#define MID_PPS_REQ 61
#define MID_PPS_RES 62
#define MID_QUD_REQ 71
#define MID_QUD_RES 72


#define SID_ACC_REQ 91
#define MID_ACC_START 11
#define MID_ACC_RESPONSE 12
#define MID_ACC_END 13
#define MID_ACC_INTERIM 15
#define MID_ACC_REQ 17
#define MID_ACC_ACPT 18
#define MID_ACC_REJ 19

#define MID_ACC_UDR 21

#define MID_ACCT_HTTP 25
#define MID_ACCT_UAWAP 26
#define MID_ACCT_WAP1 27
#define MID_ACCT_WAP2 29

#define DEF_ACCESS_REQUEST 1
#define DEF_ACCESS_ACCEPT 2
#define DEF_ACCESS_REJECT 3
#define DEF_ACCOUNTING_REQ 4
#define DEF_ACCOUNTING_RES 5


#define SID_DUP 101
#define MID_STB_REQ 101
#define MID_STB_RES 102
#define MID_ACT_REQ 111
#define MID_ACT_RES 112
# 250 "../../include/ipaf_svc.h"
typedef struct _st_AcctStartReq
{
    UCHAR ucPayType;
    UCHAR ucAmountType;
    UINT uiPacketPeriod;
    UINT uiTimePeriod;
}st_AcctStartReq, *pst_AcctStartReq;


typedef struct _st_AcctEndReq
{
        UCHAR ucReason;
}st_AcctEndReq, *pst_AcctEndReq;


typedef struct _st_CDRInfo
{
    time_t dStartTime;
    time_t dLastTime;
        INT64 llUTCPAmount;
        INT64 llDTCPAmount;
        INT64 llUUDPAmount;
        INT64 llDUDPAmount;
        INT64 llUETCAmount;
        INT64 llDETCAmount;
    INT64 llUAmount;
    INT64 llDAmount;
    INT64 llRUpAmount;
    INT64 llRDownAmount;
    UINT uiUCount;
    UINT uiDCount;
    UINT uiRUCount;
    UINT uiRDCount;
    USHORT usSVCCategory;
        char szReserved[6];
} st_CDRInfo, *pst_CDRInfo;
#define DEF_CDRINFO_LEN sizeof(st_CDRInfo)

typedef struct _st_CDR
{
        UCHAR ucPayType;
        UCHAR ucReserved1;
        USHORT usSerial;
        UINT uiReserved1;

        UINT uiUPacket;
        UINT uiDPacket;

        UINT dStartTime;
        UINT dLastTime;

        UCHAR ucCateCount;
        UCHAR szReserved1[7];
        st_CDRInfo stCDRInfo[21];
} st_CDR, *pst_CDR;
#define DEF_CDR_LEN 32

typedef struct _st_Alive
{
        UINT uiIdleTime;
} st_AliveReqMsg, *pst_AliveReqMsg;
#define DEF_ALIVEREQ_LEN 4



typedef struct _st_AAAFlgInfo
{
        UCHAR ucTimeStampF;
        UCHAR ucUDRSeqF;
        UCHAR ucCallStatIDF;
        UCHAR ucESNF;
        UCHAR ucFramedIPF;
    UCHAR ucAcctSessIDF;
    UCHAR ucCorrelationIDF;
    UCHAR ucSessContinueF;

        UCHAR ucUserNameF;
        UCHAR ucSvcOptF;
        UCHAR ucNASIPF;
        UCHAR ucPCFIPF;
        UCHAR ucHAIPF;
        UCHAR ucBSIDF;
        UCHAR ucFwdFCHMuxF;
        UCHAR ucRevFCHMuxF;

        UCHAR ucFwdTrafTypeF;
        UCHAR ucRevTrafTypeF;
        UCHAR ucFCHSizeF;
        UCHAR ucFwdFCHRCF;
        UCHAR ucRevFCHRCF;
        UCHAR ucIPTechF;
        UCHAR ucDCCHSizeF;
        UCHAR ucReleaseIndF;

        UCHAR ucNASPortF;
        UCHAR ucNASPortTypeF;
        UCHAR ucNASPortIDF;
        UCHAR ucSvcTypeF;
        UCHAR ucAcctStatTypeF;
        UCHAR ucCompTunnelIndF;
        UCHAR ucNumActF;
    UCHAR ucAcctInOctF;

        UCHAR ucAcctOutOctF;
        UCHAR ucAlwaysOnF;
        UCHAR ucAcctInPktF;
        UCHAR ucAcctOutPktF;
        UCHAR ucBadPPPFrameCntF;
        UCHAR ucActTimeF;
        UCHAR ucTermSDBOctCntF;
        UCHAR ucOrgSDBOctCntF;

        UCHAR ucTermNumSDBF;
        UCHAR ucOrgNumSDBF;
        UCHAR ucEventTimeF;
        UCHAR ucRcvHDLCOctF;
        UCHAR ucIPQoSF;
        UCHAR ucAcctSessTimeF;
        UCHAR ucAcctAuthF;
        UCHAR ucAcctTermCauseF;

        UCHAR ucAcctDelayTimeF;
        UCHAR ucAirQoSF;
        UCHAR ucUserIDF;
        UCHAR ucMDNF;
        UCHAR ucRPConnectIDF;
        UCHAR ucInMIPSigCntF;
        UCHAR ucOutMIPSigCntF;
    UCHAR ucBeginningSessF;


        UCHAR ucDataSvcTypeF;
        UCHAR ucTransIDF;
        UCHAR ucReqTimeF;
        UCHAR ucResTimeF;
        UCHAR ucSessTimeF;
        UCHAR ucDestIPF;
        UCHAR ucDestPortF;
        UCHAR ucSrcPortF;

        UCHAR ucURLF;
        UCHAR ucCTypeF;

        UCHAR ucAppIDF;
        UCHAR ucCntCodeF;
        UCHAR ucMethTypeF;
        UCHAR ucResultCodeF;
        UCHAR ucIPUpSizeF;
        UCHAR ucIPDownSizeF;

        UCHAR ucReInputSizeF;
        UCHAR ucReOutputSizeF;
        UCHAR ucCntLenF;
        UCHAR ucTransCompleteF;
        UCHAR ucTransTermReasonF;
        UCHAR ucUserAgentF;
        UCHAR ucRetryF;
        UCHAR ucReserved;

} st_AAAFlgInfo, *pst_AAAFlgInfo;




#define MAX_AUTH_SIZE 16
#define MAX_ESN_SIZE 16
#define MAX_BSID_SIZE 12
#define MAX_PORT_SIZE 24
#define MAX_USERNAME_SIZE 72

#define MAX_URL_SIZE 151
#define MAX_USERAGENT_SIZE 61
#define MAX_HOST_LEN 61

#define MAX_TRANSPORT_SIZE 201
#define MAX_SESSION_SIZE 51
#define MAX_BILLINFO_SIZE 101


typedef struct _st_ACCInfo
{
        UCHAR ucCode;
        UCHAR ucID;
        UCHAR ucUserLen;
    UCHAR ucUDRSeqF;
    UCHAR ucTimeStampF;
        UCHAR ucCallStatIDF;
        UCHAR ucESNF;
        UCHAR ucFramedIPF;

    UCHAR ucAcctSessIDF;
    UCHAR ucCorrelationIDF;
    UCHAR ucSessContinueF;
        UCHAR ucUserNameF;
        UCHAR ucSvcOptF;
        UCHAR ucUserIDF;
        UCHAR ucNASIPF;
        UCHAR ucHAIPF;

        UCHAR ucPCFIPF;
        UCHAR ucBSIDF;
        UCHAR ucFwdFCHMuxF;
        UCHAR ucRevFCHMuxF;
        UCHAR ucFwdTrafTypeF;
        UCHAR ucRevTrafTypeF;
        UCHAR ucFCHSizeF;
        UCHAR ucFwdFCHRCF;

        UCHAR ucRevFCHRCF;
        UCHAR ucIPTechF;
        UCHAR ucDCCHSizeF;
        UCHAR ucReleaseIndF;
        UCHAR ucNASPortF;
        UCHAR ucNASPortTypeF;
        UCHAR ucNASPortIDF;
        UCHAR ucNASPortIDLen;

        UCHAR ucSvcTypeF;
        UCHAR ucAcctStatTypeF;
        UCHAR ucNumActF;
    UCHAR ucAcctInOctF;
        UCHAR ucAcctOutOctF;
        UCHAR ucAlwaysOnF;
        UCHAR ucAcctInPktF;
        UCHAR ucAcctOutPktF;

        UCHAR ucBadPPPFrameCntF;
        UCHAR ucEventTimeF;
        UCHAR ucActTimeF;
        UCHAR ucTermSDBOctCntF;
        UCHAR ucOrgSDBOctCntF;
        UCHAR ucTermNumSDBF;
        UCHAR ucOrgNumSDBF;
        UCHAR ucRcvHDLCOctF;

        UCHAR ucIPQoSF;
        UCHAR ucAcctSessTimeF;
        UCHAR ucCompTunnelIndF;
        UCHAR ucAcctAuthF;
        UCHAR ucAcctTermCauseF;
        UCHAR ucAcctDelayTimeF;
        UCHAR ucAirQoSF;
        UCHAR ucRPConnectIDF;

        UCHAR ucInMIPSigCntF;
        UCHAR ucOutMIPSigCntF;
        UCHAR ucMDNF;
    UCHAR ucAAAIPF;
    UCHAR ucRetryF;
        UCHAR ucAcctInterimF;
    UCHAR ucBeginningSessF;
    UCHAR szReservd[1];


    UINT uiUDRSeq;
    UINT uiTimeStamp;
    UINT uiAAAIP;
    UINT uiKey;

    UINT uiFramedIP;
        UINT uiNASIP;
        UINT uiPCFIP;
        UINT uiHAIP;

    UINT uiRADIUSLen;
    UINT uiSessContinue;
    UINT uiBeginnigSess;
        INT dSvcOpt;

        INT dAcctStatType;
        INT dCompTunnelInd;
        INT dNumAct;
        INT dSvcType;

    INT dFwdFCHMux;
    INT dRevFCHMux;
    INT dFwdTrafType;
    INT dRevTrafType;

    INT dFCHSize;
    INT dFwdFCHRC;
    INT dRevFCHRC;
    INT dIPTech;

        INT dDCCHSize;
        INT dNASPort;
        INT dNASPortType;
    INT dReleaseInd;

    INT dAcctInOct;
    INT dAcctOutOct;
    INT dAcctInPkt;
    INT dAcctOutPkt;

    UINT uiEventTime;
    UINT uiActTime;
    UINT uiAcctSessTime;
    UINT uiAcctDelayTime;

    INT dTermSDBOctCnt;
    INT dOrgSDBOctCnt;
    INT dTermNumSDB;
    INT dOrgNumSDB;

    INT dRcvHDLCOct;
    INT dIPQoS;
    INT dAirQoS;
    INT dRPConnectID;

    INT dBadPPPFrameCnt;
    INT dAcctAuth;
    INT dAcctTermCause;
    INT dAlwaysOn;

    INT dUserID;
    INT dInMIPSigCnt;
    INT dOutMIPSigCnt;
        INT dAcctInterim;

    INT64 llAcctSessID;
        INT64 llCorrelID;
    UINT uiRetryFlg;
    INT dReserved;

        UCHAR szAuthen [16];
        UCHAR szMDN [17];
        UCHAR szESN [16];
        UCHAR szUserName [72];
        UCHAR szBSID [12];
        UCHAR szNASPortID [24];
        UCHAR szMIN [17];

} st_ACCInfo, *pst_ACCInfo;

#define DEF_ACCINFO_SIZE sizeof(st_ACCInfo)


typedef struct _st_UDRInfo_
{
    UCHAR ucAcctSessIDF;

        UCHAR ucDataSvcTypeF;
        UCHAR ucTranIDF;
    UCHAR ucReqTimeF;
    UCHAR ucResTimeF;
        UCHAR ucSessionTimeF;

    UCHAR ucDestIPF;
    UCHAR ucDestPortF;
    UCHAR ucSrcPortF;
        UCHAR ucCTypeF;

        UCHAR ucAppIDF;
        UCHAR ucContentCodeF;
        UCHAR ucMethodTypeF;
        UCHAR ucResultCodeF;

        UCHAR ucIPUpSizeF;
        UCHAR ucIPDownSizeF;
        UCHAR ucRetransInSizeF;
        UCHAR ucRetransOutSizeF;

        UCHAR ucContentLenF;
        UCHAR ucTranCompleteF;
        UCHAR ucTranTermReasonF;

        UCHAR ucURLF;
        UCHAR ucUserAgentF;
        UCHAR ucHostF;
        UCHAR ucMDNF;
    INT64 llAcctSessID;

        INT dDataSvcType;
        UINT uiTranID;
    time_t tReqTime;
    time_t tResTime;
        time_t tSessionTime;

    UINT uiDestIP;
    INT dDestPort;
    INT dSrcPort;
        INT dCType;

        INT dAppID;
        INT dContentCode;
        INT dMethodType;
        INT dResultCode;

        INT dIPUpSize;
        INT dIPDownSize;
        INT dRetransInSize;
        INT dRetransOutSize;

        INT dContentLen;
        INT dTranComplete;
        INT dTranTermReason;

        char szURL[151];
        char szUserAgent[61];
        char szHost[61];
        char szMDN[17 -1];
        UCHAR ucURLCha;
} st_UDRInfo, *pst_UDRInfo;


typedef struct _st_AAAREQ
{
    st_ACCInfo stInfo;

    INT dUDRCount;
    INT dReserved;

    st_UDRInfo stUDRInfo[3];
}st_AAAREQ, *pst_AAAREQ;




#define MAX_BODY_SIZE 4096
#define MAX_QUEUE_NUM 10000

typedef struct _st_Queue {
    UINT uiUDRSeq;
    UINT uiKey;
    INT64 llAcctSessID;
    UCHAR szMIN[17];
    UCHAR szReserved[3];
    UCHAR szBody[4096];
} st_Queue, *pst_Queue;

typedef struct _st_QueueList {
    short tail;
    short head;
    st_Queue stQueue[10000];
} st_QueueList, *pst_QueueList;



typedef struct _st_StanbyInfo{
    UINT uiAAAIP;
    UCHAR szMIN[17];
    CHAR szBody[4096];
} st_StandbyInfo, *pst_StandbyInfo;






#define DEF_ACCINFO_SIZE sizeof(st_ACCInfo)


typedef struct _st_SndDSCP
{

        char szIMSI[17];
        char szWinSvcName[6];
    UINT uiWSType;
        UINT uiSVCOption;
        USHORT usTerminal;
        char szCellInfo[17];


        INT64 llCallID;
        UCHAR ucMode;
        UCHAR ucStatus;
        UINT uiSeqID;
        time_t uiStartTime;
        time_t uiUsedTime;
        UINT uiRetCode;
        USHORT usCDRCount;
        st_CDRInfo stCDRInfo[21];
} st_SndDSCP, *pst_SndDSCP;

typedef struct _st_RcvDSCP
{

        char szIMSI[17];
        INT64 llCallID;
        UCHAR ucMode;
        UINT uiSeqID;
        UINT uiTPeriod;
        UINT uiPPeriod;
        UINT uiRetCode;
} st_RcvDSCP, *pst_RcvDSCP;


typedef struct _st_IPAFUHeader
{
    INT64 llTID;
        INT64 llIndex;
        char szMin[17];
        UCHAR szReserved[7];
        INT64 llAcctSessID;
        UINT uiSIP;
        UCHAR ucUserType;
        UCHAR ucIPAFID;
    USHORT usResult;
        USHORT usTotlLen;
        USHORT usBodyLen;
    USHORT usExtLen;
        UCHAR ucReserved[2];
} st_IPAMUHeader, *pst_IPAMUHeader;

#define IPAMU_HEADER_LEN sizeof(st_IPAMUHeader)
#define DEF_UHEADER_LEN sizeof(st_IPAMUHeader)

#define MAGIC_NUMBER 0x3812121281282828L
typedef struct _st_IPAFTHeader
{
        INT64 llMagicNumber;
        INT64 llIndex;
        USHORT usResult;
        USHORT usSerial;
        UCHAR ucIPAFID;
        UCHAR szReserved[3];
        USHORT usTotlLen;
        USHORT usBodyLen;
        USHORT usExtLen;
        UCHAR ucSvcID;
        UCHAR ucMsgID;
}st_IPAFTHeader, *pst_IPAFTHeader;

#define IPAFT_HEADER_LEN sizeof(st_IPAFTHeader)
typedef struct _st_TID
{
    UCHAR ucMsgID;
    UCHAR ucSvcID;
    USHORT usSerial;
    time_t stBuild;
} st_TID, *pst_TID;

typedef union _un_TID
{
        INT64 llTID;
        st_TID stTID;
} un_TID, *pun_TID;

typedef struct _st_Ack
{
        USHORT usRetCode;
} st_Ack, *pst_Ack;

typedef struct _st_CDRAck
{
        USHORT usRetCode;
        UCHAR ucPayType;
} st_CDRAck, *pst_CDRAck;

typedef struct _st_IPPool
{
        int iSysType;
        int iSysID;
        char szSysIP[16];
        int iPrefix;
} st_IPPool, *pst_IPPool;

typedef struct _st_CategoryInfo
{
        USHORT usCategory;
        UCHAR ucGroup;
        UCHAR ucLayer;
        UCHAR szServiceID[3];
    UCHAR ucMode;
    UCHAR ucSvcBlk;
        UCHAR ucFilterOut;
        UCHAR ucConCount;
        UCHAR ucConSize;
        UCHAR szCon[100][64];
        char szReserved[6];
} st_CategoryInfo, *pst_CategoryInfo;

#define DEF_IPADDR_SIZE 16

typedef struct _st_IPPOOL
{
    UCHAR ucSystemType;
    UCHAR ucSystemID;
    char szIP[16];
    UCHAR ucNetmask;
    char szReserved[5];
}st_IPPOOL, *pst_IPPOOL;

typedef struct _st_SerCat
{
    UCHAR ucCategory;
    UCHAR ucGroup;
    UCHAR ucLayer;
    UCHAR ucFilterOut;
    int dServiceID;
        UCHAR ucMode;
        UCHAR ucSvcBlk;
    UCHAR ucConCount;
    UCHAR ucConSize;
    UCHAR szContent[53];
    UCHAR ucStatus;
        char szAlias[24+6];
}st_SerCat, *pst_SerCat;

#define MAX_SERVICE_CATEGORY_COUNT 200

typedef struct _st_SerCatTot {
        int dCount;
        int dResev;
        st_SerCat stSerCat[200];
} st_SerCatTot, *pst_SerCatTot;

typedef struct _st_AliveReq
{
        UINT uiIdleTime;
} st_AliveReq, *pst_AliveReq;

typedef struct _st_PeriodReq
{
        UCHAR ucAmountType;
        UINT uiPacketPeriod;
        UINT uiTimePeriod;
} st_PeriodReq, *pst_PeriodReq;



typedef struct _st_PatchName
{
        char szPatchName[32];
} st_PatchName, *pst_PatchName;

#define DEF_PATCHNAME_SIZE sizeof(st_PatchName)


typedef struct _st_PatchType
{
        UINT uiPatchType;
}st_PatchType, *pst_PatchType;

#define DEF_PATCHTYPE_SIZE sizeof(st_PatchType)

#define PATCH_TYPE_UNKNOWN 0
#define PATCH_TYPE_REBOOT 1
#define PATCH_TYPE_RESET_PS_SHM 2
#define PATCH_TYPE_RESET_PS 3

#define PATCH_TYPE_EACH_PS 4
#define MAX_PROCESS_NUM 100



#define MAX_BSMSC_SIZE 13
typedef struct _st_TCP_DATA
{
    UINT uiTotUpByte;
    UINT uiTotDwByte;

    UINT uiTotTCPUpByte;
    UINT uiTotTCPDwByte;

    UINT uiTotTCPReUpByte;
    UINT uiTotTCPReDwByte;

    UINT uiTRANUpByte;
    UINT uiTRANDwByte;

    UINT uiDROPUpByte;
    UINT uiDROPDwByte;

    UINT uiREALUpByte;
    UINT uiREALDwByte;

    UINT uiFAILUpByte;
    UINT uiFAILDwByte;

    UINT uiTRANCnt;
    UINT uiReserved;
}st_TCP_DATA, *pst_TCP_DATA;

#define DEF_TCPDATA_SIZE sizeof(st_TCP_DATA)


typedef struct _st_Radius_
{
    UCHAR Code;
    UCHAR Identifier;
    UCHAR Length[2];
    UCHAR Authenticator[16];
    UCHAR Attributes[1];
} st_Radius, *pst_Radius;

#define MAX_BSMSC_SIZE 13

typedef struct _st_AccReq_
{
        UCHAR ucCode;
        UCHAR ucID;
        UCHAR szAuth[17];
        UCHAR szReqserved1[5];

        UINT uiEventTime;
        UINT uiAccStatus;
        UINT uiFramedIP;
        UINT uiSvcOpt;
        INT64 llAccSessID;
        INT64 llCorrelID;
        UCHAR szCallingID[17];
        UCHAR szBSMSCID[13];
        UCHAR szReserved2[2];
} st_AccReq, *pst_AccReq;

#define DEF_ACCREQ_SIZE sizeof(st_AccReq)

#define DEF_PAYINFO_CNT 500

typedef struct _st_PayInfo_
{
    int dPayType;
    int dAmountType;
    int dPeriod;
    char szMin[17];
} st_PayInfo, *pst_PayInfo;

typedef struct _st_PayInfoList
{
    int dCount;
    st_PayInfo stPayInfo[500];
} st_PayInfoList, *pst_PayInfoList;

typedef struct _st_UAFetchTerm
{
    unsigned int uiFetchTerm;
} st_UAFetchTerm, *pst_UAFetchTerm;


typedef struct _st_SESSClean
{
        UCHAR szMIN[17];
    INT64 llAcctSessID;
    UINT uiSrcIP;
} st_SESSClean, *pst_SESSClean;
# 9 "shmmb_print.c" 2
# 1 "../../include/mmdb_destip.h" 1
# 19 "../../include/mmdb_destip.h"
#define __MM_DESTIP_DB_HEADER___ 




# 1 "/usr/include/stdlib.h" 1 3
# 28 "/usr/include/stdlib.h" 3
#define __need_size_t 

#define __need_wchar_t 
#define __need_NULL 

# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 344 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_wchar_t
# 397 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 34 "/usr/include/stdlib.h" 2 3




#define _STDLIB_H 1
# 93 "/usr/include/stdlib.h" 3


typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;
#define __ldiv_t_defined 1


# 126 "/usr/include/stdlib.h" 3
#define RAND_MAX 2147483647




#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0



#define MB_CUR_MAX (__ctype_get_mb_cur_max ())
extern size_t __ctype_get_mb_cur_max (void) ;




extern double atof (__const char *__nptr) __attribute__ ((__pure__));

extern int atoi (__const char *__nptr) __attribute__ ((__pure__));

extern long int atol (__const char *__nptr) __attribute__ ((__pure__));





__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__pure__));





extern double strtod (__const char *__restrict __nptr,
                      char **__restrict __endptr) ;

# 174 "/usr/include/stdlib.h" 3


extern long int strtol (__const char *__restrict __nptr,
                        char **__restrict __endptr, int __base) ;

extern unsigned long int strtoul (__const char *__restrict __nptr,
                                  char **__restrict __endptr, int __base)
     ;




__extension__
extern long long int strtoq (__const char *__restrict __nptr,
                             char **__restrict __endptr, int __base) ;

__extension__
extern unsigned long long int strtouq (__const char *__restrict __nptr,
                                       char **__restrict __endptr, int __base)
     ;





__extension__
extern long long int strtoll (__const char *__restrict __nptr,
                              char **__restrict __endptr, int __base) ;

__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
                                        char **__restrict __endptr, int __base)
     ;

# 264 "/usr/include/stdlib.h" 3
extern double __strtod_internal (__const char *__restrict __nptr,
                                 char **__restrict __endptr, int __group)
     ;
extern float __strtof_internal (__const char *__restrict __nptr,
                                char **__restrict __endptr, int __group)
     ;
extern long double __strtold_internal (__const char *__restrict __nptr,
                                       char **__restrict __endptr,
                                       int __group) ;

extern long int __strtol_internal (__const char *__restrict __nptr,
                                   char **__restrict __endptr,
                                   int __base, int __group) ;
#define __strtol_internal_defined 1


extern unsigned long int __strtoul_internal (__const char *__restrict __nptr,
                                             char **__restrict __endptr,
                                             int __base, int __group) ;
#define __strtoul_internal_defined 1



__extension__
extern long long int __strtoll_internal (__const char *__restrict __nptr,
                                         char **__restrict __endptr,
                                         int __base, int __group) ;
#define __strtoll_internal_defined 1


__extension__
extern unsigned long long int __strtoull_internal (__const char *
                                                   __restrict __nptr,
                                                   char **__restrict __endptr,
                                                   int __base, int __group)
     ;
#define __strtoull_internal_defined 1







extern __inline double
strtod (__const char *__restrict __nptr, char **__restrict __endptr)
{
  return __strtod_internal (__nptr, __endptr, 0);
}
extern __inline long int
strtol (__const char *__restrict __nptr, char **__restrict __endptr,
        int __base)
{
  return __strtol_internal (__nptr, __endptr, __base, 0);
}
extern __inline unsigned long int
strtoul (__const char *__restrict __nptr, char **__restrict __endptr,
         int __base)
{
  return __strtoul_internal (__nptr, __endptr, __base, 0);
}

# 343 "/usr/include/stdlib.h" 3
__extension__ extern __inline long long int
strtoq (__const char *__restrict __nptr, char **__restrict __endptr,
        int __base)
{
  return __strtoll_internal (__nptr, __endptr, __base, 0);
}
__extension__ extern __inline unsigned long long int
strtouq (__const char *__restrict __nptr, char **__restrict __endptr,
         int __base)
{
  return __strtoull_internal (__nptr, __endptr, __base, 0);
}




__extension__ extern __inline long long int
strtoll (__const char *__restrict __nptr, char **__restrict __endptr,
         int __base)
{
  return __strtoll_internal (__nptr, __endptr, __base, 0);
}
__extension__ extern __inline unsigned long long int
strtoull (__const char * __restrict __nptr, char **__restrict __endptr,
          int __base)
{
  return __strtoull_internal (__nptr, __endptr, __base, 0);
}




extern __inline double
atof (__const char *__nptr)
{
  return strtod (__nptr, (char **) ((void *)0));
}
extern __inline int
atoi (__const char *__nptr)
{
  return (int) strtol (__nptr, (char **) ((void *)0), 10);
}
extern __inline long int
atol (__const char *__nptr)
{
  return strtol (__nptr, (char **) ((void *)0), 10);
}




__extension__ extern __inline long long int
atoll (__const char *__nptr)
{
  return strtoll (__nptr, (char **) ((void *)0), 10);
}

# 408 "/usr/include/stdlib.h" 3
extern char *l64a (long int __n) ;


extern long int a64l (__const char *__s) __attribute__ ((__pure__));
# 423 "/usr/include/stdlib.h" 3
extern long int random (void) ;


extern void srandom (unsigned int __seed) ;





extern char *initstate (unsigned int __seed, char *__statebuf,
                        size_t __statelen) ;



extern char *setstate (char *__statebuf) ;







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
                     int32_t *__restrict __result) ;

extern int srandom_r (unsigned int __seed, struct random_data *__buf) ;

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
                        size_t __statelen,
                        struct random_data *__restrict __buf) ;

extern int setstate_r (char *__restrict __statebuf,
                       struct random_data *__restrict __buf) ;






extern int rand (void) ;

extern void srand (unsigned int __seed) ;




extern int rand_r (unsigned int *__seed) ;







extern double drand48 (void) ;
extern double erand48 (unsigned short int __xsubi[3]) ;


extern long int lrand48 (void) ;
extern long int nrand48 (unsigned short int __xsubi[3]) ;


extern long int mrand48 (void) ;
extern long int jrand48 (unsigned short int __xsubi[3]) ;


extern void srand48 (long int __seedval) ;
extern unsigned short int *seed48 (unsigned short int __seed16v[3]) ;
extern void lcong48 (unsigned short int __param[7]) ;





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
                      double *__restrict __result) ;
extern int erand48_r (unsigned short int __xsubi[3],
                      struct drand48_data *__restrict __buffer,
                      double *__restrict __result) ;


extern int lrand48_r (struct drand48_data *__restrict __buffer,
                      long int *__restrict __result) ;
extern int nrand48_r (unsigned short int __xsubi[3],
                      struct drand48_data *__restrict __buffer,
                      long int *__restrict __result) ;


extern int mrand48_r (struct drand48_data *__restrict __buffer,
                      long int *__restrict __result) ;
extern int jrand48_r (unsigned short int __xsubi[3],
                      struct drand48_data *__restrict __buffer,
                      long int *__restrict __result) ;


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     ;

extern int seed48_r (unsigned short int __seed16v[3],
                     struct drand48_data *__buffer) ;

extern int lcong48_r (unsigned short int __param[7],
                      struct drand48_data *__buffer) ;
# 564 "/usr/include/stdlib.h" 3



extern void *realloc (void *__ptr, size_t __size) __attribute__ ((__malloc__));

extern void free (void *__ptr) ;




extern void cfree (void *__ptr) ;



# 1 "/usr/include/alloca.h" 1 3
# 20 "/usr/include/alloca.h" 3
#define _ALLOCA_H 1



#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 26 "/usr/include/alloca.h" 2 3







extern void *alloca (size_t __size) ;


#define alloca(size) __builtin_alloca (size)



# 579 "/usr/include/stdlib.h" 2 3




extern void *valloc (size_t __size) __attribute__ ((__malloc__));
# 592 "/usr/include/stdlib.h" 3


extern void abort (void) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) ;





extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     ;






extern void exit (int __status) __attribute__ ((__noreturn__));

# 624 "/usr/include/stdlib.h" 3


extern char *getenv (__const char *__name) ;




extern char *__secure_getenv (__const char *__name) ;





extern int putenv (char *__string) ;





extern int setenv (__const char *__name, __const char *__value, int __replace)
     ;


extern int unsetenv (__const char *__name) ;






extern int clearenv (void) ;
# 663 "/usr/include/stdlib.h" 3
extern char *mktemp (char *__template) ;
# 674 "/usr/include/stdlib.h" 3
extern int mkstemp (char *__template);
# 693 "/usr/include/stdlib.h" 3
extern char *mkdtemp (char *__template) ;








extern int system (__const char *__command);

# 720 "/usr/include/stdlib.h" 3
extern char *realpath (__const char *__restrict __name,
                       char *__restrict __resolved) ;





#define __COMPAR_FN_T 
typedef int (*__compar_fn_t) (__const void *, __const void *);









extern void *bsearch (__const void *__key, __const void *__base,
                      size_t __nmemb, size_t __size, __compar_fn_t __compar);



extern void qsort (void *__base, size_t __nmemb, size_t __size,
                   __compar_fn_t __compar);



extern int abs (int __x) __attribute__ ((__const__));
extern long int labs (long int __x) __attribute__ ((__const__));












extern div_t div (int __numer, int __denom)
     __attribute__ ((__const__));
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__const__));

# 784 "/usr/include/stdlib.h" 3
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign) ;




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign) ;




extern char *gcvt (double __value, int __ndigit, char *__buf) ;




extern char *qecvt (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign) ;
extern char *qfcvt (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign) ;
extern char *qgcvt (long double __value, int __ndigit, char *__buf) ;




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign, char *__restrict __buf,
                   size_t __len) ;
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
                   int *__restrict __sign, char *__restrict __buf,
                   size_t __len) ;

extern int qecvt_r (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign,
                    char *__restrict __buf, size_t __len) ;
extern int qfcvt_r (long double __value, int __ndigit,
                    int *__restrict __decpt, int *__restrict __sign,
                    char *__restrict __buf, size_t __len) ;







extern int mblen (__const char *__s, size_t __n) ;


extern int mbtowc (wchar_t *__restrict __pwc,
                   __const char *__restrict __s, size_t __n) ;


extern int wctomb (char *__s, wchar_t __wchar) ;



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
                        __const char *__restrict __s, size_t __n) ;

extern size_t wcstombs (char *__restrict __s,
                        __const wchar_t *__restrict __pwcs, size_t __n)
     ;








extern int rpmatch (__const char *__response) ;
# 916 "/usr/include/stdlib.h" 3
extern int getloadavg (double __loadavg[], int __nelem) ;






# 25 "../../include/mmdb_destip.h" 2

# 1 "/usr/include/sys/param.h" 1 3
# 20 "/usr/include/sys/param.h" 3
#define _SYS_PARAM_H 1

# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/limits.h" 1 3
# 23 "/usr/include/sys/param.h" 2 3

# 1 "/usr/include/linux/param.h" 1 3

#define _LINUX_PARAM_H 

# 1 "/usr/include/asm/param.h" 1 3

#define _ASMi386_PARAM_H 


#define HZ 100


#define EXEC_PAGESIZE 4096


#define NGROUPS 32



#define NOGROUP (-1)


#define MAXHOSTNAMELEN 64
# 5 "/usr/include/linux/param.h" 2 3
# 25 "/usr/include/sys/param.h" 2 3



#define NBBY CHAR_BIT



#define MAXSYMLINKS 20
#define CANBSIZ MAX_CANON
#define NCARGS ARG_MAX
#define MAXPATHLEN PATH_MAX



#define NOFILE 256





#define setbit(a,i) ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
#define clrbit(a,i) ((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))
#define isset(a,i) ((a)[(i)/NBBY] & (1<<((i)%NBBY)))
#define isclr(a,i) (((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)



#define howmany(x,y) (((x) + ((y) - 1)) / (y))


#define roundup(x,y) (__builtin_constant_p (y) && powerof2 (y) ? (((x) + (y) - 1) & ~((y) - 1)) : ((((x) + ((y) - 1)) / (y)) * (y)))





#define powerof2(x) ((((x) - 1) & (x)) == 0)


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))



#define DEV_BSIZE 512
# 27 "../../include/mmdb_destip.h" 2




# 1 "/usr/include/netdb.h" 1 3
# 24 "/usr/include/netdb.h" 3
#define _NETDB_H 1
# 33 "/usr/include/netdb.h" 3
# 1 "/usr/include/rpc/netdb.h" 1 3
# 37 "/usr/include/rpc/netdb.h" 3
#define _RPC_NETDB_H 1



#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 43 "/usr/include/rpc/netdb.h" 2 3



struct rpcent
{
  char *r_name;
  char **r_aliases;
  int r_number;
};

extern void setrpcent (int __stayopen) ;
extern void endrpcent (void) ;
extern struct rpcent *getrpcbyname (__const char *__name) ;
extern struct rpcent *getrpcbynumber (int __number) ;
extern struct rpcent *getrpcent (void) ;


extern int getrpcbyname_r (__const char *__name, struct rpcent *__result_buf,
                           char *__buffer, size_t __buflen,
                           struct rpcent **__result) ;

extern int getrpcbynumber_r (int __number, struct rpcent *__result_buf,
                             char *__buffer, size_t __buflen,
                             struct rpcent **__result) ;

extern int getrpcent_r (struct rpcent *__result_buf, char *__buffer,
                        size_t __buflen, struct rpcent **__result) ;



# 34 "/usr/include/netdb.h" 2 3
# 43 "/usr/include/netdb.h" 3
# 1 "/usr/include/bits/netdb.h" 1 3
# 27 "/usr/include/bits/netdb.h" 3
struct netent
{
  char *n_name;
  char **n_aliases;
  int n_addrtype;
  uint32_t n_net;
};
# 44 "/usr/include/netdb.h" 2 3


#define _PATH_HEQUIV "/etc/hosts.equiv"
#define _PATH_HOSTS "/etc/hosts"
#define _PATH_NETWORKS "/etc/networks"
#define _PATH_NSSWITCH_CONF "/etc/nsswitch.conf"
#define _PATH_PROTOCOLS "/etc/protocols"
#define _PATH_SERVICES "/etc/services"






#define h_errno (*__h_errno_location ())


extern int *__h_errno_location (void) __attribute__ ((__const__));



#define NETDB_INTERNAL -1
#define NETDB_SUCCESS 0
#define HOST_NOT_FOUND 1
#define TRY_AGAIN 2

#define NO_RECOVERY 3

#define NO_DATA 4

#define NO_ADDRESS NO_DATA
# 88 "/usr/include/netdb.h" 3
extern void herror (__const char *__str) ;


extern __const char *hstrerror (int __err_num) ;




struct hostent
{
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
#define h_addr h_addr_list[0]
};






extern void sethostent (int __stay_open);





extern void endhostent (void);






extern struct hostent *gethostent (void);






extern struct hostent *gethostbyaddr (__const void *__addr, __socklen_t __len,
                                      int __type);





extern struct hostent *gethostbyname (__const char *__name);
# 149 "/usr/include/netdb.h" 3
extern struct hostent *gethostbyname2 (__const char *__name, int __af);
# 161 "/usr/include/netdb.h" 3
extern int gethostent_r (struct hostent *__restrict __result_buf,
                         char *__restrict __buf, size_t __buflen,
                         struct hostent **__restrict __result,
                         int *__restrict __h_errnop);

extern int gethostbyaddr_r (__const void *__restrict __addr, __socklen_t __len,
                            int __type,
                            struct hostent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct hostent **__restrict __result,
                            int *__restrict __h_errnop);

extern int gethostbyname_r (__const char *__restrict __name,
                            struct hostent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct hostent **__restrict __result,
                            int *__restrict __h_errnop);

extern int gethostbyname2_r (__const char *__restrict __name, int __af,
                             struct hostent *__restrict __result_buf,
                             char *__restrict __buf, size_t __buflen,
                             struct hostent **__restrict __result,
                             int *__restrict __h_errnop);
# 192 "/usr/include/netdb.h" 3
extern void setnetent (int __stay_open);





extern void endnetent (void);






extern struct netent *getnetent (void);






extern struct netent *getnetbyaddr (uint32_t __net, int __type);





extern struct netent *getnetbyname (__const char *__name);
# 231 "/usr/include/netdb.h" 3
extern int getnetent_r (struct netent *__restrict __result_buf,
                        char *__restrict __buf, size_t __buflen,
                        struct netent **__restrict __result,
                        int *__restrict __h_errnop);

extern int getnetbyaddr_r (uint32_t __net, int __type,
                           struct netent *__restrict __result_buf,
                           char *__restrict __buf, size_t __buflen,
                           struct netent **__restrict __result,
                           int *__restrict __h_errnop);

extern int getnetbyname_r (__const char *__restrict __name,
                           struct netent *__restrict __result_buf,
                           char *__restrict __buf, size_t __buflen,
                           struct netent **__restrict __result,
                           int *__restrict __h_errnop);




struct servent
{
  char *s_name;
  char **s_aliases;
  int s_port;
  char *s_proto;
};






extern void setservent (int __stay_open);





extern void endservent (void);






extern struct servent *getservent (void);






extern struct servent *getservbyname (__const char *__name,
                                      __const char *__proto);






extern struct servent *getservbyport (int __port, __const char *__proto);
# 303 "/usr/include/netdb.h" 3
extern int getservent_r (struct servent *__restrict __result_buf,
                         char *__restrict __buf, size_t __buflen,
                         struct servent **__restrict __result);

extern int getservbyname_r (__const char *__restrict __name,
                            __const char *__restrict __proto,
                            struct servent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct servent **__restrict __result);

extern int getservbyport_r (int __port, __const char *__restrict __proto,
                            struct servent *__restrict __result_buf,
                            char *__restrict __buf, size_t __buflen,
                            struct servent **__restrict __result);




struct protoent
{
  char *p_name;
  char **p_aliases;
  int p_proto;
};






extern void setprotoent (int __stay_open);





extern void endprotoent (void);






extern struct protoent *getprotoent (void);





extern struct protoent *getprotobyname (__const char *__name);





extern struct protoent *getprotobynumber (int __proto);
# 369 "/usr/include/netdb.h" 3
extern int getprotoent_r (struct protoent *__restrict __result_buf,
                          char *__restrict __buf, size_t __buflen,
                          struct protoent **__restrict __result);

extern int getprotobyname_r (__const char *__restrict __name,
                             struct protoent *__restrict __result_buf,
                             char *__restrict __buf, size_t __buflen,
                             struct protoent **__restrict __result);

extern int getprotobynumber_r (int __proto,
                               struct protoent *__restrict __result_buf,
                               char *__restrict __buf, size_t __buflen,
                               struct protoent **__restrict __result);
# 390 "/usr/include/netdb.h" 3
extern int setnetgrent (__const char *__netgroup);







extern void endnetgrent (void);
# 407 "/usr/include/netdb.h" 3
extern int getnetgrent (char **__restrict __hostp,
                        char **__restrict __userp,
                        char **__restrict __domainp);
# 418 "/usr/include/netdb.h" 3
extern int innetgr (__const char *__netgroup, __const char *__host,
                    __const char *__user, __const char *domain);







extern int getnetgrent_r (char **__restrict __hostp,
                          char **__restrict __userp,
                          char **__restrict __domainp,
                          char *__restrict __buffer, size_t __buflen);
# 446 "/usr/include/netdb.h" 3
extern int rcmd (char **__restrict __ahost, unsigned short int __rport,
                 __const char *__restrict __locuser,
                 __const char *__restrict __remuser,
                 __const char *__restrict __cmd, int *__restrict __fd2p);
# 458 "/usr/include/netdb.h" 3
extern int rcmd_af (char **__restrict __ahost, unsigned short int __rport,
                    __const char *__restrict __locuser,
                    __const char *__restrict __remuser,
                    __const char *__restrict __cmd, int *__restrict __fd2p,
                    sa_family_t __af);
# 474 "/usr/include/netdb.h" 3
extern int rexec (char **__restrict __ahost, int __rport,
                  __const char *__restrict __name,
                  __const char *__restrict __pass,
                  __const char *__restrict __cmd, int *__restrict __fd2p);
# 486 "/usr/include/netdb.h" 3
extern int rexec_af (char **__restrict __ahost, int __rport,
                     __const char *__restrict __name,
                     __const char *__restrict __pass,
                     __const char *__restrict __cmd, int *__restrict __fd2p,
                     sa_family_t __af);
# 500 "/usr/include/netdb.h" 3
extern int ruserok (__const char *__rhost, int __suser,
                    __const char *__remuser, __const char *__locuser);
# 510 "/usr/include/netdb.h" 3
extern int ruserok_af (__const char *__rhost, int __suser,
                       __const char *__remuser, __const char *__locuser,
                       sa_family_t __af);
# 522 "/usr/include/netdb.h" 3
extern int rresvport (int *__alport);
# 531 "/usr/include/netdb.h" 3
extern int rresvport_af (int *__alport, sa_family_t __af);






struct addrinfo
{
  int ai_flags;
  int ai_family;
  int ai_socktype;
  int ai_protocol;
  socklen_t ai_addrlen;
  struct sockaddr *ai_addr;
  char *ai_canonname;
  struct addrinfo *ai_next;
};
# 569 "/usr/include/netdb.h" 3
#define AI_PASSIVE 0x0001
#define AI_CANONNAME 0x0002
#define AI_NUMERICHOST 0x0004
#define AI_V4MAPPED 0x0008
#define AI_ALL 0x0010
#define AI_ADDRCONFIG 0x0020



#define EAI_BADFLAGS -1
#define EAI_NONAME -2
#define EAI_AGAIN -3
#define EAI_FAIL -4
#define EAI_NODATA -5
#define EAI_FAMILY -6
#define EAI_SOCKTYPE -7
#define EAI_SERVICE -8
#define EAI_ADDRFAMILY -9
#define EAI_MEMORY -10
#define EAI_SYSTEM -11
# 597 "/usr/include/netdb.h" 3
#define NI_MAXHOST 1025
#define NI_MAXSERV 32

#define NI_NUMERICHOST 1
#define NI_NUMERICSERV 2
#define NI_NOFQDN 4
#define NI_NAMEREQD 8
#define NI_DGRAM 16






extern int getaddrinfo (__const char *__restrict __name,
                        __const char *__restrict __service,
                        __const struct addrinfo *__restrict __req,
                        struct addrinfo **__restrict __pai);


extern void freeaddrinfo (struct addrinfo *__ai) ;


extern __const char *gai_strerror (int __ecode) ;





extern int getnameinfo (__const struct sockaddr *__restrict __sa,
                        socklen_t __salen, char *__restrict __host,
                        socklen_t __hostlen, char *__restrict __serv,
                        socklen_t __servlen, unsigned int __flags);
# 662 "/usr/include/netdb.h" 3

# 32 "../../include/mmdb_destip.h" 2
# 1 "/usr/include/errno.h" 1 3
# 28 "/usr/include/errno.h" 3
#define _ERRNO_H 1







# 1 "/usr/include/bits/errno.h" 1 3
# 25 "/usr/include/bits/errno.h" 3
# 1 "/usr/include/linux/errno.h" 1 3

#define _LINUX_ERRNO_H 

# 1 "/usr/include/asm/errno.h" 1 3

#define _I386_ERRNO_H 

#define EPERM 1
#define ENOENT 2
#define ESRCH 3
#define EINTR 4
#define EIO 5
#define ENXIO 6
#define E2BIG 7
#define ENOEXEC 8
#define EBADF 9
#define ECHILD 10
#define EAGAIN 11
#define ENOMEM 12
#define EACCES 13
#define EFAULT 14
#define ENOTBLK 15
#define EBUSY 16
#define EEXIST 17
#define EXDEV 18
#define ENODEV 19
#define ENOTDIR 20
#define EISDIR 21
#define EINVAL 22
#define ENFILE 23
#define EMFILE 24
#define ENOTTY 25
#define ETXTBSY 26
#define EFBIG 27
#define ENOSPC 28
#define ESPIPE 29
#define EROFS 30
#define EMLINK 31
#define EPIPE 32
#define EDOM 33
#define ERANGE 34
#define EDEADLK 35
#define ENAMETOOLONG 36
#define ENOLCK 37
#define ENOSYS 38
#define ENOTEMPTY 39
#define ELOOP 40
#define EWOULDBLOCK EAGAIN
#define ENOMSG 42
#define EIDRM 43
#define ECHRNG 44
#define EL2NSYNC 45
#define EL3HLT 46
#define EL3RST 47
#define ELNRNG 48
#define EUNATCH 49
#define ENOCSI 50
#define EL2HLT 51
#define EBADE 52
#define EBADR 53
#define EXFULL 54
#define ENOANO 55
#define EBADRQC 56
#define EBADSLT 57

#define EDEADLOCK EDEADLK

#define EBFONT 59
#define ENOSTR 60
#define ENODATA 61
#define ETIME 62
#define ENOSR 63
#define ENONET 64
#define ENOPKG 65
#define EREMOTE 66
#define ENOLINK 67
#define EADV 68
#define ESRMNT 69
#define ECOMM 70
#define EPROTO 71
#define EMULTIHOP 72
#define EDOTDOT 73
#define EBADMSG 74
#define EOVERFLOW 75
#define ENOTUNIQ 76
#define EBADFD 77
#define EREMCHG 78
#define ELIBACC 79
#define ELIBBAD 80
#define ELIBSCN 81
#define ELIBMAX 82
#define ELIBEXEC 83
#define EILSEQ 84
#define ERESTART 85
#define ESTRPIPE 86
#define EUSERS 87
#define ENOTSOCK 88
#define EDESTADDRREQ 89
#define EMSGSIZE 90
#define EPROTOTYPE 91
#define ENOPROTOOPT 92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP 95
#define EPFNOSUPPORT 96
#define EAFNOSUPPORT 97
#define EADDRINUSE 98
#define EADDRNOTAVAIL 99
#define ENETDOWN 100
#define ENETUNREACH 101
#define ENETRESET 102
#define ECONNABORTED 103
#define ECONNRESET 104
#define ENOBUFS 105
#define EISCONN 106
#define ENOTCONN 107
#define ESHUTDOWN 108
#define ETOOMANYREFS 109
#define ETIMEDOUT 110
#define ECONNREFUSED 111
#define EHOSTDOWN 112
#define EHOSTUNREACH 113
#define EALREADY 114
#define EINPROGRESS 115
#define ESTALE 116
#define EUCLEAN 117
#define ENOTNAM 118
#define ENAVAIL 119
#define EISNAM 120
#define EREMOTEIO 121
#define EDQUOT 122

#define ENOMEDIUM 123
#define EMEDIUMTYPE 124
# 5 "/usr/include/linux/errno.h" 2 3
# 26 "/usr/include/bits/errno.h" 2 3


#define ENOTSUP EOPNOTSUPP




#define ECANCELED 125




extern int *__errno_location (void) __attribute__ ((__const__));



#define errno (*__errno_location ())
# 37 "/usr/include/errno.h" 2 3
# 59 "/usr/include/errno.h" 3

# 33 "../../include/mmdb_destip.h" 2
# 1 "/usr/include/fcntl.h" 1 3
# 24 "/usr/include/fcntl.h" 3
#define _FCNTL_H 1








# 1 "/usr/include/bits/fcntl.h" 1 3
# 29 "/usr/include/bits/fcntl.h" 3
#define O_ACCMODE 0003
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100
#define O_EXCL 0200
#define O_NOCTTY 0400
#define O_TRUNC 01000
#define O_APPEND 02000
#define O_NONBLOCK 04000
#define O_NDELAY O_NONBLOCK
#define O_SYNC 010000
#define O_FSYNC O_SYNC
#define O_ASYNC 020000
# 54 "/usr/include/bits/fcntl.h" 3
#define O_DSYNC O_SYNC
#define O_RSYNC O_SYNC







#define F_DUPFD 0
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4

#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7





#define F_GETLK64 12
#define F_SETLK64 13
#define F_SETLKW64 14


#define F_SETOWN 8
#define F_GETOWN 9
# 98 "/usr/include/bits/fcntl.h" 3
#define FD_CLOEXEC 1


#define F_RDLCK 0
#define F_WRLCK 1
#define F_UNLCK 2


#define F_EXLCK 4
#define F_SHLCK 8



#define LOCK_SH 1
#define LOCK_EX 2
#define LOCK_NB 4

#define LOCK_UN 8
# 136 "/usr/include/bits/fcntl.h" 3
struct flock
  {
    short int l_type;
    short int l_whence;

    __off_t l_start;
    __off_t l_len;




    __pid_t l_pid;
  };
# 164 "/usr/include/bits/fcntl.h" 3
#define FAPPEND O_APPEND
#define FFSYNC O_FSYNC
#define FASYNC O_ASYNC
#define FNONBLOCK O_NONBLOCK
#define FNDELAY O_NDELAY
# 181 "/usr/include/bits/fcntl.h" 3



extern ssize_t readahead (int __fd, __off64_t __offset, size_t __count)
    ;


# 34 "/usr/include/fcntl.h" 2 3
# 44 "/usr/include/fcntl.h" 3
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
# 63 "/usr/include/fcntl.h" 3
extern int fcntl (int __fd, int __cmd, ...);
# 72 "/usr/include/fcntl.h" 3
extern int open (__const char *__file, int __oflag, ...);
# 90 "/usr/include/fcntl.h" 3
extern int creat (__const char *__file, __mode_t __mode);
# 113 "/usr/include/fcntl.h" 3
#define F_ULOCK 0
#define F_LOCK 1
#define F_TLOCK 2
#define F_TEST 3


extern int lockf (int __fd, int __cmd, __off_t __len);
# 173 "/usr/include/fcntl.h" 3

# 34 "../../include/mmdb_destip.h" 2
# 1 "/usr/include/unistd.h" 1 3
# 24 "/usr/include/unistd.h" 3
#define _UNISTD_H 1









#define _POSIX_VERSION 200112L





#define _POSIX2_VERSION 200112L



#define _POSIX2_C_BIND 200112L



#define _POSIX2_C_DEV 200112L



#define _POSIX2_SW_DEV 200112L



#define _POSIX2_LOCALEDEF 200112L





#define _XOPEN_VERSION 4



#define _XOPEN_XCU_VERSION 4


#define _XOPEN_XPG2 1
#define _XOPEN_XPG3 1
#define _XOPEN_XPG4 1


#define _XOPEN_UNIX 1


#define _XOPEN_CRYPT 1



#define _XOPEN_ENH_I18N 1


#define _XOPEN_LEGACY 1
# 171 "/usr/include/unistd.h" 3
# 1 "/usr/include/bits/posix_opt.h" 1 3
# 21 "/usr/include/bits/posix_opt.h" 3
#define _POSIX_OPT_H 1


#define _POSIX_JOB_CONTROL 1


#define _POSIX_SAVED_IDS 1


#define _POSIX_PRIORITY_SCHEDULING 200112L


#define _POSIX_SYNCHRONIZED_IO 200112L


#define _POSIX_FSYNC 200112L


#define _POSIX_MAPPED_FILES 200112L


#define _POSIX_MEMLOCK 200112L


#define _POSIX_MEMLOCK_RANGE 200112L


#define _POSIX_MEMORY_PROTECTION 200112L


#define _POSIX_CHOWN_RESTRICTED 1



#define _POSIX_VDISABLE '\0'


#define _POSIX_NO_TRUNC 1


#define _XOPEN_REALTIME 1


#define _XOPEN_REALTIME_THREADS 1


#define _XOPEN_SHM 1


#define _POSIX_THREADS 200112L


#define _POSIX_REENTRANT_FUNCTIONS 1
#define _POSIX_THREAD_SAFE_FUNCTIONS 200112L


#define _POSIX_THREAD_PRIORITY_SCHEDULING 200112L


#define _POSIX_THREAD_ATTR_STACKSIZE 200112L


#define _POSIX_THREAD_ATTR_STACKADDR 200112L


#define _POSIX_SEMAPHORES 200112L


#define _POSIX_REALTIME_SIGNALS 200112L


#define _POSIX_ASYNCHRONOUS_IO 200112L
#define _POSIX_ASYNC_IO 1

#define _LFS_ASYNCHRONOUS_IO 1


#define _LFS64_ASYNCHRONOUS_IO 1


#define _LFS_LARGEFILE 1
#define _LFS64_LARGEFILE 1
#define _LFS64_STDIO 1


#define _POSIX_SHARED_MEMORY_OBJECTS 200112L


#define _POSIX_CPUTIME 200112L


#define _POSIX_THREAD_CPUTIME 200112L


#define _POSIX_REGEXP 1


#define _POSIX_READER_WRITER_LOCKS 200112L


#define _POSIX_SHELL 1


#define _POSIX_TIMEOUTS 200112L


#define _POSIX_SPIN_LOCKS 200112L


#define _POSIX_SPAWN 200112L


#define _POSIX_TIMERS 200112L


#define _POSIX_BARRIERS 200112L





#define _POSIX_MONOTONIC_CLOCK 0
# 172 "/usr/include/unistd.h" 2 3







#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
# 193 "/usr/include/unistd.h" 3
#define __need_size_t 
#define __need_NULL 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 397 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 196 "/usr/include/unistd.h" 2 3
# 250 "/usr/include/unistd.h" 3
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0


extern int access (__const char *__name, int __type) ;
# 274 "/usr/include/unistd.h" 3
#define L_SET SEEK_SET
#define L_INCR SEEK_CUR
#define L_XTND SEEK_END
# 286 "/usr/include/unistd.h" 3
extern __off_t lseek (int __fd, __off_t __offset, int __whence) ;
# 305 "/usr/include/unistd.h" 3
extern int close (int __fd);






extern ssize_t read (int __fd, void *__buf, size_t __nbytes);





extern ssize_t write (int __fd, __const void *__buf, size_t __n);
# 369 "/usr/include/unistd.h" 3
extern int pipe (int __pipedes[2]) ;
# 378 "/usr/include/unistd.h" 3
extern unsigned int alarm (unsigned int __seconds) ;
# 390 "/usr/include/unistd.h" 3
extern unsigned int sleep (unsigned int __seconds);






extern __useconds_t ualarm (__useconds_t __value, __useconds_t __interval)
     ;






extern int usleep (__useconds_t __useconds);
# 414 "/usr/include/unistd.h" 3
extern int pause (void);



extern int chown (__const char *__file, __uid_t __owner, __gid_t __group)
     ;



extern int fchown (int __fd, __uid_t __owner, __gid_t __group) ;




extern int lchown (__const char *__file, __uid_t __owner, __gid_t __group)
     ;




extern int chdir (__const char *__path) ;



extern int fchdir (int __fd) ;
# 448 "/usr/include/unistd.h" 3
extern char *getcwd (char *__buf, size_t __size) ;
# 461 "/usr/include/unistd.h" 3
extern char *getwd (char *__buf) ;




extern int dup (int __fd) ;


extern int dup2 (int __fd, int __fd2) ;


extern char **__environ;







extern int execve (__const char *__path, char *__const __argv[],
                   char *__const __envp[]) ;
# 492 "/usr/include/unistd.h" 3
extern int execv (__const char *__path, char *__const __argv[]) ;



extern int execle (__const char *__path, __const char *__arg, ...) ;



extern int execl (__const char *__path, __const char *__arg, ...) ;



extern int execvp (__const char *__file, char *__const __argv[]) ;




extern int execlp (__const char *__file, __const char *__arg, ...) ;




extern int nice (int __inc) ;




extern void _exit (int __status) __attribute__ ((__noreturn__));





# 1 "/usr/include/bits/confname.h" 1 3
# 25 "/usr/include/bits/confname.h" 3
enum
  {
    _PC_LINK_MAX,
#define _PC_LINK_MAX _PC_LINK_MAX
    _PC_MAX_CANON,
#define _PC_MAX_CANON _PC_MAX_CANON
    _PC_MAX_INPUT,
#define _PC_MAX_INPUT _PC_MAX_INPUT
    _PC_NAME_MAX,
#define _PC_NAME_MAX _PC_NAME_MAX
    _PC_PATH_MAX,
#define _PC_PATH_MAX _PC_PATH_MAX
    _PC_PIPE_BUF,
#define _PC_PIPE_BUF _PC_PIPE_BUF
    _PC_CHOWN_RESTRICTED,
#define _PC_CHOWN_RESTRICTED _PC_CHOWN_RESTRICTED
    _PC_NO_TRUNC,
#define _PC_NO_TRUNC _PC_NO_TRUNC
    _PC_VDISABLE,
#define _PC_VDISABLE _PC_VDISABLE
    _PC_SYNC_IO,
#define _PC_SYNC_IO _PC_SYNC_IO
    _PC_ASYNC_IO,
#define _PC_ASYNC_IO _PC_ASYNC_IO
    _PC_PRIO_IO,
#define _PC_PRIO_IO _PC_PRIO_IO
    _PC_SOCK_MAXBUF,
#define _PC_SOCK_MAXBUF _PC_SOCK_MAXBUF
    _PC_FILESIZEBITS,
#define _PC_FILESIZEBITS _PC_FILESIZEBITS
    _PC_REC_INCR_XFER_SIZE,
#define _PC_REC_INCR_XFER_SIZE _PC_REC_INCR_XFER_SIZE
    _PC_REC_MAX_XFER_SIZE,
#define _PC_REC_MAX_XFER_SIZE _PC_REC_MAX_XFER_SIZE
    _PC_REC_MIN_XFER_SIZE,
#define _PC_REC_MIN_XFER_SIZE _PC_REC_MIN_XFER_SIZE
    _PC_REC_XFER_ALIGN,
#define _PC_REC_XFER_ALIGN _PC_REC_XFER_ALIGN
    _PC_ALLOC_SIZE_MIN,
#define _PC_ALLOC_SIZE_MIN _PC_ALLOC_SIZE_MIN
    _PC_SYMLINK_MAX,
#define _PC_SYMLINK_MAX _PC_SYMLINK_MAX
    _PC_2_SYMLINKS
#define _PC_2_SYMLINKS _PC_2_SYMLINKS
  };


enum
  {
    _SC_ARG_MAX,
#define _SC_ARG_MAX _SC_ARG_MAX
    _SC_CHILD_MAX,
#define _SC_CHILD_MAX _SC_CHILD_MAX
    _SC_CLK_TCK,
#define _SC_CLK_TCK _SC_CLK_TCK
    _SC_NGROUPS_MAX,
#define _SC_NGROUPS_MAX _SC_NGROUPS_MAX
    _SC_OPEN_MAX,
#define _SC_OPEN_MAX _SC_OPEN_MAX
    _SC_STREAM_MAX,
#define _SC_STREAM_MAX _SC_STREAM_MAX
    _SC_TZNAME_MAX,
#define _SC_TZNAME_MAX _SC_TZNAME_MAX
    _SC_JOB_CONTROL,
#define _SC_JOB_CONTROL _SC_JOB_CONTROL
    _SC_SAVED_IDS,
#define _SC_SAVED_IDS _SC_SAVED_IDS
    _SC_REALTIME_SIGNALS,
#define _SC_REALTIME_SIGNALS _SC_REALTIME_SIGNALS
    _SC_PRIORITY_SCHEDULING,
#define _SC_PRIORITY_SCHEDULING _SC_PRIORITY_SCHEDULING
    _SC_TIMERS,
#define _SC_TIMERS _SC_TIMERS
    _SC_ASYNCHRONOUS_IO,
#define _SC_ASYNCHRONOUS_IO _SC_ASYNCHRONOUS_IO
    _SC_PRIORITIZED_IO,
#define _SC_PRIORITIZED_IO _SC_PRIORITIZED_IO
    _SC_SYNCHRONIZED_IO,
#define _SC_SYNCHRONIZED_IO _SC_SYNCHRONIZED_IO
    _SC_FSYNC,
#define _SC_FSYNC _SC_FSYNC
    _SC_MAPPED_FILES,
#define _SC_MAPPED_FILES _SC_MAPPED_FILES
    _SC_MEMLOCK,
#define _SC_MEMLOCK _SC_MEMLOCK
    _SC_MEMLOCK_RANGE,
#define _SC_MEMLOCK_RANGE _SC_MEMLOCK_RANGE
    _SC_MEMORY_PROTECTION,
#define _SC_MEMORY_PROTECTION _SC_MEMORY_PROTECTION
    _SC_MESSAGE_PASSING,
#define _SC_MESSAGE_PASSING _SC_MESSAGE_PASSING
    _SC_SEMAPHORES,
#define _SC_SEMAPHORES _SC_SEMAPHORES
    _SC_SHARED_MEMORY_OBJECTS,
#define _SC_SHARED_MEMORY_OBJECTS _SC_SHARED_MEMORY_OBJECTS
    _SC_AIO_LISTIO_MAX,
#define _SC_AIO_LISTIO_MAX _SC_AIO_LISTIO_MAX
    _SC_AIO_MAX,
#define _SC_AIO_MAX _SC_AIO_MAX
    _SC_AIO_PRIO_DELTA_MAX,
#define _SC_AIO_PRIO_DELTA_MAX _SC_AIO_PRIO_DELTA_MAX
    _SC_DELAYTIMER_MAX,
#define _SC_DELAYTIMER_MAX _SC_DELAYTIMER_MAX
    _SC_MQ_OPEN_MAX,
#define _SC_MQ_OPEN_MAX _SC_MQ_OPEN_MAX
    _SC_MQ_PRIO_MAX,
#define _SC_MQ_PRIO_MAX _SC_MQ_PRIO_MAX
    _SC_VERSION,
#define _SC_VERSION _SC_VERSION
    _SC_PAGESIZE,
#define _SC_PAGESIZE _SC_PAGESIZE
#define _SC_PAGE_SIZE _SC_PAGESIZE
    _SC_RTSIG_MAX,
#define _SC_RTSIG_MAX _SC_RTSIG_MAX
    _SC_SEM_NSEMS_MAX,
#define _SC_SEM_NSEMS_MAX _SC_SEM_NSEMS_MAX
    _SC_SEM_VALUE_MAX,
#define _SC_SEM_VALUE_MAX _SC_SEM_VALUE_MAX
    _SC_SIGQUEUE_MAX,
#define _SC_SIGQUEUE_MAX _SC_SIGQUEUE_MAX
    _SC_TIMER_MAX,
#define _SC_TIMER_MAX _SC_TIMER_MAX



    _SC_BC_BASE_MAX,
#define _SC_BC_BASE_MAX _SC_BC_BASE_MAX
    _SC_BC_DIM_MAX,
#define _SC_BC_DIM_MAX _SC_BC_DIM_MAX
    _SC_BC_SCALE_MAX,
#define _SC_BC_SCALE_MAX _SC_BC_SCALE_MAX
    _SC_BC_STRING_MAX,
#define _SC_BC_STRING_MAX _SC_BC_STRING_MAX
    _SC_COLL_WEIGHTS_MAX,
#define _SC_COLL_WEIGHTS_MAX _SC_COLL_WEIGHTS_MAX
    _SC_EQUIV_CLASS_MAX,
#define _SC_EQUIV_CLASS_MAX _SC_EQUIV_CLASS_MAX
    _SC_EXPR_NEST_MAX,
#define _SC_EXPR_NEST_MAX _SC_EXPR_NEST_MAX
    _SC_LINE_MAX,
#define _SC_LINE_MAX _SC_LINE_MAX
    _SC_RE_DUP_MAX,
#define _SC_RE_DUP_MAX _SC_RE_DUP_MAX
    _SC_CHARCLASS_NAME_MAX,
#define _SC_CHARCLASS_NAME_MAX _SC_CHARCLASS_NAME_MAX

    _SC_2_VERSION,
#define _SC_2_VERSION _SC_2_VERSION
    _SC_2_C_BIND,
#define _SC_2_C_BIND _SC_2_C_BIND
    _SC_2_C_DEV,
#define _SC_2_C_DEV _SC_2_C_DEV
    _SC_2_FORT_DEV,
#define _SC_2_FORT_DEV _SC_2_FORT_DEV
    _SC_2_FORT_RUN,
#define _SC_2_FORT_RUN _SC_2_FORT_RUN
    _SC_2_SW_DEV,
#define _SC_2_SW_DEV _SC_2_SW_DEV
    _SC_2_LOCALEDEF,
#define _SC_2_LOCALEDEF _SC_2_LOCALEDEF

    _SC_PII,
#define _SC_PII _SC_PII
    _SC_PII_XTI,
#define _SC_PII_XTI _SC_PII_XTI
    _SC_PII_SOCKET,
#define _SC_PII_SOCKET _SC_PII_SOCKET
    _SC_PII_INTERNET,
#define _SC_PII_INTERNET _SC_PII_INTERNET
    _SC_PII_OSI,
#define _SC_PII_OSI _SC_PII_OSI
    _SC_POLL,
#define _SC_POLL _SC_POLL
    _SC_SELECT,
#define _SC_SELECT _SC_SELECT
    _SC_UIO_MAXIOV,
#define _SC_UIO_MAXIOV _SC_UIO_MAXIOV
    _SC_IOV_MAX = _SC_UIO_MAXIOV,
#define _SC_IOV_MAX _SC_IOV_MAX
    _SC_PII_INTERNET_STREAM,
#define _SC_PII_INTERNET_STREAM _SC_PII_INTERNET_STREAM
    _SC_PII_INTERNET_DGRAM,
#define _SC_PII_INTERNET_DGRAM _SC_PII_INTERNET_DGRAM
    _SC_PII_OSI_COTS,
#define _SC_PII_OSI_COTS _SC_PII_OSI_COTS
    _SC_PII_OSI_CLTS,
#define _SC_PII_OSI_CLTS _SC_PII_OSI_CLTS
    _SC_PII_OSI_M,
#define _SC_PII_OSI_M _SC_PII_OSI_M
    _SC_T_IOV_MAX,
#define _SC_T_IOV_MAX _SC_T_IOV_MAX


    _SC_THREADS,
#define _SC_THREADS _SC_THREADS
    _SC_THREAD_SAFE_FUNCTIONS,
#define _SC_THREAD_SAFE_FUNCTIONS _SC_THREAD_SAFE_FUNCTIONS
    _SC_GETGR_R_SIZE_MAX,
#define _SC_GETGR_R_SIZE_MAX _SC_GETGR_R_SIZE_MAX
    _SC_GETPW_R_SIZE_MAX,
#define _SC_GETPW_R_SIZE_MAX _SC_GETPW_R_SIZE_MAX
    _SC_LOGIN_NAME_MAX,
#define _SC_LOGIN_NAME_MAX _SC_LOGIN_NAME_MAX
    _SC_TTY_NAME_MAX,
#define _SC_TTY_NAME_MAX _SC_TTY_NAME_MAX
    _SC_THREAD_DESTRUCTOR_ITERATIONS,
#define _SC_THREAD_DESTRUCTOR_ITERATIONS _SC_THREAD_DESTRUCTOR_ITERATIONS
    _SC_THREAD_KEYS_MAX,
#define _SC_THREAD_KEYS_MAX _SC_THREAD_KEYS_MAX
    _SC_THREAD_STACK_MIN,
#define _SC_THREAD_STACK_MIN _SC_THREAD_STACK_MIN
    _SC_THREAD_THREADS_MAX,
#define _SC_THREAD_THREADS_MAX _SC_THREAD_THREADS_MAX
    _SC_THREAD_ATTR_STACKADDR,
#define _SC_THREAD_ATTR_STACKADDR _SC_THREAD_ATTR_STACKADDR
    _SC_THREAD_ATTR_STACKSIZE,
#define _SC_THREAD_ATTR_STACKSIZE _SC_THREAD_ATTR_STACKSIZE
    _SC_THREAD_PRIORITY_SCHEDULING,
#define _SC_THREAD_PRIORITY_SCHEDULING _SC_THREAD_PRIORITY_SCHEDULING
    _SC_THREAD_PRIO_INHERIT,
#define _SC_THREAD_PRIO_INHERIT _SC_THREAD_PRIO_INHERIT
    _SC_THREAD_PRIO_PROTECT,
#define _SC_THREAD_PRIO_PROTECT _SC_THREAD_PRIO_PROTECT
    _SC_THREAD_PROCESS_SHARED,
#define _SC_THREAD_PROCESS_SHARED _SC_THREAD_PROCESS_SHARED

    _SC_NPROCESSORS_CONF,
#define _SC_NPROCESSORS_CONF _SC_NPROCESSORS_CONF
    _SC_NPROCESSORS_ONLN,
#define _SC_NPROCESSORS_ONLN _SC_NPROCESSORS_ONLN
    _SC_PHYS_PAGES,
#define _SC_PHYS_PAGES _SC_PHYS_PAGES
    _SC_AVPHYS_PAGES,
#define _SC_AVPHYS_PAGES _SC_AVPHYS_PAGES
    _SC_ATEXIT_MAX,
#define _SC_ATEXIT_MAX _SC_ATEXIT_MAX
    _SC_PASS_MAX,
#define _SC_PASS_MAX _SC_PASS_MAX

    _SC_XOPEN_VERSION,
#define _SC_XOPEN_VERSION _SC_XOPEN_VERSION
    _SC_XOPEN_XCU_VERSION,
#define _SC_XOPEN_XCU_VERSION _SC_XOPEN_XCU_VERSION
    _SC_XOPEN_UNIX,
#define _SC_XOPEN_UNIX _SC_XOPEN_UNIX
    _SC_XOPEN_CRYPT,
#define _SC_XOPEN_CRYPT _SC_XOPEN_CRYPT
    _SC_XOPEN_ENH_I18N,
#define _SC_XOPEN_ENH_I18N _SC_XOPEN_ENH_I18N
    _SC_XOPEN_SHM,
#define _SC_XOPEN_SHM _SC_XOPEN_SHM

    _SC_2_CHAR_TERM,
#define _SC_2_CHAR_TERM _SC_2_CHAR_TERM
    _SC_2_C_VERSION,
#define _SC_2_C_VERSION _SC_2_C_VERSION
    _SC_2_UPE,
#define _SC_2_UPE _SC_2_UPE

    _SC_XOPEN_XPG2,
#define _SC_XOPEN_XPG2 _SC_XOPEN_XPG2
    _SC_XOPEN_XPG3,
#define _SC_XOPEN_XPG3 _SC_XOPEN_XPG3
    _SC_XOPEN_XPG4,
#define _SC_XOPEN_XPG4 _SC_XOPEN_XPG4

    _SC_CHAR_BIT,
#define _SC_CHAR_BIT _SC_CHAR_BIT
    _SC_CHAR_MAX,
#define _SC_CHAR_MAX _SC_CHAR_MAX
    _SC_CHAR_MIN,
#define _SC_CHAR_MIN _SC_CHAR_MIN
    _SC_INT_MAX,
#define _SC_INT_MAX _SC_INT_MAX
    _SC_INT_MIN,
#define _SC_INT_MIN _SC_INT_MIN
    _SC_LONG_BIT,
#define _SC_LONG_BIT _SC_LONG_BIT
    _SC_WORD_BIT,
#define _SC_WORD_BIT _SC_WORD_BIT
    _SC_MB_LEN_MAX,
#define _SC_MB_LEN_MAX _SC_MB_LEN_MAX
    _SC_NZERO,
#define _SC_NZERO _SC_NZERO
    _SC_SSIZE_MAX,
#define _SC_SSIZE_MAX _SC_SSIZE_MAX
    _SC_SCHAR_MAX,
#define _SC_SCHAR_MAX _SC_SCHAR_MAX
    _SC_SCHAR_MIN,
#define _SC_SCHAR_MIN _SC_SCHAR_MIN
    _SC_SHRT_MAX,
#define _SC_SHRT_MAX _SC_SHRT_MAX
    _SC_SHRT_MIN,
#define _SC_SHRT_MIN _SC_SHRT_MIN
    _SC_UCHAR_MAX,
#define _SC_UCHAR_MAX _SC_UCHAR_MAX
    _SC_UINT_MAX,
#define _SC_UINT_MAX _SC_UINT_MAX
    _SC_ULONG_MAX,
#define _SC_ULONG_MAX _SC_ULONG_MAX
    _SC_USHRT_MAX,
#define _SC_USHRT_MAX _SC_USHRT_MAX

    _SC_NL_ARGMAX,
#define _SC_NL_ARGMAX _SC_NL_ARGMAX
    _SC_NL_LANGMAX,
#define _SC_NL_LANGMAX _SC_NL_LANGMAX
    _SC_NL_MSGMAX,
#define _SC_NL_MSGMAX _SC_NL_MSGMAX
    _SC_NL_NMAX,
#define _SC_NL_NMAX _SC_NL_NMAX
    _SC_NL_SETMAX,
#define _SC_NL_SETMAX _SC_NL_SETMAX
    _SC_NL_TEXTMAX,
#define _SC_NL_TEXTMAX _SC_NL_TEXTMAX

    _SC_XBS5_ILP32_OFF32,
#define _SC_XBS5_ILP32_OFF32 _SC_XBS5_ILP32_OFF32
    _SC_XBS5_ILP32_OFFBIG,
#define _SC_XBS5_ILP32_OFFBIG _SC_XBS5_ILP32_OFFBIG
    _SC_XBS5_LP64_OFF64,
#define _SC_XBS5_LP64_OFF64 _SC_XBS5_LP64_OFF64
    _SC_XBS5_LPBIG_OFFBIG,
#define _SC_XBS5_LPBIG_OFFBIG _SC_XBS5_LPBIG_OFFBIG

    _SC_XOPEN_LEGACY,
#define _SC_XOPEN_LEGACY _SC_XOPEN_LEGACY
    _SC_XOPEN_REALTIME,
#define _SC_XOPEN_REALTIME _SC_XOPEN_REALTIME
    _SC_XOPEN_REALTIME_THREADS,
#define _SC_XOPEN_REALTIME_THREADS _SC_XOPEN_REALTIME_THREADS

    _SC_ADVISORY_INFO,
#define _SC_ADVISORY_INFO _SC_ADVISORY_INFO
    _SC_BARRIERS,
#define _SC_BARRIERS _SC_BARRIERS
    _SC_BASE,
#define _SC_BASE _SC_BASE
    _SC_C_LANG_SUPPORT,
#define _SC_C_LANG_SUPPORT _SC_C_LANG_SUPPORT
    _SC_C_LANG_SUPPORT_R,
#define _SC_C_LANG_SUPPORT_R _SC_C_LANG_SUPPORT_R
    _SC_CLOCK_SELECTION,
#define _SC_CLOCK_SELECTION _SC_CLOCK_SELECTION
    _SC_CPUTIME,
#define _SC_CPUTIME _SC_CPUTIME
    _SC_THREAD_CPUTIME,
#define _SC_THREAD_CPUTIME _SC_THREAD_CPUTIME
    _SC_DEVICE_IO,
#define _SC_DEVICE_IO _SC_DEVICE_IO
    _SC_DEVICE_SPECIFIC,
#define _SC_DEVICE_SPECIFIC _SC_DEVICE_SPECIFIC
    _SC_DEVICE_SPECIFIC_R,
#define _SC_DEVICE_SPECIFIC_R _SC_DEVICE_SPECIFIC_R
    _SC_FD_MGMT,
#define _SC_FD_MGMT _SC_FD_MGMT
    _SC_FIFO,
#define _SC_FIFO _SC_FIFO
    _SC_PIPE,
#define _SC_PIPE _SC_PIPE
    _SC_FILE_ATTRIBUTES,
#define _SC_FILE_ATTRIBUTES _SC_FILE_ATTRIBUTES
    _SC_FILE_LOCKING,
#define _SC_FILE_LOCKING _SC_FILE_LOCKING
    _SC_FILE_SYSTEM,
#define _SC_FILE_SYSTEM _SC_FILE_SYSTEM
    _SC_MONOTONIC_CLOCK,
#define _SC_MONOTONIC_CLOCK _SC_MONOTONIC_CLOCK
    _SC_MULTI_PROCESS,
#define _SC_MULTI_PROCESS _SC_MULTI_PROCESS
    _SC_SINGLE_PROCESS,
#define _SC_SINGLE_PROCESS _SC_SINGLE_PROCESS
    _SC_NETWORKING,
#define _SC_NETWORKING _SC_NETWORKING
    _SC_READER_WRITER_LOCKS,
#define _SC_READER_WRITER_LOCKS _SC_READER_WRITER_LOCKS
    _SC_SPIN_LOCKS,
#define _SC_SPIN_LOCKS _SC_SPIN_LOCKS
    _SC_REGEXP,
#define _SC_REGEXP _SC_REGEXP
    _SC_REGEX_VERSION,
#define _SC_REGEX_VERSION _SC_REGEX_VERSION
    _SC_SHELL,
#define _SC_SHELL _SC_SHELL
    _SC_SIGNALS,
#define _SC_SIGNALS _SC_SIGNALS
    _SC_SPAWN,
#define _SC_SPAWN _SC_SPAWN
    _SC_SPORADIC_SERVER,
#define _SC_SPORADIC_SERVER _SC_SPORADIC_SERVER
    _SC_THREAD_SPORADIC_SERVER,
#define _SC_THREAD_SPORADIC_SERVER _SC_THREAD_SPORADIC_SERVER
    _SC_SYSTEM_DATABASE,
#define _SC_SYSTEM_DATABASE _SC_SYSTEM_DATABASE
    _SC_SYSTEM_DATABASE_R,
#define _SC_SYSTEM_DATABASE_R _SC_SYSTEM_DATABASE_R
    _SC_TIMEOUTS,
#define _SC_TIMEOUTS _SC_TIMEOUTS
    _SC_TYPED_MEMORY_OBJECTS,
#define _SC_TYPED_MEMORY_OBJECTS _SC_TYPED_MEMORY_OBJECTS
    _SC_USER_GROUPS,
#define _SC_USER_GROUPS _SC_USER_GROUPS
    _SC_USER_GROUPS_R,
#define _SC_USER_GROUPS_R _SC_USER_GROUPS_R
    _SC_2_PBS,
#define _SC_2_PBS _SC_2_PBS
    _SC_2_PBS_ACCOUNTING,
#define _SC_2_PBS_ACCOUNTING _SC_2_PBS_ACCOUNTING
    _SC_2_PBS_LOCATE,
#define _SC_2_PBS_LOCATE _SC_2_PBS_LOCATE
    _SC_2_PBS_MESSAGE,
#define _SC_2_PBS_MESSAGE _SC_2_PBS_MESSAGE
    _SC_2_PBS_TRACK,
#define _SC_2_PBS_TRACK _SC_2_PBS_TRACK
    _SC_SYMLOOP_MAX,
#define _SC_SYMLOOP_MAX _SC_SYMLOOP_MAX
    _SC_STREAMS,
#define _SC_STREAMS _SC_STREAMS
    _SC_2_PBS_CHECKPOINT,
#define _SC_2_PBS_CHECKPOINT _SC_2_PBS_CHECKPOINT

    _SC_V6_ILP32_OFF32,
#define _SC_V6_ILP32_OFF32 _SC_V6_ILP32_OFF32
    _SC_V6_ILP32_OFFBIG,
#define _SC_V6_ILP32_OFFBIG _SC_V6_ILP32_OFFBIG
    _SC_V6_LP64_OFF64,
#define _SC_V6_LP64_OFF64 _SC_V6_LP64_OFF64
    _SC_V6_LPBIG_OFFBIG,
#define _SC_V6_LPBIG_OFFBIG _SC_V6_LPBIG_OFFBIG

    _SC_HOST_NAME_MAX,
#define _SC_HOST_NAME_MAX _SC_HOST_NAME_MAX
    _SC_TRACE,
#define _SC_TRACE _SC_TRACE
    _SC_TRACE_EVENT_FILTER,
#define _SC_TRACE_EVENT_FILTER _SC_TRACE_EVENT_FILTER
    _SC_TRACE_INHERIT,
#define _SC_TRACE_INHERIT _SC_TRACE_INHERIT
    _SC_TRACE_LOG
#define _SC_TRACE_LOG _SC_TRACE_LOG
  };


enum
  {
    _CS_PATH,
#define _CS_PATH _CS_PATH

    _CS_V6_WIDTH_RESTRICTED_ENVS,
#define _CS_V6_WIDTH_RESTRICTED_ENVS _CS_V6_WIDTH_RESTRICTED_ENVS

    _CS_GNU_LIBC_VERSION,
#define _CS_GNU_LIBC_VERSION _CS_GNU_LIBC_VERSION
    _CS_GNU_LIBPTHREAD_VERSION,
#define _CS_GNU_LIBPTHREAD_VERSION _CS_GNU_LIBPTHREAD_VERSION

    _CS_LFS_CFLAGS = 1000,
#define _CS_LFS_CFLAGS _CS_LFS_CFLAGS
    _CS_LFS_LDFLAGS,
#define _CS_LFS_LDFLAGS _CS_LFS_LDFLAGS
    _CS_LFS_LIBS,
#define _CS_LFS_LIBS _CS_LFS_LIBS
    _CS_LFS_LINTFLAGS,
#define _CS_LFS_LINTFLAGS _CS_LFS_LINTFLAGS
    _CS_LFS64_CFLAGS,
#define _CS_LFS64_CFLAGS _CS_LFS64_CFLAGS
    _CS_LFS64_LDFLAGS,
#define _CS_LFS64_LDFLAGS _CS_LFS64_LDFLAGS
    _CS_LFS64_LIBS,
#define _CS_LFS64_LIBS _CS_LFS64_LIBS
    _CS_LFS64_LINTFLAGS,
#define _CS_LFS64_LINTFLAGS _CS_LFS64_LINTFLAGS

    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,
#define _CS_XBS5_ILP32_OFF32_CFLAGS _CS_XBS5_ILP32_OFF32_CFLAGS
    _CS_XBS5_ILP32_OFF32_LDFLAGS,
#define _CS_XBS5_ILP32_OFF32_LDFLAGS _CS_XBS5_ILP32_OFF32_LDFLAGS
    _CS_XBS5_ILP32_OFF32_LIBS,
#define _CS_XBS5_ILP32_OFF32_LIBS _CS_XBS5_ILP32_OFF32_LIBS
    _CS_XBS5_ILP32_OFF32_LINTFLAGS,
#define _CS_XBS5_ILP32_OFF32_LINTFLAGS _CS_XBS5_ILP32_OFF32_LINTFLAGS
    _CS_XBS5_ILP32_OFFBIG_CFLAGS,
#define _CS_XBS5_ILP32_OFFBIG_CFLAGS _CS_XBS5_ILP32_OFFBIG_CFLAGS
    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,
#define _CS_XBS5_ILP32_OFFBIG_LDFLAGS _CS_XBS5_ILP32_OFFBIG_LDFLAGS
    _CS_XBS5_ILP32_OFFBIG_LIBS,
#define _CS_XBS5_ILP32_OFFBIG_LIBS _CS_XBS5_ILP32_OFFBIG_LIBS
    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,
#define _CS_XBS5_ILP32_OFFBIG_LINTFLAGS _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
    _CS_XBS5_LP64_OFF64_CFLAGS,
#define _CS_XBS5_LP64_OFF64_CFLAGS _CS_XBS5_LP64_OFF64_CFLAGS
    _CS_XBS5_LP64_OFF64_LDFLAGS,
#define _CS_XBS5_LP64_OFF64_LDFLAGS _CS_XBS5_LP64_OFF64_LDFLAGS
    _CS_XBS5_LP64_OFF64_LIBS,
#define _CS_XBS5_LP64_OFF64_LIBS _CS_XBS5_LP64_OFF64_LIBS
    _CS_XBS5_LP64_OFF64_LINTFLAGS,
#define _CS_XBS5_LP64_OFF64_LINTFLAGS _CS_XBS5_LP64_OFF64_LINTFLAGS
    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,
#define _CS_XBS5_LPBIG_OFFBIG_CFLAGS _CS_XBS5_LPBIG_OFFBIG_CFLAGS
    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,
#define _CS_XBS5_LPBIG_OFFBIG_LDFLAGS _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
    _CS_XBS5_LPBIG_OFFBIG_LIBS,
#define _CS_XBS5_LPBIG_OFFBIG_LIBS _CS_XBS5_LPBIG_OFFBIG_LIBS
    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,
#define _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS

    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,
#define _CS_POSIX_V6_ILP32_OFF32_CFLAGS _CS_POSIX_V6_ILP32_OFF32_CFLAGS
    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,
#define _CS_POSIX_V6_ILP32_OFF32_LDFLAGS _CS_POSIX_V6_ILP32_OFF32_LDFLAGS
    _CS_POSIX_V6_ILP32_OFF32_LIBS,
#define _CS_POSIX_V6_ILP32_OFF32_LIBS _CS_POSIX_V6_ILP32_OFF32_LIBS
    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,
#define _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS
    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,
#define _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS
    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,
#define _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS
    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,
#define _CS_POSIX_V6_ILP32_OFFBIG_LIBS _CS_POSIX_V6_ILP32_OFFBIG_LIBS
    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,
#define _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS
    _CS_POSIX_V6_LP64_OFF64_CFLAGS,
#define _CS_POSIX_V6_LP64_OFF64_CFLAGS _CS_POSIX_V6_LP64_OFF64_CFLAGS
    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,
#define _CS_POSIX_V6_LP64_OFF64_LDFLAGS _CS_POSIX_V6_LP64_OFF64_LDFLAGS
    _CS_POSIX_V6_LP64_OFF64_LIBS,
#define _CS_POSIX_V6_LP64_OFF64_LIBS _CS_POSIX_V6_LP64_OFF64_LIBS
    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,
#define _CS_POSIX_V6_LP64_OFF64_LINTFLAGS _CS_POSIX_V6_LP64_OFF64_LINTFLAGS
    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,
#define _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS
    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,
#define _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS
    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,
#define _CS_POSIX_V6_LPBIG_OFFBIG_LIBS _CS_POSIX_V6_LPBIG_OFFBIG_LIBS
    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS
#define _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS
  };
# 526 "/usr/include/unistd.h" 2 3


extern long int pathconf (__const char *__path, int __name) ;


extern long int fpathconf (int __fd, int __name) ;


extern long int sysconf (int __name) __attribute__ ((__const__));



extern size_t confstr (int __name, char *__buf, size_t __len) ;




extern __pid_t getpid (void) ;


extern __pid_t getppid (void) ;




extern __pid_t getpgrp (void) ;
# 561 "/usr/include/unistd.h" 3
extern __pid_t __getpgid (__pid_t __pid) ;
# 570 "/usr/include/unistd.h" 3
extern int setpgid (__pid_t __pid, __pid_t __pgid) ;
# 587 "/usr/include/unistd.h" 3
extern int setpgrp (void) ;
# 605 "/usr/include/unistd.h" 3
extern __pid_t setsid (void) ;







extern __uid_t getuid (void) ;


extern __uid_t geteuid (void) ;


extern __gid_t getgid (void) ;


extern __gid_t getegid (void) ;




extern int getgroups (int __size, __gid_t __list[]) ;
# 638 "/usr/include/unistd.h" 3
extern int setuid (__uid_t __uid) ;




extern int setreuid (__uid_t __ruid, __uid_t __euid) ;




extern int seteuid (__uid_t __uid) ;






extern int setgid (__gid_t __gid) ;




extern int setregid (__gid_t __rgid, __gid_t __egid) ;




extern int setegid (__gid_t __gid) ;
# 690 "/usr/include/unistd.h" 3
extern __pid_t fork (void) ;






extern __pid_t vfork (void) ;





extern char *ttyname (int __fd) ;



extern int ttyname_r (int __fd, char *__buf, size_t __buflen) ;



extern int isatty (int __fd) ;





extern int ttyslot (void) ;




extern int link (__const char *__from, __const char *__to) ;



extern int symlink (__const char *__from, __const char *__to) ;




extern int readlink (__const char *__restrict __path, char *__restrict __buf,
                     size_t __len) ;



extern int unlink (__const char *__name) ;


extern int rmdir (__const char *__path) ;



extern __pid_t tcgetpgrp (int __fd) ;


extern int tcsetpgrp (int __fd, __pid_t __pgrp_id) ;






extern char *getlogin (void);
# 766 "/usr/include/unistd.h" 3
extern int setlogin (__const char *__name) ;







#define __need_getopt 
# 1 "/usr/include/getopt.h" 1 3
# 55 "/usr/include/getopt.h" 3
extern char *optarg;
# 69 "/usr/include/getopt.h" 3
extern int optind;




extern int opterr;



extern int optopt;
# 153 "/usr/include/getopt.h" 3
extern int getopt (int ___argc, char *const *___argv, const char *__shortopts)
       ;
# 190 "/usr/include/getopt.h" 3
#undef __need_getopt
# 776 "/usr/include/unistd.h" 2 3







extern int gethostname (char *__name, size_t __len) ;






extern int sethostname (__const char *__name, size_t __len) ;



extern int sethostid (long int __id) ;





extern int getdomainname (char *__name, size_t __len) ;
extern int setdomainname (__const char *__name, size_t __len) ;





extern int vhangup (void) ;


extern int revoke (__const char *__file) ;







extern int profil (unsigned short int *__sample_buffer, size_t __size,
                   size_t __offset, unsigned int __scale) ;





extern int acct (__const char *__name) ;



extern char *getusershell (void) ;
extern void endusershell (void) ;
extern void setusershell (void) ;





extern int daemon (int __nochdir, int __noclose) ;






extern int chroot (__const char *__path) ;



extern char *getpass (__const char *__prompt);
# 857 "/usr/include/unistd.h" 3
extern int fsync (int __fd);






extern long int gethostid (void);


extern void sync (void) ;




extern int getpagesize (void) __attribute__ ((__const__));




extern int truncate (__const char *__file, __off_t __length) ;
# 893 "/usr/include/unistd.h" 3
extern int ftruncate (int __fd, __off_t __length) ;
# 909 "/usr/include/unistd.h" 3
extern int getdtablesize (void) ;
# 918 "/usr/include/unistd.h" 3
extern int brk (void *__addr) ;





extern void *sbrk (intptr_t __delta) ;
# 939 "/usr/include/unistd.h" 3
extern long int syscall (long int __sysno, ...) ;
# 993 "/usr/include/unistd.h" 3
extern int fdatasync (int __fildes) ;
# 1024 "/usr/include/unistd.h" 3

# 35 "../../include/mmdb_destip.h" 2

# 1 "/usr/include/sys/msg.h" 1 3
# 20 "/usr/include/sys/msg.h" 3
#define _SYS_MSG_H 







# 1 "/usr/include/bits/msq.h" 1 3
# 26 "/usr/include/bits/msq.h" 3
#define MSG_NOERROR 010000





typedef unsigned long int msgqnum_t;
typedef unsigned long int msglen_t;




struct msqid_ds
{
  struct ipc_perm msg_perm;
  __time_t msg_stime;
  unsigned long int __unused1;
  __time_t msg_rtime;
  unsigned long int __unused2;
  __time_t msg_ctime;
  unsigned long int __unused3;
  unsigned long int __msg_cbytes;
  msgqnum_t msg_qnum;
  msglen_t msg_qbytes;
  __pid_t msg_lspid;
  __pid_t msg_lrpid;
  unsigned long int __unused4;
  unsigned long int __unused5;
};



#define msg_cbytes __msg_cbytes


#define MSG_STAT 11
#define MSG_INFO 12


struct msginfo
  {
    int msgpool;
    int msgmap;
    int msgmax;
    int msgmnb;
    int msgmni;
    int msgssz;
    int msgtql;
    unsigned short int msgseg;
  };
# 29 "/usr/include/sys/msg.h" 2 3


#define __need_time_t 
# 57 "/usr/include/sys/msg.h" 3



extern int msgctl (int __msqid, int __cmd, struct msqid_ds *__buf) ;


extern int msgget (key_t __key, int __msgflg) ;





extern int msgrcv (int __msqid, void *__msgp, size_t __msgsz,
                   long int __msgtyp, int __msgflg);





extern int msgsnd (int __msqid, __const void *__msgp, size_t __msgsz,
                   int __msgflg);


# 37 "../../include/mmdb_destip.h" 2


# 1 "../../include/ipaf_define.h" 1
# 18 "../../include/ipaf_define.h"
#define __IPAM_DEFINE_HEADER_FILE__ 

#define INTEL 1

#define PROC_PATH "/proc"
#define PARENT_PATH ".."
#define HOME_PATH "."

#define START_PATH "/DSC"
#define APP_PATH "/DSC/NEW"
#define BACKUP_PATH "/DSC/OLD"

#define BIN_PATH START_PATH"/NEW/BIN/"

#define APP_HOME_BIN BIN_PATH
#define APP_HOME BIN_PATH


#define DATA_PATH START_PATH"/NEW/DATA/"
#define FIDB_FILE DATA_PATH"fidb.mem"
#define DEF_IP_POOL_FILE DATA_PATH"IP_POOL.dat"
#define DEF_SER_CAT_FILE DATA_PATH"SERVICE_CATEGORY.dat"
#define DEF_SER_CAT_NEW_FILE DATA_PATH"SERVICE_CATEGORY_ADD.dat"

#define MC_INIT DATA_PATH"McInit"
#define TOTO_IPADDR_FILE DATA_PATH"ipaddr.dat"
#define ALARMCLASS_FILE DATA_PATH"AlarmClass"

#define VOD_PAYTYPE_TIME DATA_PATH"VODS_TIMEOUT.dat"

#define DEF_SYSCONF_FILE DATA_PATH"sysconfig"


#define LOG_PATH START_PATH"/APPLOG/"
#define CHSMD_LOG LOG_PATH"CHSMD"
#define ALMD_LOG LOG_PATH"ALMD"
#define COND_LOG LOG_PATH"COND"
#define MMCD_LOG LOG_PATH"MMCD"
#define QMONITOR_LOG LOG_PATH"QMONITOR"
#define RMI_LOG LOG_PATH"RMI"
#define MMC_LOG LOG_PATH"MMC"
#define EDFALMD_LOG LOG_PATH"EDFALMD"
#define IPAFALMD_LOG LOG_PATH"IPAFALMD"

#define IPAMTIF_LOG LOG_PATH"IPAMTIF"
#define IPAMUIF_LOG LOG_PATH"IPAMUIF"
#define MDBMGR_LOG LOG_PATH"MDBMGR"


#define SVCANA_LOG LOG_PATH"SVCANA"
#define ETH_CAPD_LOG LOG_PATH"ETHCAPD"
#define SESSANA_LOG LOG_PATH"SESSANA"

#define RDRIF_LOG LOG_PATH"RDRIF"
#define MESVC_LOG LOG_PATH"MESVC"
#define KUNSVC_LOG LOG_PATH"KUNSVC"
#define ADSSVC_LOG LOG_PATH"ADSSVC"
#define MARSSVC_LOG LOG_PATH"MARSSVC"

#define MACSSVC_LOG LOG_PATH"MACSSVC"
#define WICGSSVC_LOG LOG_PATH"WICGSSVC"

#define VODANA_LOG LOG_PATH"VODANA"

#define MESSAGE_FILE "/var/log/messages"
#define FAN_FILE "/proc/cpqfan"
#define ENCLO_S_FILE DATA_PATH"ENCLO_SERIAL.dat"

#define MAX_MSGQ_COUNT 100


#define MSGQ_DEFINE 8000
#define PORT_DEFINE 18000
#define PROC_DEFINE 9000
#define SSHM_DEFINE 10000
#define SEMA_DEFINE 11000


#define S_MSGQ_CHSMD ( MSGQ_DEFINE + 0 )
#define S_MSGQ_COND ( MSGQ_DEFINE + 1 )
#define S_MSGQ_MMCD ( MSGQ_DEFINE + 2 )
#define S_MSGQ_EDFALMD ( MSGQ_DEFINE + 3 )
#define S_MSGQ_IPAFALMD ( MSGQ_DEFINE + 4 )
#define S_MSGQ_ALMD ( MSGQ_DEFINE + 5 )


#define S_MSGQ_IPAMTIF ( MSGQ_DEFINE + 11 )
#define S_MSGQ_IPAMUIF ( MSGQ_DEFINE + 12 )
#define S_MSGQ_CDRSVR ( MSGQ_DEFINE + 13 )
#define S_MSGQ_MDBMGR ( MSGQ_DEFINE + 14 )
#define S_MSGQ_ADMIN ( MSGQ_DEFINE + 15 )


#define S_MSGQ_MGNSVC ( MSGQ_DEFINE + 21 )
#define S_MSGQ_RDRIF ( MSGQ_DEFINE + 22 )
#define S_MSGQ_MESVC ( MSGQ_DEFINE + 23 )
#define S_MSGQ_SESSANA ( MSGQ_DEFINE + 24 )
#define S_MSGQ_KUNSVC ( MSGQ_DEFINE + 25 )
#define S_MSGQ_ADSSVC ( MSGQ_DEFINE + 26 )
#define S_MSGQ_MARSSVC ( MSGQ_DEFINE + 27 )
#define S_MSGQ_VODANA ( MSGQ_DEFINE + 28 )
#define S_MSGQ_VODMANA ( MSGQ_DEFINE + 29 )
#define S_MSGQ_VODDANA ( MSGQ_DEFINE + 30 )
#define S_MSGQ_SIM ( MSGQ_DEFINE + 40 )

#define S_MSGQ_MACSSVC ( MSGQ_DEFINE + 41 )
#define S_MSGQ_WICGSSVC ( MSGQ_DEFINE + 42 )
#define S_MSGQ_AAAIFSVC ( MSGQ_DEFINE + 43 )


#define S_PORT_ALMD ( PORT_DEFINE + 301 )
#define S_PORT_COND ( PORT_DEFINE + 302 )
#define S_PORT_MMCD ( PORT_DEFINE + 500 )
#define S_PORT_IPAMTIF ( PORT_DEFINE + 600 )
#define S_PORT_IPAMUIF ( PORT_DEFINE + 700 )


#define S_PORT_RDRIF ( PORT_DEFINE + 800 )

#define S_PORT_ACCOUNT 1813
#define S_PORT_QUD 49149

#define S_SSHM_FIDB ( SSHM_DEFINE + 300 )
#define S_SSHM_KEEPALIVE ( SSHM_DEFINE + 350 )
#define S_SSHM_UPGRADE_DB ( SSHM_DEFINE + 360 )
#define S_SSHM_GENINFO ( SSHM_DEFINE + 370 )
#define S_SSHM_MEMHDR ( SSHM_DEFINE + 380 )
#define S_SSHM_MEM ( SSHM_DEFINE + 381 )
//#define S_SSHM_MMDBSESS ( SSHM_DEFINE + 210 )
//#define S_SSHM_MMDBOBJ ( SSHM_DEFINE + 211 )
#define S_SSHM_MMDBCDR ( SSHM_DEFINE + 212 )
//#define S_SSHM_MMDBDESTIP ( SSHM_DEFINE + 213 )
//#define S_SSHM_MMDBDESTPORT ( SSHM_DEFINE + 214 )
#define S_SSHM_VERSION ( SSHM_DEFINE + 215 )
#define S_SSHM_MMDBLIST ( SSHM_DEFINE + 216 )
#define S_SSHM_SESSANA ( SSHM_DEFINE + 217 )
#define S_SSHM_MESVC ( SSHM_DEFINE + 218 )
#define S_SSHM_KUNSVC ( SSHM_DEFINE + 219 )
#define S_SSHM_ADSSVC ( SSHM_DEFINE + 220 )
#define S_SSHM_MARSSVC ( SSHM_DEFINE + 221 )

#define S_SSHM_MESTAT ( SSHM_DEFINE + 222 )
#define S_SSHM_KUNSTAT ( SSHM_DEFINE + 223 )
#define S_SSHM_ADSSTAT ( SSHM_DEFINE + 224 )
#define S_SSHM_MARSSTAT ( SSHM_DEFINE + 225 )
//#define S_SSHM_SESSSTAT ( SSHM_DEFINE + 226 )

//#define S_SSHM_RDRSEQ ( SSHM_DEFINE + 227 )

#define S_SSHM_VODANA ( SSHM_DEFINE + 228 )
#define S_SSHM_VODUDP ( SSHM_DEFINE + 229 )

#define S_SSHM_MACSSVC ( SSHM_DEFINE + 230 )
#define S_SSHM_MACSSTAT ( SSHM_DEFINE + 231 )

#define S_SSHM_VODMANA ( SSHM_DEFINE + 232 )
#define S_SSHM_VODDANA ( SSHM_DEFINE + 233 )

#define S_SSHM_WICGSSVC ( SSHM_DEFINE + 234 )
#define S_SSHM_WICGSSTAT ( SSHM_DEFINE + 235 )

#define S_SSHM_VODSSTAT ( SSHM_DEFINE + 236 )
#define S_SSHM_VODMSTAT ( SSHM_DEFINE + 237 )
#define S_SSHM_VODDSTAT ( SSHM_DEFINE + 238 )

#define S_SSHM_WAPSVC ( SSHM_DEFINE + 239 )
#define S_SSHM_WAPSTAT ( SSHM_DEFINE + 240 )



#define S_SEMA_SESS ( SEMA_DEFINE + 0 )
#define S_SEMA_CDR ( SEMA_DEFINE + 1 )
#define S_SEMA_OBJ ( SEMA_DEFINE + 2 )
#define S_SEMA_DESTIP ( SEMA_DEFINE + 3 )
#define S_SEMA_DESTPORT ( SEMA_DEFINE + 4 )
#define S_SEMA_CAPINDEX ( SEMA_DEFINE + 5 )
# 40 "../../include/mmdb_destip.h" 2
# 1 "../../include/ipaf_sem.h" 1



# 1 "/usr/include/sys/sem.h" 1 3
# 20 "/usr/include/sys/sem.h" 3
#define _SYS_SEM_H 1



#define __need_size_t 
# 1 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 1 3
# 235 "/usr/lib/gcc-lib/i386-redhat-linux/3.2.3/include/stddef.h" 3
#undef __need_size_t
# 26 "/usr/include/sys/sem.h" 2 3





# 1 "/usr/include/bits/sem.h" 1 3
# 26 "/usr/include/bits/sem.h" 3
#define SEM_UNDO 0x1000


#define GETPID 11
#define GETVAL 12
#define GETALL 13
#define GETNCNT 14
#define GETZCNT 15
#define SETVAL 16
#define SETALL 17



struct semid_ds
{
  struct ipc_perm sem_perm;
  __time_t sem_otime;
  unsigned long int __unused1;
  __time_t sem_ctime;
  unsigned long int __unused2;
  unsigned long int sem_nsems;
  unsigned long int __unused3;
  unsigned long int __unused4;
};
# 65 "/usr/include/bits/sem.h" 3
#define _SEM_SEMUN_UNDEFINED 1




#define SEM_STAT 18
#define SEM_INFO 19

struct seminfo
{
  int semmap;
  int semmni;
  int semmns;
  int semmnu;
  int semmsl;
  int semopm;
  int semume;
  int semusz;
  int semvmx;
  int semaem;
};
# 32 "/usr/include/sys/sem.h" 2 3
# 42 "/usr/include/sys/sem.h" 3
struct sembuf
{
  unsigned short int sem_num;
  short int sem_op;
  short int sem_flg;
};





extern int semctl (int __semid, int __semnum, int __cmd, ...) ;


extern int semget (key_t __key, int __nsems, int __semflg) ;


extern int semop (int __semid, struct sembuf *__sops, size_t __nsops) ;








# 5 "../../include/ipaf_sem.h" 2

# 1 "/usr/include/errno.h" 1 3
# 7 "../../include/ipaf_sem.h" 2

extern int (*__errno_location ());

#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

int Init_sem( key_t semkey );
int Remove_sem( key_t semkey );
int p( int semid );
int v( int semid );
# 41 "../../include/mmdb_destip.h" 2



#define DEF_ALL_ACCT 4
#define DEF_CONTENT 3
#define DEF_SERVICE_CONTENT 2
#define DEF_SERVICE 1

typedef struct _st_destip_key {
        unsigned char ucFlag;
        unsigned char Reserved[3];
    unsigned int uiIP;
} st_destip_key, *pst_destip_key;

typedef struct _st_destip {
    st_destip_key key;

        unsigned int uiNetmask;
        unsigned short usCatID;
        unsigned char ucGroupID;
        unsigned char ucSerial;
        unsigned char ucFilterOut;

        unsigned char ucLayer;
        unsigned char ucMode;
        unsigned char ucSvcBlk;
        unsigned char ucIPType;
        unsigned char ucURLChar;
        unsigned char Reserved[2];
} st_destip, *pst_destip;



#define DESTIP_KEY st_destip_key
#define PDESTIP_KEY pst_destip_key
#define DESTIP_DATA st_destip
#define PDESTIP_DATA pst_destip


#define JMM_DESTIP_RECORD 413


#define JMM_DESTIP_KEY_LEN ( sizeof( DESTIP_KEY ) / 8 )
#define JMM_DESTIP_BODY_LEN ( ( sizeof( DESTIP_DATA ) / 8 ) - JMM_DESTIP_KEY_LEN )


#define DESTIP_SHM_KEY S_SSHM_MMDBDESTIP



typedef struct JMM_typical_tbl_destip {
        st_destip_key key;
        long long body[( ( sizeof( st_destip ) / 8 ) - ( sizeof( st_destip_key ) / 8 ) )];
        int left;
        int right;
        short bf;
        short reserved[3];
} DESTIP_TYPE;


typedef struct {
        DESTIP_TYPE tuple[413];
        int free;
        int root;
        unsigned int uiCount;
        int reserved[127];
} DESTIP_TABLE;



extern DESTIP_TABLE *destip_tbl;
extern int semid_destip;



int avl_insert_destip( pst_destip_key key, long long *body, int *root );
int left_rotation_destip( int index, int *pindex );
int right_rotation_destip( int index, int *pindex );
DESTIP_TYPE *avl_search_destip( int root, pst_destip_key key );
int avl_delete_destip( int *root, pst_destip_key key );
DESTIP_TYPE *avl_select_destip( int root, pst_destip_key first_key, pst_destip_key last_key );
int avl_update_destip( DESTIP_TYPE *tree, long long *body );
DESTIP_TYPE *get_destip( int index );

int destip_alloc();
void destip_dealloc( int index );

int Insert_DESTIP( pst_destip disp );
pst_destip Search_DESTIP( pst_destip_key key );
int Delete_DESTIP( pst_destip_key key );
int Update_DESTIP( pst_destip disp, pst_destip input );
pst_destip Select_DESTIP( pst_destip_key first_key, pst_destip_key last_key );
void KeyTo_DESTIP( pst_destip_key key );
pst_destip Filter_DESTIP( pst_destip_key key );
# 10 "shmmb_print.c" 2
# 1 "../../include/mmdb_destport.h" 1
# 19 "../../include/mmdb_destport.h"
#define __MM_DESTPORT_DB_HEADER___ 
# 32 "../../include/mmdb_destport.h"
# 1 "/usr/include/errno.h" 1 3
# 33 "../../include/mmdb_destport.h" 2







# 1 "../../include/ipaf_sem.h" 1





# 1 "/usr/include/errno.h" 1 3
# 7 "../../include/ipaf_sem.h" 2

extern int (*__errno_location ());

#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

int Init_sem( key_t semkey );
int Remove_sem( key_t semkey );
int p( int semid );
int v( int semid );
# 41 "../../include/mmdb_destport.h" 2



typedef struct _st_destport_key {
    unsigned char ucProtocol;
        unsigned char Reserved;
        unsigned short usDestPort;

    unsigned int uiDestIP;
} st_destport_key, *pst_destport_key;

typedef struct _st_destport {
    st_destport_key key;

        unsigned int uiNetmask;
    unsigned short usCatID;
        unsigned char ucGroupID;
        unsigned char ucSerial;

        unsigned char ucFilterOut;
        unsigned char ucLayer;
        unsigned char ucMode;
        unsigned char ucSvcBlk;
        unsigned char ucIPType;
        unsigned char ucURLChar;
        unsigned char Reserved[2];
} st_destport, *pst_destport;


#define JMM_DESTPORT_RECORD 303


typedef struct _st_DestIPList
{
    int dCount;
    int dReserved;
    st_destport stDestIP[303];
}stDestIPList, *pstDestIPList;



#define DESTPORT_KEY st_destport_key
#define PDESTPORT_KEY pst_destport_key
#define DESTPORT_DATA st_destport
#define PDESTPORT_DATA pst_destport


#define JMM_DESTPORT_KEY_LEN ( sizeof( DESTPORT_KEY ) / 8 )
#define JMM_DESTPORT_BODY_LEN ( ( sizeof( DESTPORT_DATA ) / 8 ) - JMM_DESTPORT_KEY_LEN )


#define DESTPORT_SHM_KEY S_SSHM_MMDBDESTPORT



typedef struct JMM_typical_tbl_destport {
        st_destport_key key;
        long long body[( ( sizeof( st_destport ) / 8 ) - ( sizeof( st_destport_key ) / 8 ) )];
        int left;
        int right;
        short bf;
        short reserved[3];
} DESTPORT_TYPE;


typedef struct {
        DESTPORT_TYPE tuple[303];
        int free;
        int root;
        unsigned int uiCount;
        int reserved[127];
} DESTPORT_TABLE;



extern DESTPORT_TABLE *destport_tbl;
extern int semid_destport;


int avl_insert_destport( pst_destport_key key, long long *body, int *root );
int left_rotation_destport( int index, int *pindex );
int right_rotation_destport( int index, int *pindex );
DESTPORT_TYPE *avl_search_destport( int root, pst_destport_key key );
int avl_delete_destport( int *root, pst_destport_key key );
DESTPORT_TYPE *avl_select_destport( int root, pst_destport_key first_key, pst_destport_key last_key );
int avl_update_destport( DESTPORT_TYPE *tree, long long *body );
DESTPORT_TYPE *get_destport( int index );

int destport_alloc();
void destport_dealloc( int index );

int Insert_DESTPORT( pst_destport disp );
pst_destport Search_DESTPORT( pst_destport_key key );
int Delete_DESTPORT( pst_destport_key key );
int Update_DESTPORT( pst_destport disp, pst_destport input );
pst_destport Select_DESTPORT( pst_destport_key first_key, pst_destport_key last_key );
pst_destport Filter_DESTPORT( pst_destport_key key );
# 11 "shmmb_print.c" 2
# 1 "../../include/mmdb_svcopt.h" 1
# 19 "../../include/mmdb_svcopt.h"
#define __MM_SVCOPT_DB_HEADER___ 
# 32 "../../include/mmdb_svcopt.h"
# 1 "/usr/include/errno.h" 1 3
# 33 "../../include/mmdb_svcopt.h" 2







# 1 "../../include/ipaf_sem.h" 1





# 1 "/usr/include/errno.h" 1 3
# 7 "../../include/ipaf_sem.h" 2

extern int (*__errno_location ());

#define SEMPERM 0600
#define TRUE 1
#define FALSE 0

int Init_sem( key_t semkey );
int Remove_sem( key_t semkey );
int p( int semid );
int v( int semid );
# 41 "../../include/mmdb_svcopt.h" 2


typedef struct _st_svcopt_key {
        unsigned short svcOpt;
        unsigned char Reserved[6];
} st_svcopt_key, *pst_svcopt_key;

typedef struct _st_svcopt {
    st_svcopt_key key;
        unsigned short svcType;
        unsigned char layer;
        unsigned char fout;
        unsigned char block;
        unsigned char urlCha;
        unsigned char Reserved[2];
} st_svcopt, *pst_svcopt;



#define SVCOPT_KEY st_svcopt_key
#define PSVCOPT_KEY pst_svcopt_key
#define SVCOPT_DATA st_svcopt
#define PSVCOPT_DATA pst_svcopt


#define S_SEMA_SVCOPT 0x20001


#define JMM_SVCOPT_RECORD 100


#define JMM_SVCOPT_KEY_LEN ( sizeof( SVCOPT_KEY ) / 8 )
#define JMM_SVCOPT_BODY_LEN ( ( sizeof( SVCOPT_DATA ) / 8 ) - JMM_SVCOPT_KEY_LEN )




typedef struct JMM_typical_tbl_svcopt {
        st_svcopt_key key;
        long long body[( ( sizeof( st_svcopt ) / 8 ) - ( sizeof( st_svcopt_key ) / 8 ) )];
        int left;
        int right;
        short bf;
        short reserved[3];
} SVCOPT_TYPE;


typedef struct {
        SVCOPT_TYPE tuple[100];
        int free;
        int root;
        unsigned int uiCount;
        int reserved[127];
} SVCOPT_TABLE;


extern SVCOPT_TABLE *svcopt_tbl;
extern int semid_svcopt;



int avl_insert_svcopt( pst_svcopt_key key, long long *body, int *root );
int left_rotation_svcopt( int index, int *pindex );
int right_rotation_svcopt( int index, int *pindex );
SVCOPT_TYPE *avl_search_svcopt( int root, pst_svcopt_key key );
int avl_delete_svcopt( int *root, pst_svcopt_key key );
SVCOPT_TYPE *avl_select_svcopt( int root, pst_svcopt_key first_key, pst_svcopt_key last_key );
int avl_update_svcopt( SVCOPT_TYPE *tree, long long *body );
SVCOPT_TYPE *get_svcopt( int index );

int svcopt_alloc();
void svcopt_dealloc( int index );

int Insert_SVCOPT( pst_svcopt disp );
pst_svcopt Search_SVCOPT( pst_svcopt_key key );
int Delete_SVCOPT( pst_svcopt_key key );
int Update_SVCOPT( pst_svcopt disp, pst_svcopt input );
pst_svcopt Select_SVCOPT( pst_svcopt_key first_key, pst_svcopt_key last_key );
void KeyTo_SVCOPT( pst_svcopt_key key );
# 12 "shmmb_print.c" 2


#define IPTYPE_IPPOOL 0

#define PRINT_IP(b) { struct in_addr addr; addr.s_addr = b; printf( "%-15s", inet_ntoa(addr)); }







#define PRINT_PORT(a) printf( "%5d", a);
#define PRINT_SVCOPT(a) printf( "%5d", a);
#define PRINT_SVCTYPE(a) printf( "%5d", a);
#define PRINT_CATEGORY(a) printf( "%8d", a);
#define PRINT_SVCBLOCK(a) { char *cp = NULL; switch(a){ case 1: cp = "CDR"; break; case 2: cp = "WAP1ANA"; break; case 3: cp = "UAWAPANA"; break; case 4: cp = "WAP2ANA"; break; case 5: cp = "HTTPANA"; break; } printf( "%-8s", cp); }
# 52 "shmmb_print.c"
#define PRINT_ONOFF(a) printf("%d(%-3s)", a, (a == 0)?"OFF":"ON");

#define PRINT_FLAG(a) printf( "%c(%-11s)", a, (a == 'S')?"SOURCE":"DESTINATION");
#define PRINT_IPTYPE(a) printf( "%d(%-7s)", a, (a == IPTYPE_IPPOOL)?"IP POOL":"PDSN");
#define PRINT_PROTOCOL(a) { if(a == 6) printf( "%d(%-7s)", a, "TCP"); else if(a == 17) printf( "%d(%-7s)", a, "UDP"); else printf( "%d(%-7s)", a, "UNKNOWN"); }
# 66 "shmmb_print.c"
#define PRINT_LAYER(a) { if(a == 1) printf( "%d(%-7s)", a, "IP"); else if(a == 2) printf( "%d(%-7s)", a, "TCP"); else if(a == 3) printf( "%d(%-7s)", a, "UDP"); else if(a == 21) printf( "%d(%-7s)", a, "DEFAULT"); else printf( "%d(%-7s)", a, "UNKNOWN"); }
# 80 "shmmb_print.c"
#define PRINT_DESTIP(a,b) { printf( "%3d", b); printf(" "); PRINT_IP(a ##.key.uiIP) printf(" "); PRINT_FLAG(a ##.key.ucFlag) printf(" "); PRINT_IP(a ##.uiNetmask) printf("  "); PRINT_IPTYPE(a ##.ucIPType) printf("\n"); }
# 94 "shmmb_print.c"
#define PRINT_DESTIP1(a,b) { printf( "%3d", b); printf(" "); PRINT_IP(a ##.key.uiIP) printf(" "); PRINT_FLAG(a ##.key.ucFlag) printf(" "); PRINT_IP(a ##.uiNetmask) printf("  "); PRINT_LAYER(a ##.ucLayer); printf(" "); PRINT_ONOFF(a ##.ucFilterOut); printf("\n"); }
# 110 "shmmb_print.c"
#define PRINT_DESTPORT(a,b) { printf( "%3d", b); printf(" "); PRINT_PROTOCOL(a ##.key.ucProtocol) printf(" "); PRINT_IP(a ##.key.uiDestIP) printf(" "); PRINT_PORT(a ##.key.usDestPort); printf("  "); PRINT_IP(a ##.uiNetmask) printf(" "); PRINT_CATEGORY(a ##.usCatID); printf("    "); PRINT_LAYER(a ##.ucLayer); printf(" "); PRINT_SVCBLOCK(a ##.ucSvcBlk); printf("  "); PRINT_ONOFF(a ##.ucFilterOut); printf( "\n"); }
# 132 "shmmb_print.c"
#define PRINT_SVCOPTION(a,b) { printf( "%3d", b); printf(" "); PRINT_SVCOPT(a ##.key.svcOpt) printf("     "); PRINT_SVCTYPE(a ##.svcType) printf("      "); PRINT_LAYER(a ##.layer) printf(" "); PRINT_ONOFF(a ##.fout) printf("  "); PRINT_SVCBLOCK(a ##.block) printf("   "); PRINT_ONOFF(a ##.urlCha) printf( "\n"); }
# 152 "shmmb_print.c"
int semid_destip=-1;
int semid_destport =-1;
int semid_svcopt =-1;

DESTPORT_TABLE *destport_tbl;
DESTIP_TABLE *destip_tbl;
SVCOPT_TABLE *svcopt_tbl;




void main()
{

    int key, shmId, count;

        st_destip DestIP, *pDestIP;
        st_destip_key DestIPKey1, DestIPKey2;

        st_destport DestPort, *pDestPort;
        st_destport_key DestPortKey1, DestPortKey2;

    st_svcopt SvcOpt, *pSvcOpt;
    st_svcopt_key SvcOptKey1, SvcOptKey2;


    key = strtol("0x2844",0,0);

    if ((shmId = (int)shmget (key, sizeof(DESTPORT_TABLE), 0644)) < 0) {
        if ((*__errno_location ()) != 2) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, (*__errno_location ()), strerror((*__errno_location ())));
            return ;
        }
    }

    if ((destport_tbl = (DESTPORT_TABLE*) shmat (shmId,0,0)) == (DESTPORT_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, (*__errno_location ()), strerror((*__errno_location ())));
        return ;
    }

    key = strtol("0x2843",0,0);

    if ((shmId = (int)shmget (key, sizeof(DESTIP_TABLE), 0644)) < 0) {
        if ((*__errno_location ()) != 2) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, (*__errno_location ()), strerror((*__errno_location ())));
            return ;
        }
    }

    if ((destip_tbl = (DESTIP_TABLE*) shmat (shmId,0,0)) == (DESTIP_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, (*__errno_location ()), strerror((*__errno_location ())));
        return ;
    }

    key = strtol("0x5121",0,0);

    if ((shmId = (int)shmget (key, sizeof(SVCOPT_TABLE), 0644)) < 0) {
        if ((*__errno_location ()) != 2) {
            fprintf (stderr,"[mmcr_init] shmget fail; key=0x%x, err=%d(%s)\n", key, (*__errno_location ()), strerror((*__errno_location ())));
            return ;
        }
    }

    if ((svcopt_tbl = (SVCOPT_TABLE*) shmat (shmId,0,0)) == (SVCOPT_TABLE*)-1) {
        fprintf (stderr,"[mmdr_init] shmat fail; key=0x%x, err=%d(%s)\n", key, (*__errno_location ()), strerror((*__errno_location ())));
        return ;
    }

        fprintf(stderr, "================================= DEST IP START ===================================\n");
    printf("NO. IP-Address      FLAG           SUBNET-MASK      IP-TYPE \n"); (__extension__ (__builtin_constant_p (sizeof(st_destip_key)) && (sizeof(st_destip_key)) <= 16 ? ((sizeof(st_destip_key)) == 1 ? ({ void *__s = (&DestIPKey1); *((__uint8_t *) __s) = (__uint8_t) 0x00; __s; }) : ({ void *__s = (&DestIPKey1); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0x00); switch ((unsigned int) (sizeof(st_destip_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0x00) && (0x00) == '\0' ? ({ void *__s = (&DestIPKey1); __builtin_memset (__s, '\0', sizeof(st_destip_key)); __s; }) : memset (&DestIPKey1, 0x00, sizeof(st_destip_key)))));

        DestIPKey1.ucFlag = 'S';

        (__extension__ (__builtin_constant_p (sizeof(st_destip_key)) && (sizeof(st_destip_key)) <= 16 ? ((sizeof(st_destip_key)) == 1 ? ({ void *__s = (&DestIPKey2); *((__uint8_t *) __s) = (__uint8_t) 0xff; __s; }) : ({ void *__s = (&DestIPKey2); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0xff); switch ((unsigned int) (sizeof(st_destip_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0xff) && (0xff) == '\0' ? ({ void *__s = (&DestIPKey2); __builtin_memset (__s, '\0', sizeof(st_destip_key)); __s; }) : memset (&DestIPKey2, 0xff, sizeof(st_destip_key)))));
        DestIPKey2.ucFlag = 'S';

        count = 0;
        while(1){
                pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
                if( pDestIP == 0 || pDestIP->key.ucFlag != 'S')
                        break;
        count++;
                memcpy( &DestIPKey1, &pDestIP->key, sizeof(st_destip_key) );
                memcpy( &DestIP, pDestIP, sizeof(st_destip) );
                { printf( "%3d", count); printf(" "); { struct in_addr addr; addr.s_addr = DestIP.key.uiIP; printf( "%-15s", inet_ntoa(addr)); } printf(" "); printf( "%c(%-11s)", DestIP.key.ucFlag, (DestIP.key.ucFlag == 'S')?"SOURCE":"DESTINATION"); printf(" "); { struct in_addr addr; addr.s_addr = DestIP.uiNetmask; printf( "%-15s", inet_ntoa(addr)); } printf("  "); printf( "%d(%-7s)", DestIP.ucIPType, (DestIP.ucIPType == 0)?"IP POOL":"PDSN"); printf("\n"); }
        }

        fprintf(stderr, "================================== DEST IP END ====================================\n");

        fprintf(stderr, "================================= DEST PORT(IP) START ===================================\n");
    printf("NO. IP-Address      FLAG           SUBNET-MASK      LAYER       FLT-OUT\n"); (__extension__ (__builtin_constant_p (sizeof(st_destip_key)) && (sizeof(st_destip_key)) <= 16 ? ((sizeof(st_destip_key)) == 1 ? ({ void *__s = (&DestIPKey1); *((__uint8_t *) __s) = (__uint8_t) 0x00; __s; }) : ({ void *__s = (&DestIPKey1); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0x00); switch ((unsigned int) (sizeof(st_destip_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0x00) && (0x00) == '\0' ? ({ void *__s = (&DestIPKey1); __builtin_memset (__s, '\0', sizeof(st_destip_key)); __s; }) : memset (&DestIPKey1, 0x00, sizeof(st_destip_key)))));

        DestIPKey1.ucFlag = 'D';

        (__extension__ (__builtin_constant_p (sizeof(st_destip_key)) && (sizeof(st_destip_key)) <= 16 ? ((sizeof(st_destip_key)) == 1 ? ({ void *__s = (&DestIPKey2); *((__uint8_t *) __s) = (__uint8_t) 0xff; __s; }) : ({ void *__s = (&DestIPKey2); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0xff); switch ((unsigned int) (sizeof(st_destip_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0xff) && (0xff) == '\0' ? ({ void *__s = (&DestIPKey2); __builtin_memset (__s, '\0', sizeof(st_destip_key)); __s; }) : memset (&DestIPKey2, 0xff, sizeof(st_destip_key)))));
        DestIPKey2.ucFlag = 'D';

        count = 0;
        while(1){
                pDestIP = Select_DESTIP( &DestIPKey1, &DestIPKey2 );
                if( pDestIP == 0 || pDestIP->key.ucFlag != 'D')
                        break;
        count++;
                memcpy( &DestIPKey1, &pDestIP->key, sizeof(st_destip_key) );
                memcpy( &DestIP, pDestIP, sizeof(st_destip) );
                { printf( "%3d", count); printf(" "); { struct in_addr addr; addr.s_addr = DestIP.key.uiIP; printf( "%-15s", inet_ntoa(addr)); } printf(" "); printf( "%c(%-11s)", DestIP.key.ucFlag, (DestIP.key.ucFlag == 'S')?"SOURCE":"DESTINATION"); printf(" "); { struct in_addr addr; addr.s_addr = DestIP.uiNetmask; printf( "%-15s", inet_ntoa(addr)); } printf("  "); { if(DestIP.ucLayer == 1) printf( "%d(%-7s)", DestIP.ucLayer, "IP"); else if(DestIP.ucLayer == 2) printf( "%d(%-7s)", DestIP.ucLayer, "TCP"); else if(DestIP.ucLayer == 3) printf( "%d(%-7s)", DestIP.ucLayer, "UDP"); else if(DestIP.ucLayer == 21) printf( "%d(%-7s)", DestIP.ucLayer, "DEFAULT"); else printf( "%d(%-7s)", DestIP.ucLayer, "UNKNOWN"); }; printf(" "); printf("%d(%-3s)", DestIP.ucFilterOut, (DestIP.ucFilterOut == 0)?"OFF":"ON");; printf("\n"); }
        }

        fprintf(stderr, "=============================== DEST PORT(IP) END =================================\n");

        fprintf(stderr, "============================== DEST PORT(TCP/UDP) START ================================\n");
    printf("NO. PROTOCOL   IP-Address       PORT  SUBNET-MASK     CATEGORY-ID LAYER      SVC-BLOCK FLT-OUT\n"); (__extension__ (__builtin_constant_p (sizeof(st_destport_key)) && (sizeof(st_destport_key)) <= 16 ? ((sizeof(st_destport_key)) == 1 ? ({ void *__s = (&DestPortKey1); *((__uint8_t *) __s) = (__uint8_t) 0x00; __s; }) : ({ void *__s = (&DestPortKey1); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0x00); switch ((unsigned int) (sizeof(st_destport_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0x00) && (0x00) == '\0' ? ({ void *__s = (&DestPortKey1); __builtin_memset (__s, '\0', sizeof(st_destport_key)); __s; }) : memset (&DestPortKey1, 0x00, sizeof(st_destport_key)))));

        (__extension__ (__builtin_constant_p (sizeof(st_destport_key)) && (sizeof(st_destport_key)) <= 16 ? ((sizeof(st_destport_key)) == 1 ? ({ void *__s = (&DestPortKey2); *((__uint8_t *) __s) = (__uint8_t) 0xff; __s; }) : ({ void *__s = (&DestPortKey2); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0xff); switch ((unsigned int) (sizeof(st_destport_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0xff) && (0xff) == '\0' ? ({ void *__s = (&DestPortKey2); __builtin_memset (__s, '\0', sizeof(st_destport_key)); __s; }) : memset (&DestPortKey2, 0xff, sizeof(st_destport_key)))));

        count = 0;
        while(1){
                pDestPort = Select_DESTPORT( &DestPortKey1, &DestPortKey2 );
        if(pDestPort == 0)
            break;
        count++;
                memcpy( &DestPortKey1, &pDestPort->key, sizeof(st_destport_key) );
                memcpy( &DestPort, pDestPort, sizeof(st_destport) );
                { printf( "%3d", count); printf(" "); { if(DestPort.key.ucProtocol == 6) printf( "%d(%-7s)", DestPort.key.ucProtocol, "TCP"); else if(DestPort.key.ucProtocol == 17) printf( "%d(%-7s)", DestPort.key.ucProtocol, "UDP"); else printf( "%d(%-7s)", DestPort.key.ucProtocol, "UNKNOWN"); } printf(" "); { struct in_addr addr; addr.s_addr = DestPort.key.uiDestIP; printf( "%-15s", inet_ntoa(addr)); } printf(" "); printf( "%5d", DestPort.key.usDestPort);; printf("  "); { struct in_addr addr; addr.s_addr = DestPort.uiNetmask; printf( "%-15s", inet_ntoa(addr)); } printf(" "); printf( "%8d", DestPort.usCatID);; printf("    "); { if(DestPort.ucLayer == 1) printf( "%d(%-7s)", DestPort.ucLayer, "IP"); else if(DestPort.ucLayer == 2) printf( "%d(%-7s)", DestPort.ucLayer, "TCP"); else if(DestPort.ucLayer == 3) printf( "%d(%-7s)", DestPort.ucLayer, "UDP"); else if(DestPort.ucLayer == 21) printf( "%d(%-7s)", DestPort.ucLayer, "DEFAULT"); else printf( "%d(%-7s)", DestPort.ucLayer, "UNKNOWN"); }; printf(" "); { char *cp = ((void *)0); switch(DestPort.ucSvcBlk){ case 1: cp = "CDR"; break; case 2: cp = "WAP1ANA"; break; case 3: cp = "UAWAPANA"; break; case 4: cp = "WAP2ANA"; break; case 5: cp = "HTTPANA"; break; } printf( "%-8s", cp); }; printf("  "); printf("%d(%-3s)", DestPort.ucFilterOut, (DestPort.ucFilterOut == 0)?"OFF":"ON");; printf( "\n"); }
        }

        fprintf(stderr, "=============================== DEST PORT(TCP/UDP) END =================================\n");

        fprintf(stderr, "============================== SERVICE OPTION START ================================\n");
    printf("NO. SVC-OPT   SVC-TYPE   LAYER      F-OUT   SVC-BLOCK   URL-CHA   \n");
        (__extension__ (__builtin_constant_p (sizeof(st_svcopt_key)) && (sizeof(st_svcopt_key)) <= 16 ? ((sizeof(st_svcopt_key)) == 1 ? ({ void *__s = (&SvcOptKey1); *((__uint8_t *) __s) = (__uint8_t) 0x00; __s; }) : ({ void *__s = (&SvcOptKey1); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0x00); switch ((unsigned int) (sizeof(st_svcopt_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0x00) && (0x00) == '\0' ? ({ void *__s = (&SvcOptKey1); __builtin_memset (__s, '\0', sizeof(st_svcopt_key)); __s; }) : memset (&SvcOptKey1, 0x00, sizeof(st_svcopt_key)))));
        (__extension__ (__builtin_constant_p (sizeof(st_svcopt_key)) && (sizeof(st_svcopt_key)) <= 16 ? ((sizeof(st_svcopt_key)) == 1 ? ({ void *__s = (&SvcOptKey2); *((__uint8_t *) __s) = (__uint8_t) 0xff; __s; }) : ({ void *__s = (&SvcOptKey2); union { unsigned int __ui; unsigned short int __usi; unsigned char __uc; } *__u = __s; __uint8_t __c = (__uint8_t) (0xff); switch ((unsigned int) (sizeof(st_svcopt_key))) { case 15: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 11: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 7: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 3: __u->__usi = (unsigned short int) __c * 0x0101; __u = __extension__ ((void *) __u + 2); __u->__uc = (unsigned char) __c; break; case 14: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 10: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 6: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 2: __u->__usi = (unsigned short int) __c * 0x0101; break; case 13: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 9: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 5: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 1: __u->__uc = (unsigned char) __c; break; case 16: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 12: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 8: __u->__ui = __c * 0x01010101; __u = __extension__ ((void *) __u + 4); case 4: __u->__ui = __c * 0x01010101; case 0: break; } __s; })) : (__builtin_constant_p (0xff) && (0xff) == '\0' ? ({ void *__s = (&SvcOptKey2); __builtin_memset (__s, '\0', sizeof(st_svcopt_key)); __s; }) : memset (&SvcOptKey2, 0xff, sizeof(st_svcopt_key)))));

        count = 0;
        while(1){
                pSvcOpt = Select_SVCOPT( &SvcOptKey1, &SvcOptKey2 );
        if(pSvcOpt == 0)
            break;
        count++;
                memcpy( &SvcOptKey1, &pSvcOpt->key, sizeof(st_svcopt_key) );
                memcpy( &SvcOpt, pSvcOpt, sizeof(st_svcopt) );
                { printf( "%3d", count); printf(" "); printf( "%5d", SvcOpt.key.svcOpt); printf("     "); printf( "%5d", SvcOpt.svcType); printf("      "); { if(SvcOpt.layer == 1) printf( "%d(%-7s)", SvcOpt.layer, "IP"); else if(SvcOpt.layer == 2) printf( "%d(%-7s)", SvcOpt.layer, "TCP"); else if(SvcOpt.layer == 3) printf( "%d(%-7s)", SvcOpt.layer, "UDP"); else if(SvcOpt.layer == 21) printf( "%d(%-7s)", SvcOpt.layer, "DEFAULT"); else printf( "%d(%-7s)", SvcOpt.layer, "UNKNOWN"); } printf(" "); printf("%d(%-3s)", SvcOpt.fout, (SvcOpt.fout == 0)?"OFF":"ON"); printf("  "); { char *cp = ((void *)0); switch(SvcOpt.block){ case 1: cp = "CDR"; break; case 2: cp = "WAP1ANA"; break; case 3: cp = "UAWAPANA"; break; case 4: cp = "WAP2ANA"; break; case 5: cp = "HTTPANA"; break; } printf( "%-8s", cp); } printf("   "); printf("%d(%-3s)", SvcOpt.urlCha, (SvcOpt.urlCha == 0)?"OFF":"ON"); printf( "\n"); }
    }
        fprintf(stderr, "=============================== SERVICE OPTION END =================================\n");

    return;

}
