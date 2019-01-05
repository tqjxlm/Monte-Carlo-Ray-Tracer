cmake_minimum_required (VERSION 3.0)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install here")

project (MT-Tracer)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_BUILD_TYPE Release)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

###############################################################################
## file globbing ##############################################################
###############################################################################

include_directories(include)
file(GLOB SOURCES src/*.cpp include/*.h)
set(ENV{RSC_DIR} ${CMAKE_SOURCE_DIR})

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(MT-Tracer ${SOURCES})
target_link_libraries(MT-Tracer ${PROJECT_LINK_LIBS} )

###############################################################################
## dependencies ###############################################################
###############################################################################

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty CACHE PATH "Third party dependency root")
include_directories(${THIRD_PARTY_DIR}/include)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(MT-Tracer INTERFACE ${OPENGL_LIBRARIES})

if(WIN32)
    # visual studio running environment
    file( WRITE "${CMAKE_CURRENT_BINARY_DIR}/MT-Tracer.vcxproj.user" 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>     \
    <Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">
        <LocalDebuggerCommandArguments>2</LocalDebuggerCommandArguments>
        <LocalDebuggerWorkingDirectory>$(RSC_DIR)</LocalDebuggerWorkingDirectory>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">
        <LocalDebuggerCommandArguments>2</LocalDebuggerCommandArguments>
        <LocalDebuggerWorkingDirectory>$(RSC_DIR)</LocalDebuggerWorkingDirectory>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    </Project>")    
endif()

###############################################################################
## installation ###############################################################
###############################################################################

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS MT-Tracer DESTINATION ${CMAKE_INSTALL_PREFIX})
