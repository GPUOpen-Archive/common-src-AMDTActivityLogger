//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Implementation of the AMDTActivityLogger lib
//==============================================================================

#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <sstream>
#include <mutex>

#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osFile.h>

#include "CXLActivityLogger.h"
#include "AMDTActivityLoggerProfileControl.h"
#include "AMDTGPUProfilerDefs.h"
#include "AMDTCpuProfileControl.h"
#include "AMDTActivityLoggerTimeStamp.h"

using namespace std;

#define INDENT "   "
#define DEFAULT_GROUP "Default"

/// Class to track a perf marker
class PerfMarkerItem
{
public:
    /// Constructor
    PerfMarkerItem()
    {
        m_pOstream = nullptr;
        m_depth = 0;
    }

    /// Destructor
    ~PerfMarkerItem()
    {
        delete m_pOstream;
        m_pOstream = nullptr;
    }

    ostream* m_pOstream; ///< output stream used to write the perf marker data
    int m_depth;         ///< depth of this perf marker

private:
    /// Disabled copy contructor
    PerfMarkerItem(const PerfMarkerItem& obj);

    /// Disabled assignment operator
    PerfMarkerItem& operator = (const PerfMarkerItem& obj);
};

std::mutex g_mtx;                                      ///< mutex to protect access to marker API
bool g_bInit = false;                                  ///< global flag indicating if the library has been initialized
bool g_bFinalized = false;                             ///< global flag indicating if the library has been finalized

bool g_isTimeoutMode = false;                          ///< global flag indicating if timeout mode is being used
string g_tempPerfMarkerFile;                           ///< name of the temp perf marker file
string g_perfFileName;                                 ///< name of the perf marker file
map<osThreadId, PerfMarkerItem*> g_perfMarkerItemMap;  ///< map from thread id to permarker items

/// ofstream descendant which specifies a file name
class ofstream_with_filename : public ofstream
{
public:
    /// Constructor
    ofstream_with_filename(const char* file)
        : ofstream(file)
    {
        m_fileName = file;
    }

    string m_fileName; ///< the filename
};

/// Gets the name of the temp file used to pass params betwee the GPU profiler and the ActivityLogger
/// \param[out] tempParamsFile the name of the params file
void GetTempActivityLoggerParamsFile(osFilePath& tempParamsFile)
{
#ifdef _WIN32
    tempParamsFile.setPath(osFilePath::OS_TEMP_DIRECTORY);
    tempParamsFile.setFileName(L"rcpdata");
    tempParamsFile.setFileExtension(AL_PERFMARKER_EXT);
#else //_LINUX || LINUX
    tempParamsFile.setPath(osFilePath::OS_USER_DOCUMENTS);
    tempParamsFile.setFileName(L".rcpdata");
    tempParamsFile.setFileExtension(AL_PERFMARKER_EXT);
#endif
}

/// Reads the temp params file and sets the global vars
/// \return true on success
bool GetParametersFromFile()
{
    bool retVal = false;
    osFilePath tempParamsFile;
    GetTempActivityLoggerParamsFile(tempParamsFile);
    osFile tempFile(tempParamsFile);
    retVal = tempFile.open(osChannel::OS_ASCII_TEXT_CHANNEL);

    if (retVal)
    {
        gtASCIIString line;
        bool timeoutParamFound = false;
        bool tempFileParamFound = false;
        bool outputFileParamFound = false;

        while (tempFile.readLine(line))
        {
            int equalPos = line.find("=");
            gtASCIIString paramName = line.substr(0, equalPos);
            gtASCIIString value = line.substr(equalPos + 1);

            if (paramName == "TimeOut")
            {
                timeoutParamFound = true;
                g_isTimeoutMode = value == "True";
            }
            else if (paramName == "PerfMarkerTempFileBaseName")
            {
                tempFileParamFound = true;
                g_tempPerfMarkerFile = value.asCharArray();
            }
            else if (paramName == "PerfMarkerOutputFileName")
            {
                outputFileParamFound = true;
                g_perfFileName = value.asCharArray();
            }
        }

        tempFile.close();
        retVal = timeoutParamFound && tempFileParamFound && outputFileParamFound;
    }

    return retVal;
}

/// Gets the current perf marker item
/// \param[out] ppItem the current perf marker item
/// \return the status code
int GetPerfMarkerItem(PerfMarkerItem** ppItem)
{
    if (ppItem == NULL)
    {
        return AL_INTERNAL_ERROR;
    }

    ostream* os = NULL;
    osThreadId tid = osGetUniqueCurrentThreadId();
    map<osThreadId, PerfMarkerItem*>::const_iterator it;
    it = g_perfMarkerItemMap.find(tid);

    if (it != g_perfMarkerItemMap.end())
    {
        *ppItem = it->second;
        return AL_SUCCESS;
    }
    else
    {
        if (g_isTimeoutMode)
        {
            stringstream ss;
            // Timeout mode, create a tmp file
            string path;
            osProcessId pid = osGetCurrentProcessId();

            path = g_tempPerfMarkerFile;
            ss << path << pid << "_" << tid << "." << AL_PERFMARKER_EXT;
            os = new(nothrow) ofstream_with_filename(ss.str().c_str());
        }
        else
        {
            os = new(nothrow) stringstream();
        }

        if (os == NULL)
        {
            return AL_OUT_OF_MEMORY;
        }

        PerfMarkerItem* pItem = new(nothrow) PerfMarkerItem();

        if (pItem == NULL)
        {
            delete os;
            return AL_OUT_OF_MEMORY;
        }

        pItem->m_depth = 0;
        pItem->m_pOstream = os;
        g_perfMarkerItemMap.insert(pair<osThreadId, PerfMarkerItem*>(tid, pItem));

        *ppItem = pItem;
        return AL_SUCCESS;
    }
}

extern "C"
int AL_API_CALL amdtInitializeActivityLogger()
{
    std::lock_guard<std::mutex> lock(g_mtx);

    if (g_bInit)
    {
        return AL_SUCCESS;
    }

    if (g_bFinalized)
    {
        return AL_FINALIZED_ACTIVITY_LOGGER;
    }


    gtString envVarValue;
    bool envVarFound = osGetCurrentProcessEnvVariableValue(AL_CL_AGENT, envVarValue);

    bool isTraceAgentFound = false;

    if (envVarFound && !envVarValue.isEmpty())
    {
        isTraceAgentFound = -1 != envVarValue.find(AL_CL_TRACE_AGENT_DLL);
    }

    if (!isTraceAgentFound)
    {
        envVarFound = osGetCurrentProcessEnvVariableValue(AL_HSA_TOOLS_LIB, envVarValue);

        if (envVarFound && !envVarValue.isEmpty())
        {
            isTraceAgentFound = -1 != envVarValue.find(AL_HSA_TRACE_AGENT_DLL);
        }
    }

    if (!isTraceAgentFound)
    {
        return AL_GPU_PROFILER_NOT_DETECTED;
    }

    g_bInit = true;

    if (!GetParametersFromFile())
    {
        return AL_GPU_PROFILER_MISMATCH;
    }

    return AL_SUCCESS;
}

const size_t s_DEFAULT_MARKER_NAME_WIDTH = 50; ///< default marker name width

extern "C"
int AL_API_CALL amdtBeginMarker(const char* szMarkerName, const char* szGroupName, const char* szUserString)
{
    // TODO: szUserString is currently unused. Need to use it.
    (void)(szUserString);

    std::lock_guard<std::mutex> lock(g_mtx);

    if (!g_bInit)
    {
        return AL_UNINITIALIZED_ACTIVITY_LOGGER;
    }

    if (g_bFinalized)
    {
        return AL_FINALIZED_ACTIVITY_LOGGER;
    }

    if (szMarkerName == NULL)
    {
        return AL_NULL_MARKER_NAME;
    }

    gtASCIIString strGroupName;

    if (szGroupName == NULL)
    {
        strGroupName = DEFAULT_GROUP;
    }
    else
    {
        strGroupName = szGroupName;
    }

    if (strGroupName.isEmpty())
    {
        strGroupName = DEFAULT_GROUP;
    }

    gtASCIIString strMarkerName(szMarkerName);

    PerfMarkerItem* pItem;
    int ret = GetPerfMarkerItem(&pItem);

    if (ret != AL_SUCCESS)
    {
        return ret;
    }

    strMarkerName.replace(" ", AL_SPACE);
    strGroupName.replace(" ", AL_SPACE);

    bool fit = static_cast<size_t>(strMarkerName.length()) < s_DEFAULT_MARKER_NAME_WIDTH;

    if (fit)
    {
        (*pItem->m_pOstream) << left << setw(20) << "clBeginPerfMarker" << left << setw(s_DEFAULT_MARKER_NAME_WIDTH) << strMarkerName.asCharArray() << setw(20) << AMDTActivityLoggerTimeStamp::Instance()->GetTimeNanos() << "   " << strGroupName.asCharArray() << endl;
    }
    else
    {
        // super long marker name
        (*pItem->m_pOstream) << "clBeginPerfMarker   " << strMarkerName.asCharArray() << "   " << AMDTActivityLoggerTimeStamp::Instance()->GetTimeNanos() << "   " << strGroupName.asCharArray() << endl;
    }

    pItem->m_depth++;

    return AL_SUCCESS;
}

extern "C"
int AL_API_CALL amdtEndMarker()
{
    return amdtEndMarkerEx("", "", "");
}

extern "C"
int AL_API_CALL amdtEndMarkerEx(const char* szMarkerName, const char* szGroupName, const char* szUserString)
{
    // TODO: szUserString is currently unused. Need to use it.
    (void)(szUserString);

    std::lock_guard<std::mutex> lock(g_mtx);

    if (!g_bInit)
    {
        return AL_UNINITIALIZED_ACTIVITY_LOGGER;
    }

    if (g_bFinalized)
    {
        return AL_FINALIZED_ACTIVITY_LOGGER;
    }

    if (szMarkerName == NULL)
    {
        return AL_NULL_MARKER_NAME;
    }

    string strGroupName(DEFAULT_GROUP);

    if (szGroupName != NULL)
    {
        strGroupName = szGroupName;

        if (strGroupName.empty())
        {
            strGroupName = DEFAULT_GROUP;
        }
    }

    string strMarkerName(szMarkerName);

    PerfMarkerItem* pItem;
    int ret = GetPerfMarkerItem(&pItem);

    if (ret != AL_SUCCESS)
    {
        return ret;
    }

    if (pItem->m_depth <= 0)
    {
        return AL_UNBALANCED_MARKER;
    }

    if (strMarkerName.empty() && strGroupName == DEFAULT_GROUP)
    {
        (*pItem->m_pOstream) << left << setw(20) << "clEndPerfMarker" << left << setw(20) << AMDTActivityLoggerTimeStamp::Instance()->GetTimeNanos() << endl;
    }
    else
    {
        /// the marker name must not be an empty string
        if (strMarkerName.empty())
        {
            return AL_NULL_MARKER_NAME;
        }

        bool fit = strMarkerName.length() < s_DEFAULT_MARKER_NAME_WIDTH;

        if (fit)
        {
            (*pItem->m_pOstream) << left << setw(20) << "clEndPerfMarkerEx" << setw(20) << AMDTActivityLoggerTimeStamp::Instance()->GetTimeNanos() << left << setw(s_DEFAULT_MARKER_NAME_WIDTH) << strMarkerName << "   " << strGroupName << endl;
        }
        else
        {
            // super long marker name
            (*pItem->m_pOstream) << "clEndPerfMarkerEx   " << setw(20) << AMDTActivityLoggerTimeStamp::Instance()->GetTimeNanos() << "   " << strMarkerName << "   " << strGroupName << endl;
        }
    }

    pItem->m_depth--;

    return AL_SUCCESS;
}

/// Helper function to count the number of newlines in a string
/// \param str the input string
/// \return the number of newlines in the string
int GetNumLines(const string& str)
{
    int ret = 0;

    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] == '\n')
        {
            ret++;
        }
    }

    return ret;
}


extern "C"
int AL_API_CALL amdtFinalizeActivityLogger()
{
    std::lock_guard<std::mutex> lock(g_mtx);

    if (g_bFinalized)
    {
        return AL_SUCCESS;
    }

    if (g_bInit)
    {

        ofstream fout;
        fout.open(g_perfFileName.c_str());

        if (!fout.fail())
        {
            // write header
            fout << "=====Perfmarker Output=====\n";

            for (map<osThreadId, PerfMarkerItem*>::iterator it = g_perfMarkerItemMap.begin(); it != g_perfMarkerItemMap.end(); ++it)
            {
                string content;
                // thread ID
                fout << it->first << endl;

                if (it->second->m_depth != 0)
                {
                    cout << "[Thread " << it->first << "] Unbalanced PerfMarker detected.\n";
                }

                if (g_isTimeoutMode)
                {
                    ofstream_with_filename* pOfstream = dynamic_cast<ofstream_with_filename*>(it->second->m_pOstream);
                    pOfstream->close();
                    gtString ofstreamFileName;
                    ofstreamFileName.fromASCIIString(pOfstream->m_fileName.c_str());
                    osFilePath markerFilePath;
                    markerFilePath.setFullPathFromString(ofstreamFileName);
                    osFile markerFile(markerFilePath);
                    markerFile.open(osChannel::OS_ASCII_TEXT_CHANNEL);
                    gtASCIIString fileContents;
                    markerFile.readIntoString(fileContents);
                    markerFile.close();
                    content.assign(fileContents.asCharArray());
                    remove(pOfstream->m_fileName.c_str());
                }
                else
                {
                    content = dynamic_cast<stringstream*>(it->second->m_pOstream)->str();
                }

                // num of markers
                fout << GetNumLines(content) << endl;
                fout << content;
                delete it->second;
            }

            g_perfMarkerItemMap.clear();
            fout.close();

            g_bFinalized = true;
            return AL_SUCCESS;
        }
        else
        {
            return AL_FAILED_TO_OPEN_OUTPUT_FILE;
        }
    }
    else
    {
        return AL_UNINITIALIZED_ACTIVITY_LOGGER;
    }
}

extern "C"
int AL_API_CALL amdtStopProfiling(amdtProfilingControlMode profilingControlMode)
{
    int result = AL_SUCCESS;

    if (profilingControlMode & AMDT_CPU_PROFILING)
    {
        result = AMDTCpuProfilePause();
    }
    else
    {
        result = AMDTActivityLoggerProfileControl::Instance()->StopProfiling(profilingControlMode);
    }

    return result;
}

extern "C"
int AL_API_CALL amdtResumeProfiling(amdtProfilingControlMode profilingControlMode)
{
    int result = AL_SUCCESS;

    if (profilingControlMode & AMDT_CPU_PROFILING)
    {
        result = AMDTCpuProfileResume();
    }
    else
    {
        result = AMDTActivityLoggerProfileControl::Instance()->ResumeProfiling(profilingControlMode);
    }

    return result;
}

extern "C"
int AL_API_CALL amdtStopProfilingEx()
{
    return amdtStopProfiling(AMDT_CPU_PROFILING);
}

extern "C"
int AL_API_CALL amdtResumeProfilingEx()
{
    return amdtResumeProfiling(AMDT_CPU_PROFILING);
}
