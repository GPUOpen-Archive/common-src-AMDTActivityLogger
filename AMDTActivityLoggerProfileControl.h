//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the AMDTActivityLogger Profile Control singleton
//==============================================================================

#ifndef _AMDT_ACTIVITY_LOGGER_PROFILE_CONTROL_H_
#define _AMDT_ACTIVITY_LOGGER_PROFILE_CONTROL_H_

#include "TSingleton.h"

#include <AMDTOSWrappers/Include/osModule.h>

#include "CXLActivityLogger.h"


/// function typedef for the profiling control functions exported from the agent libraries
typedef void(*ProfilingControlProc)();

/// function typedef for the profiling control functions exported from the agent libraries -- this version passes the mode being started/stopped
typedef void(*ProfilingControlProcWithMode)(amdtProfilingControlMode);

/// Singleton class to interact with agents to stop/resume profiling
class AMDTActivityLoggerProfileControl : public TSingleton <AMDTActivityLoggerProfileControl>
{
public:
    /// Tell the profiler to stop profiling
    /// \param profilingControlMode the profiling mode being stopped
    /// \return AL_SUCCESS if at least one of the profiling agents is loaded and the the Stop Profiling entry point is called, AL_FAILED_TO_ATTACH_TO_PROFILER otherwise
    int StopProfiling(amdtProfilingControlMode profilingControlMode);

    /// Tell the profiler to resume profiling
    /// \param profilingControlMode the profiling mode being resumed
    /// \return AL_SUCCESS if at least one of the profiling agents is loaded and the the Resume Profiling entry point is called, AL_FAILED_TO_ATTACH_TO_PROFILER otherwise
    int ResumeProfiling(amdtProfilingControlMode profilingControlMode);

private:
    /// Helper function to get the handle for the specified profiler library
    /// \param pBaseName the base name of the profiler lib whose handle is needed
    /// \param[out] libHandle the handle of the library, if it is already loaded in the current process
    /// \return true if the profiler lib was loaded in the current process
    bool GetHandleForProfilerLib(const wchar_t* pBaseName, osModuleHandle& libHandle);

    /// Helper function to initialize lib handles and function pointers, and to call the specified function pointer
    /// \param[in,out] libHandle the handle of the library to initialize the entry point from
    /// \param[in] pLibName the name of the library to initialize the entry point from
    /// \param[in,out] profilingControlProc the function pointer for the entry point
    /// \param[in] pProcName the name of the entry point
    /// \return true on success, false otherwise
    bool CallProfileControlEntryPointFromLibrary(osModuleHandle& libHandle, const wchar_t* pLibName, ProfilingControlProc& profilingControlProc, const char* pProcName);

    /// Helper function to initialize lib handles and function pointers, and to call the specified function pointer -- this method passes the mode param to the agent
    /// \param[in,out] libHandle the handle of the library to initialize the entry point from
    /// \param[in] pLibName the name of the library to initialize the entry point from
    /// \param[in,out] profilingControlProc the function pointer for the entry point
    /// \param[in] pProcName the name of the entry point
    /// \param[in] mode the profiling mode being stopped or resumed
    /// \return true on success, false otherwise
    bool CallProfileControlEntryPointFromLibraryWithMode(osModuleHandle& libHandle, const wchar_t* pLibName, ProfilingControlProcWithMode& profilingControlProc, const char* pProcName, amdtProfilingControlMode mode);

    osModuleHandle           m_clTraceAgentHandle = nullptr;                  ///< handle to the CL Tracing Agent module
    osModuleHandle           m_hsaTraceAgentHandle = nullptr;                 ///< handle to the HSA Tracing Agent module
    osModuleHandle           m_clProfilingAgentHandle = nullptr;              ///< handle to the CL Profiling Agent module
    osModuleHandle           m_hsaProfilingAgentHandle = nullptr;             ///< handle to the HSA Profiling Agent module
    osModuleHandle           m_clOccupancyAgentHandle = nullptr;              ///< handle to the CL Occupancy Agent module

    ProfilingControlProc m_pCLTraceStopProfilingProc = nullptr;               ///< Pointer to the CL StopTracing entry point
    ProfilingControlProc m_pCLTraceResumeProfilingProc = nullptr;             ///< Pointer to the CL ResumeTracing entry point
    ProfilingControlProc m_pCLPerfCounterStopProfilingProc = nullptr;         ///< Pointer to the CL StopProfiling entry point
    ProfilingControlProc m_pCLPerfCounterResumeProfilingProc = nullptr;       ///< Pointer to the CL ResumeProfiling entry point

    ProfilingControlProc m_pHSATraceStopProfilingProc = nullptr;              ///< Pointer to the HSA StopTracing entry point
    ProfilingControlProc m_pHSATraceResumeProfilingProc = nullptr;            ///< Pointer to the HSA ResumeTracing entry point
    ProfilingControlProc m_pHSAPerfCounterStopProfilingProc = nullptr;        ///< Pointer to the HSA StopProfiling entry point
    ProfilingControlProc m_pHSAPerfCounterResumeProfilingProc = nullptr;      ///< Pointer to the HSA ResumeProfiling entry point

    ProfilingControlProcWithMode m_pCLOccupancyStopProfilingProc = nullptr;   ///< Pointer to the CL Occupancy StopProfiling entry point
    ProfilingControlProcWithMode m_pCLOccupancyResumeProfilingProc = nullptr; ///< Pointer to the CL Occupancy ResumeProfiling entry point
};

#endif // _AMDT_ACTIVITY_LOGGER_PROFILE_CONTROL_H_
