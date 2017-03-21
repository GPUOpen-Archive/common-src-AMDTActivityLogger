//==============================================================================
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Singleton class to retrieve CPU timestamps in nanoseconds
///        This matches the timestamps used by the GPU Profiler
//==============================================================================

#include "AMDTActivityLoggerTimeStamp.h"

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include "windows.h"
#endif

AMDTActivityLoggerTimeStamp::AMDTActivityLoggerTimeStamp()
{
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    m_invFrequency = 1.0 / freq.QuadPart * 1e9;
#endif
}

unsigned long long AMDTActivityLoggerTimeStamp::GetTimeNanos()
{
#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return static_cast<unsigned long long>(static_cast<double>(current.QuadPart) * m_invFrequency);
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);
    return static_cast<unsigned long long>(tp.tv_sec) * (1000ULL * 1000ULL * 1000ULL) +
        static_cast<unsigned long long>(tp.tv_nsec);
#endif
}
