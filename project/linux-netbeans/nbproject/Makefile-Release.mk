#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/56252444/FinjinPrecompiled.o \
	${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationDelegate.o \
	${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationSettings.o \
	${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationViewportDelegate.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/home/build/finjin-3rd-party/cpp/boost/stage/lib -L/home/build/finjin-3rd-party/cpp/curl/lib ../../../finjin-engine/cpp/library/project/linux-netbeans/dist/Release/GNU-Linux/libfinjin-engine.a ../../../finjin-common/cpp/library/project/linux-netbeans/dist/Release/GNU-Linux/libfinjin-common.a -lpthread -lxcb -lxcb-keysyms -lxcb-ewmh -lxcb-xinerama -lxcb-randr -lboost_system -lboost_filesystem -lboost_thread -lboost_regex -lboost_chrono -lboost_timer -lboost_locale -lboost_program_options -lboost_log -lcurl -lrt -ldl -lX11 -lopenal

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/linux-netbeans

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/linux-netbeans: ../../../finjin-engine/cpp/library/project/linux-netbeans/dist/Release/GNU-Linux/libfinjin-engine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/linux-netbeans: ../../../finjin-common/cpp/library/project/linux-netbeans/dist/Release/GNU-Linux/libfinjin-common.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/linux-netbeans: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/linux-netbeans ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/56252444/FinjinPrecompiled.o: ../../src/FinjinPrecompiled.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_XCB_KHR -I/home/build/finjin-3rd-party/cpp/include -I/home/build/finjin-3rd-party/cpp/boost -I/home/build/finjin-3rd-party/cpp/eigen -I/home/derek/git/nedelman/finjin-common/cpp/library/include -I/home/derek/git/nedelman/finjin-engine/cpp/library/src -I/home/derek/git/nedelman/finjin-engine/cpp/library/src/finjin/engine/internal/app/linux -I../../src -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/FinjinPrecompiled.o ../../src/FinjinPrecompiled.cpp

${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationDelegate.o: ../../src/FinjinViewerApplicationDelegate.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_XCB_KHR -I/home/build/finjin-3rd-party/cpp/include -I/home/build/finjin-3rd-party/cpp/boost -I/home/build/finjin-3rd-party/cpp/eigen -I/home/derek/git/nedelman/finjin-common/cpp/library/include -I/home/derek/git/nedelman/finjin-engine/cpp/library/src -I/home/derek/git/nedelman/finjin-engine/cpp/library/src/finjin/engine/internal/app/linux -I../../src -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationDelegate.o ../../src/FinjinViewerApplicationDelegate.cpp

${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationSettings.o: ../../src/FinjinViewerApplicationSettings.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_XCB_KHR -I/home/build/finjin-3rd-party/cpp/include -I/home/build/finjin-3rd-party/cpp/boost -I/home/build/finjin-3rd-party/cpp/eigen -I/home/derek/git/nedelman/finjin-common/cpp/library/include -I/home/derek/git/nedelman/finjin-engine/cpp/library/src -I/home/derek/git/nedelman/finjin-engine/cpp/library/src/finjin/engine/internal/app/linux -I../../src -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationSettings.o ../../src/FinjinViewerApplicationSettings.cpp

${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationViewportDelegate.o: ../../src/FinjinViewerApplicationViewportDelegate.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/56252444
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_XCB_KHR -I/home/build/finjin-3rd-party/cpp/include -I/home/build/finjin-3rd-party/cpp/boost -I/home/build/finjin-3rd-party/cpp/eigen -I/home/derek/git/nedelman/finjin-common/cpp/library/include -I/home/derek/git/nedelman/finjin-engine/cpp/library/src -I/home/derek/git/nedelman/finjin-engine/cpp/library/src/finjin/engine/internal/app/linux -I../../src -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/56252444/FinjinViewerApplicationViewportDelegate.o ../../src/FinjinViewerApplicationViewportDelegate.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -s -DVK_NO_PROTOTYPES -DVK_USE_PLATFORM_XCB_KHR -I/home/build/finjin-3rd-party/cpp/include -I/home/build/finjin-3rd-party/cpp/boost -I/home/build/finjin-3rd-party/cpp/eigen -I/home/derek/git/nedelman/finjin-common/cpp/library/include -I/home/derek/git/nedelman/finjin-engine/cpp/library/src -I/home/derek/git/nedelman/finjin-engine/cpp/library/src/finjin/engine/internal/app/linux -I../../src -std=c++14 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:
	cd ../../../finjin-engine/cpp/library/project/linux-netbeans && ${MAKE}  -f Makefile CONF=Release
	cd ../../../finjin-common/cpp/library/project/linux-netbeans && ${MAKE}  -f Makefile CONF=Release

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/linux-netbeans

# Subprojects
.clean-subprojects:
	cd ../../../finjin-engine/cpp/library/project/linux-netbeans && ${MAKE}  -f Makefile CONF=Release clean
	cd ../../../finjin-common/cpp/library/project/linux-netbeans && ${MAKE}  -f Makefile CONF=Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
