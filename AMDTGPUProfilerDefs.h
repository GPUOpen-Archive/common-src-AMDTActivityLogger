//==============================================================================
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Definitions for interaction with the GPU Profiler
//==============================================================================

#ifndef _AMDT_GPU_PROFILER_DEFS_H_
#define _AMDT_GPU_PROFILER_DEFS_H_

#include "AMDTBaseTools/Include/AMDTDefinitions.h"

#define AL_CL_AGENT L"CL_AGENT"
#define AL_HSA_TOOLS_LIB L"HSA_TOOLS_LIB"

#ifdef _WIN32
#define AL_LIB_PREFIX L""
#define AL_LIB_SUFFIX L".dll"
#else
#define AL_LIB_PREFIX L"lib"
#define AL_LIB_SUFFIX L".so"
#endif

#define AL_DEBUG_SUFFIX L"-d"
#define AL_INTERNAL_SUFFIX L"-Internal"

#ifdef AMDT_PLATFORM_SUFFIX_W
    #define AL_PLATFORM_SUFFIX AMDT_PLATFORM_SUFFIX_W
#else
    #if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        #define AL_PLATFORM_SUFFIX L"32"
    #else
        #define AL_PLATFORM_SUFFIX L""
    #endif
#endif


#define AL_RCP_PREFIX L"RCP"
#define AL_CXLGPUPROFILER_PREFIX L"CodeXLGpuProfiler"

// the base name for the various GPU Profiler agents
#define AL_CL_TRACE_AGENT_DLL L"CLTraceAgent"
#define AL_CL_PROFILE_AGENT_DLL L"CLProfileAgent"
#define AL_CL_OCCUPANCY_AGENT_DLL L"CLOccupancyAgent"
#define AL_HSA_TRACE_AGENT_DLL L"HSATraceAgent"
#define AL_HSA_PROFILE_AGENT_DLL L"HSAProfileAgent"

#define AL_PERFMARKER_EXT L"amdtperfmarker"

#define AL_SPACE "&nbsp;"

#endif // _AMDT_GPU_PROFILER_DEFS_H_
