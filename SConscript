# -*- Python -*-

Import('*')
from CXL_init import *

libName = "CXLActivityLogger"

env = CXL_env.Clone()

env.Append( CPPPATH = [ 
    ".",
    "../",
    "../TSingleton",
    "../AMDTMutex",
    "../../Lib/Ext/utf8cpp/source",
    env['CXL_gpu_profiler_backend_dir'],
    env['CXL_commonproj_dir'],
])

# osMessageBox, osDesktop
env.Append(CPPFLAGS = '-std=c++11 -fno-strict-aliasing -D_LINUX -DAMDT_BUILD_SUFFIX= -DAMDT_DEBUG_SUFFIX=')

sources = \
[
    env['CXL_gpu_profiler_backend_dir'] + "/Common/Logger.cpp",
    env['CXL_gpu_profiler_backend_dir'] + "/Common/StringUtils.cpp",
    env['CXL_gpu_profiler_backend_dir'] + "/Common/OSUtils.cpp",
    env['CXL_gpu_profiler_backend_dir'] + "/Common/FileUtils.cpp",
    env['CXL_gpu_profiler_backend_dir'] + "/Common/BinFileHeader.cpp",
    env['CXL_gpu_profiler_backend_dir'] + "/Common/LocaleSetting.cpp",
    "../../../Common/Src/AMDTMutex/AMDTMutex.cpp",
    "AMDTActivityLogger.cpp",
    "AMDTActivityLoggerProfileControl.cpp",
    "AMDTCpuProfileControl_Lin.cpp"
]
    
# Creating object files    
objFiles = env.SharedObject(sources)


env.Append (LIBS = [
    "libCXLOSWrappers",
    "libCXLBaseTools"
])

# Creating shared libraries
soFiles = env.SharedLibrary(
    target = libName, 
    source = objFiles)

# Installing libraries
libInstall = env.Install( 
    dir = env['CXL_lib_dir'], 
    source = (soFiles))

Return('libInstall')
