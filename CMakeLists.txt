cmake_minimum_required (VERSION 3.0)

project(Tracer)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install here")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

###############################################################################
## file globbing ##############################################################
###############################################################################

include_directories(include)
file(GLOB SOURCES src/*.cpp include/*.h include/*.hpp)

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(Tracer ${SOURCES})

###############################################################################
## dependencies ###############################################################
###############################################################################

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty CACHE PATH "Third party dependency root")
include_directories(${THIRD_PARTY_DIR}/include)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(Tracer INTERFACE ${OPENGL_LIBRARIES})

if(APPLE)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)

    if(CMAKE_C_COMPILER_ID MATCHES "Clang\$")
        set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_C_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY omp)
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang\$")
        set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp")
        set(OpenMP_CXX_LIB_NAMES "omp")
        set(OpenMP_omp_LIBRARY omp)
    endif()
endif()

find_package(OpenMP REQUIRED)

if(WIN32)
    # visual studio running environment
    file( WRITE "${CMAKE_CURRENT_BINARY_DIR}/Tracer.vcxproj.user" 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>     \
    <Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">
        <LocalDebuggerWorkingDirectory>${CMAKE_BINARY_DIR}</LocalDebuggerWorkingDirectory>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">
        <LocalDebuggerWorkingDirectory>${CMAKE_BINARY_DIR}</LocalDebuggerWorkingDirectory>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    </Project>")    
endif()

###############################################################################
## installation ###############################################################
###############################################################################

file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS Tracer DESTINATION ${CMAKE_INSTALL_PREFIX})
