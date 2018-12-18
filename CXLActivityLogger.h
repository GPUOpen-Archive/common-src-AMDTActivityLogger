//=============================================================================
//
// Author: AMD Developer Tools
//         AMD, Inc.
//
// This file is the only header that must be included by an application that
// wishes to use AMDT Activity Logger. It defines all the available entrypoints.
//=============================================================================
// Copyright (c) 2013 - 2015 Advanced Micro Devices, Inc.  All rights reserved.
//=============================================================================

#ifndef _CXL_ACTIVITY_LOGGER_H_
#define _CXL_ACTIVITY_LOGGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define AL_SUCCESS                             0
#define AL_UNINITIALIZED_ACTIVITY_LOGGER      -1
#define AL_FINALIZED_ACTIVITY_LOGGER          -2
#define AL_UNBALANCED_MARKER                  -3
#define AL_APP_PROFILER_NOT_DETECTED          -4 // left here for backward compatibility
#define AL_CODEXL_GPU_PROFILER_NOT_DETECTED   -4 // left here for backward compatibility
#define AL_GPU_PROFILER_NOT_DETECTED          -4
#define AL_NULL_MARKER_NAME                   -5
#define AL_INTERNAL_ERROR                     -6
#define AL_OUT_OF_MEMORY                      -7
#define AL_FAILED_TO_OPEN_OUTPUT_FILE         -8
#define AL_FAILED_TO_ATTACH_TO_PROFILER       -9
#define AL_WARN_PROFILE_ALREADY_RESUMED       -10
#define AL_WARN_PROFILE_ALREADY_PAUSED        -11
#define AL_GPU_PROFILER_MISMATCH              -12

#if defined(_WIN32) || defined(__CYGWIN__)
#define AL_API_CALL __stdcall
#else
#define AL_API_CALL __attribute__((visibility("default")))
#endif

#ifdef __GNUC__
#define AL_DEPRECATED_PREFIX
#define AL_DEPRECATED_SUFFIX __attribute__((deprecated))
#elif _WIN32
#define AL_DEPRECATED_PREFIX __declspec(deprecated)
#define AL_DEPRECATED_SUFFIX
#else
#define AL_DEPRECATED_PREFIX
#define AL_DEPRECATED_SUFFIX
#endif

//-----------------------------------------------------------------------------
/// AMDTActivityLogger allows you to annotate the timeline with additional
/// application-defined information. Markers created in different threads will
/// be displayed in different rows in the Timeline. Hierarchical markers can be
/// created by nesting amdtBeginMarker calls.
//-----------------------------------------------------------------------------

/// Initialize AMDTActivityLogger.
/// \return status code
extern int AL_API_CALL amdtInitializeActivityLogger();

/// Begin AMDTActivityLogger block
/// Nested calls to amdtBeginMarker will result in markers showing in a hierarchical way.
/// \param szMarkerName Marker name
/// \param szGroupName Group name, Optional, Pass in NULL to use default group name
///        If group name is specified, additional sub-branch will be created under PerfMarker branch
///        in Timeline and all markers that belong to the group will be displayed in the group branch.
/// \param szUserString Currently ignored -- reserved for future use (see below for one expected future use)
///        User string, Optional, Pass in NULL to use default color and no user specific string.
///        If User string is specified it should be formatted as a XML string. Optional tag is Color, as in the following example
///        amdtBeginMarker("MyMarker", "MyGroup", "<Color Hex="FF0000"/>\n<UserComment Comment="Starting major calulcation"/>")
/// \return status code
extern int AL_API_CALL amdtBeginMarker(const char* szMarkerName, const char* szGroupName, const char* szUserString);

/// End AMDTActivityLogger block
/// \return status code
extern int AL_API_CALL amdtEndMarker();

/// End AMDTActivityLogger block
/// \param szMarkerName Marker name -- the name will replace the name passed in to amdtBeginMarker
/// \param szGroupName Group name, Optional, Pass in NULL to use default group name
///        If group name is specified, additional sub-branch will be created under PerfMarker branch
///        in Timeline and all markers that belong to the group will be displayed in the group branch.
/// \param szUserString Currently ignored -- reserved for future use (see amdtBeginMarker for more info)
/// \return status code -- it is not valid to pass in a non-empty szGroupName with an empty szMarkerName
extern int AL_API_CALL amdtEndMarkerEx(const char* szMarkerName, const char* szGroupName, const char* szUserString);

/// Finalize AMDTActivityLogger, Save collected data in specified output file.
/// Failed to call the function will result in no AMDTActivityLogger file is generated.
/// \return status code
extern int AL_API_CALL amdtFinalizeActivityLogger();

/// Enum to define the profiling modes that can be stopped or resumed
typedef enum
{
    /// Application Timeline Trace Profiling
    AMDT_TRACE_PROFILING = 0x1,

    /// Performance Counter Profiling
    AMDT_PERF_COUNTER_PROFILING = 0x2,

    /// CPU Profiling
    AMDT_CPU_PROFILING = 0x4,

    /// All profiling modes
    AMDT_ALL_PROFILING = AMDT_TRACE_PROFILING | AMDT_PERF_COUNTER_PROFILING | AMDT_CPU_PROFILING
} amdtProfilingControlMode;

/// Instruct profiler to stop profiling. Profiling can be resumed using amdtResumeProfiling.
/// \param profilingControlMode the profiling mode (or modes) to stop
/// \return status code
extern int AL_API_CALL amdtStopProfiling(amdtProfilingControlMode profilingControlMode);

/// Instruct profiler to resume profiling. Profiling can be stopped using amdtStopProfiling.
/// By default the CodeXL GPU Profiler starts profiling an application from the first API call (or
/// first kernel dispatch in the case of performance counter profiling). To tell the profiler to
/// disable profiling at the start of the application you can start rcprof with the --startdisabled flag.
/// \param profilingControlMode the profiling mode (or modes) to resume
/// \return status code
extern int AL_API_CALL amdtResumeProfiling(amdtProfilingControlMode profilingControlMode);

/// Instruct profiler to stop profiling. Profiling can be resumed using amdtResumeProfilingEx.
/// \return status code
extern int AL_API_CALL amdtStopProfilingEx(void);

/// Instruct profiler to resume profiling. Profiling can be stopped using amdtStopProfilingEx.
/// \return status code
extern int AL_API_CALL amdtResumeProfilingEx(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/// A utility class that opens a marker in the constructor and closes it in the destructor
/// This saves the user the need to explicitly call amdtEndMarker and also handles user code
/// with exceptions and multiple exit points correctly.
class amdtScopedMarker
{
public:
    amdtScopedMarker(const char* szMarkerName, const char* szGroupName, const char* szUserString)
    {
        amdtBeginMarker(szMarkerName, szGroupName, szUserString);
    }

    amdtScopedMarker(const char* szMarkerName, const char* szGroupName)
    {
        amdtBeginMarker(szMarkerName, szGroupName, nullptr);
    }

    ~amdtScopedMarker()
    {
        amdtEndMarker();
    }
};
#endif

#endif // _CXL_ACTIVITY_LOGGER_H_
