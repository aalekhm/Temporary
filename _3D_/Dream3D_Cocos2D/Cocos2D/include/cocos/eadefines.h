#ifndef __EADEFINES_H__
#define __EADEFINES_H__

    //#include "EABase/config/eacompilertraits.h"

	#if defined(_WIN32)
		#undef  EA_PLATFORM_WINDOWS
		#define EA_PLATFORM_WINDOWS 1
	#endif

    #ifndef CC_DISABLE_VC_WARNING
        #if defined(_MSC_VER)
            #define CC_DISABLE_VC_WARNING(w)  \
                __pragma(warning(disable:w))
        #else
            #define CC_DISABLE_VC_WARNING(w)
        #endif
    #endif

#endif //__EADEFINES_H__
