/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2014 Chukong Technologies Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#ifndef __CC_STD_C_H__
#define __CC_STD_C_H__

#include "platform/CCPlatformConfig.h"
#if defined(EA_PLATFORM_WINDOWS)
#include <BaseTsd.h>
#ifndef __SSIZE_T
#define __SSIZE_T
typedef SSIZE_T ssize_t;
#endif // __SSIZE_T
#endif
#include "platform/CCPlatformMacros.h"
#include <float.h>

#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_WINDOWS_PHONE)
// for math.h on win32 platform

#if !defined(_USE_MATH_DEFINES)
    #define _USE_MATH_DEFINES       // make M_PI can be use
#endif

#if !defined(isnan)
    #define isnan   _isnan
#endif

#if _MSC_VER < 1900
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#endif // EA WIN32

#include <math.h>
#include <string>
//#include <EASTL/sort.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef M_PI
  #define M_PI      3.14159265358
#endif
#ifndef M_PI_2
  #define M_PI_2    1.57079632679
#endif
// for MIN MAX and sys/time.h on win32 platform
//#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_WINDOWS_PHONE)
//#include <sys/time.h>
//#endif // EA WIN32

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef MIN
#define MIN(x,y) (((x) > (y)) ? (y) : (x))
#endif  // MIN

#ifndef MAX
#define MAX(x,y) (((x) < (y)) ? (y) : (x))
#endif  // MAX

#ifdef max
#undef max
#endif // max

//EAMT
#if defined(EA_PLATFORM_WINDOWS) || defined(EA_PLATFORM_WINDOWS_PHONE)
#if _MSC_VER >= 1600
    #include <stdint.h>
#else
    #include "./compat/stdint.h"
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
    // Structure timeval has define in winsock.h, include windows.h for it.
    //EAMT
    #pragma warning(push)
    #pragma warning(disable:4005 4548)
    #include <WinSock2.h> //to define timeval struct
    #pragma warning(pop)
#endif

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};
#endif

//#include <EAStdC/EADateTime.h>
#ifndef gettimeofday
    //#define gettimeofday EA::StdC::GetTimeOfDay
#endif

// Conflicted with cocos2d::MessageBox, so we need to undef it.
#ifdef MessageBox
#undef MessageBox
#endif

// Conflicted with ParticleSystem::PositionType::RELATIVE, so we need to undef it.
#ifdef RELATIVE
#undef RELATIVE
#endif

// Conflicted with CCBReader::SizeType::RELATIVE and CCBReader::ScaleType::RELATIVE, so we need to undef it.
#ifdef ABSOLUTE
#undef ABSOLUTE
#endif

// Conflicted with HttpRequest::Type::DELETE, so we need to undef it.
#ifdef DELETE
#undef DELETE
#endif


#endif