//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the AMDTActivityLogger Profile Control singleton
//==============================================================================

#include <AMDTOSWrappers/Include/osFilePath.h>

#include "AMDTGPUProfilerDefs.h"
#include "AMDTActivityLoggerProfileControl.h"

bool AMDTActivityLoggerProfileControl::GetHandleForProfilerLib(const wchar_t* pBaseName, osModuleHandle& libHandle)
{

    bool hasHandle = false;
    gtString moduleName;
    osFilePath moduleFilePath;

    //first try unadorned lib name
    moduleName = AL_LIB_PREFIX;
    moduleName.append(AL_RCP_PREFIX);
    moduleName.append(pBaseName);
    moduleName.append(AL_PLATFORM_SUFFIX);
    moduleName.append(AL_LIB_SUFFIX);
    moduleFilePath.setFileName(moduleName);
    hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);

    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_CXLGPUPROFILER_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    // try debug builds
    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_RCP_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_DEBUG_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_CXLGPUPROFILER_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_DEBUG_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    // try internal builds
    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_RCP_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_INTERNAL_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_CXLGPUPROFILER_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_INTERNAL_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    // try internal debug builds
    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_RCP_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_DEBUG_SUFFIX);
        moduleName.append(AL_INTERNAL_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    if (!hasHandle)
    {
        moduleName = AL_LIB_PREFIX;
        moduleName.append(AL_CXLGPUPROFILER_PREFIX);
        moduleName.append(pBaseName);
        moduleName.append(AL_PLATFORM_SUFFIX);
        moduleName.append(AL_DEBUG_SUFFIX);
        moduleName.append(AL_INTERNAL_SUFFIX);
        moduleName.append(AL_LIB_SUFFIX);
        moduleFilePath.setFileName(moduleName);
        hasHandle = osGetLoadedModuleHandle(moduleFilePath, libHandle);
    }

    return hasHandle;
}

bool AMDTActivityLoggerProfileControl::CallProfileControlEntryPointFromLibrary(osModuleHandle& libHandle, const wchar_t* pLibName, ProfilingControlProc& profilingControlProc, const char* pProcName)
{
    bool retVal = false;
    bool hasHandle = true;

    if (nullptr == libHandle)
    {
        hasHandle = GetHandleForProfilerLib(pLibName, libHandle);
    }

    if (hasHandle && nullptr != libHandle)
    {
        bool procFound = true;

        if (nullptr == profilingControlProc)
        {
            osProcedureAddress procAddress;
            procFound = osGetProcedureAddress(libHandle, pProcName, procAddress);

            if (procFound)
            {
                profilingControlProc = reinterpret_cast<ProfilingControlProc>(procAddress);
            }
        }

        if (nullptr != profilingControlProc)
        {
            profilingControlProc();
            retVal = true;
        }
    }

    return retVal;
}

bool AMDTActivityLoggerProfileControl::CallProfileControlEntryPointFromLibraryWithMode(osModuleHandle& libHandle, const wchar_t* pLibName, ProfilingControlProcWithMode& profilingControlProc, const char* pProcName, amdtProfilingControlMode mode)
{
    bool retVal = false;
    bool hasHandle = true;

    if (nullptr == libHandle)
    {
        hasHandle = GetHandleForProfilerLib(pLibName, libHandle);
    }

    if (hasHandle && nullptr != libHandle)
    {
        bool procFound = true;

        if (nullptr == profilingControlProc)
        {
            osProcedureAddress procAddress;
            procFound = osGetProcedureAddress(libHandle, pProcName, procAddress);

            if (procFound)
            {
                profilingControlProc = reinterpret_cast<ProfilingControlProcWithMode>(procAddress);
            }
        }

        if (nullptr != profilingControlProc)
        {
            profilingControlProc(mode);
            retVal = true;
        }
    }

    return retVal;
}

int AMDTActivityLoggerProfileControl::StopProfiling(amdtProfilingControlMode profilingControlMode)
{
    int retVal = AL_FAILED_TO_ATTACH_TO_PROFILER;
    bool procCalled = false;

    if ((profilingControlMode & AMDT_TRACE_PROFILING) == AMDT_TRACE_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clTraceAgentHandle, AL_CL_TRACE_AGENT_DLL, m_pCLTraceStopProfilingProc, "amdtCodeXLStopProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaTraceAgentHandle, AL_HSA_TRACE_AGENT_DLL, m_pHSATraceStopProfilingProc, "amdtCodeXLStopProfiling");
    }

    if ((profilingControlMode & AMDT_PERF_COUNTER_PROFILING) == AMDT_PERF_COUNTER_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clProfilingAgentHandle, AL_CL_PROFILE_AGENT_DLL, m_pCLPerfCounterStopProfilingProc, "amdtCodeXLStopProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaProfilingAgentHandle, AL_HSA_PROFILE_AGENT_DLL, m_pHSAPerfCounterStopProfilingProc, "amdtCodeXLStopProfiling");
    }

    procCalled |= CallProfileControlEntryPointFromLibraryWithMode(m_clOccupancyAgentHandle, AL_CL_OCCUPANCY_AGENT_DLL, m_pCLOccupancyStopProfilingProc, "amdtCodeXLStopProfiling", profilingControlMode);

    if (procCalled)
    {
        retVal = AL_SUCCESS;
    }

    return retVal;
}

int AMDTActivityLoggerProfileControl::ResumeProfiling(amdtProfilingControlMode profilingControlMode)
{
    int retVal = AL_FAILED_TO_ATTACH_TO_PROFILER;
    bool procCalled = false;

    if ((profilingControlMode & AMDT_TRACE_PROFILING) == AMDT_TRACE_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clTraceAgentHandle, AL_CL_TRACE_AGENT_DLL, m_pCLTraceResumeProfilingProc, "amdtCodeXLResumeProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaTraceAgentHandle, AL_HSA_TRACE_AGENT_DLL, m_pHSATraceResumeProfilingProc, "amdtCodeXLResumeProfiling");
    }

    if ((profilingControlMode & AMDT_PERF_COUNTER_PROFILING) == AMDT_PERF_COUNTER_PROFILING)
    {
        procCalled = CallProfileControlEntryPointFromLibrary(m_clProfilingAgentHandle, AL_CL_PROFILE_AGENT_DLL, m_pCLPerfCounterResumeProfilingProc, "amdtCodeXLResumeProfiling");
        procCalled |= CallProfileControlEntryPointFromLibrary(m_hsaProfilingAgentHandle, AL_HSA_PROFILE_AGENT_DLL, m_pHSAPerfCounterResumeProfilingProc, "amdtCodeXLResumeProfiling");
    }

    procCalled |= CallProfileControlEntryPointFromLibraryWithMode(m_clOccupancyAgentHandle, AL_CL_OCCUPANCY_AGENT_DLL, m_pCLOccupancyResumeProfilingProc, "amdtCodeXLResumeProfiling", profilingControlMode);

    if (procCalled)
    {
        retVal = AL_SUCCESS;
    }

    return retVal;
}
