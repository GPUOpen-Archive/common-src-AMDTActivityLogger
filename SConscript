# -*- Python -*-

Import('*')
from CXL_init import *

libName = "CXLActivityLogger"

env = CXL_env.Clone()

env.Append( CPPPATH = [
    ".",
    "../",
    "../TSingleton",
    "../../Lib/Ext/utf8cpp/source",
    env['CXL_commonproj_dir'],
])

# osMessageBox, osDesktop
env.Append(CPPFLAGS = '-std=c++11 -fno-strict-aliasing -D_LINUX -DAMDT_BUILD_SUFFIX= -DAMDT_DEBUG_SUFFIX=')

sources = \
[
    "AMDTActivityLogger.cpp",
    "AMDTActivityLoggerProfileControl.cpp",
    "AMDTActivityLoggerTimeStamp.cpp",
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
