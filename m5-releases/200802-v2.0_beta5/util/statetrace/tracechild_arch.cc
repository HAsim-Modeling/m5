#if defined __alpha__
        #error "Alpha architecture not implemented"
#elif defined __amd64__
//        #error "AMD64 architecture not implemented"
        #include "arch/tracechild_amd64.cc"
#elif defined __hppa__
        #error "Hppa architecture not implemented"
#elif defined __i386__ || defined __i486__ || \
                defined __i586__ || defined __i686
        #include "arch/tracechild_i386.cc"
#elif defined __ia64__
        #error "IA64 architecture not implemented"
#elif defined __mips__
        #error "Mips architecture not implemented"
#elif defined __powerpc__
        #error "PowerPC architecture not implemented"
#elif defined __sparc__
        #include "arch/tracechild_sparc.cc"
#elif defined __sh__
        #include "SuperH architecture not implemented"
#elif defined __s390__
        #include "System/390 architecture not implemented"
#else
        #error "Couldn't determine architecture"
#endif
