//==============================================================================
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Singleton class to retrieve CPU timestamps in nanoseconds.
///        This matches the timestamps used by the GPU Profiler.
//==============================================================================

#ifndef _AMDT_ACTIVITY_LOGGER_TIME_STAMP_H_
#define _AMDT_ACTIVITY_LOGGER_TIME_STAMP_H_

#include "AMDTBaseTools/Include/AMDTDefinitions.h"

#include "TSingleton.h"

/// Singleton class to retrieve CPU timestamps in nanoseconds
class AMDTActivityLoggerTimeStamp : public TSingleton<AMDTActivityLoggerTimeStamp>
{
public:
    /// Constructor
    AMDTActivityLoggerTimeStamp();

    /// Get the current CPU timestamp in nanoseconds
    /// \return current CPU timestamp in nanoseconds
    unsigned long long GetTimeNanos();

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
private:
    double m_invFrequency; ///< the inverse of the CPU frequency
#endif
};

#endif // _AMDT_ACTIVITY_LOGGER_TIME_STAMP_H_
