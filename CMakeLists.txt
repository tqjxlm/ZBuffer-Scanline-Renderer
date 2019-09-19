cmake_minimum_required (VERSION 3.0)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "Install here")

project (ScanLine)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_BUILD_TYPE Release)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

###############################################################################
## file globbing ##############################################################
###############################################################################

include_directories(include)
file(GLOB SOURCES src/*.cpp include/*.h resources/shaders/*)
set(ENV{RSC_DIR} ${CMAKE_SOURCE_DIR})

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(ScanLine ${SOURCES})
target_link_libraries(ScanLine ${PROJECT_LINK_LIBS} )
SOURCE_GROUP("Shader Files" resources/shaders/*)

###############################################################################
## dependencies ###############################################################
###############################################################################

set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty CACHE PATH "The root directory for all third party dependencies.")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${THIRD_PARTY_DIR})
set(ENV{THIRD_PARTY_DIR} ${THIRD_PARTY_DIR})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(ScanLine INTERFACE ${OPENGL_LIBRARIES})

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(ScanLine PUBLIC OpenMP::OpenMP_CXX)
endif()

find_package(glm CONFIG REQUIRED)
target_link_libraries(ScanLine PUBLIC glm)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(ScanLine PUBLIC glfw)

find_package(GLEW CONFIG REQUIRED)
target_link_libraries(ScanLine PUBLIC GLEW::GLEW)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(ScanLine PUBLIC ${ASSIMP_LIBRARIES})

find_package(SOIL CONFIG REQUIRED)
target_link_libraries(ScanLine PUBLIC SOIL)

if(WIN32)
    # visual studio running environment
    file( WRITE "${CMAKE_CURRENT_BINARY_DIR}/ScanLine.vcxproj.user" 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>     \
    <Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Debug|x64'\">
        <LocalDebuggerCommandArguments>2</LocalDebuggerCommandArguments>
        <LocalDebuggerWorkingDirectory>$(RSC_DIR)</LocalDebuggerWorkingDirectory>
        <LocalDebuggerEnvironment>PATH=$(THIRD_PARTY_DIR)/debug/bin</LocalDebuggerEnvironment>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    <PropertyGroup Condition=\"'$(Configuration)|$(Platform)'=='Release|x64'\">
        <LocalDebuggerCommandArguments>2</LocalDebuggerCommandArguments>
        <LocalDebuggerWorkingDirectory>$(RSC_DIR)</LocalDebuggerWorkingDirectory>
        <LocalDebuggerEnvironment>PATH=$(THIRD_PARTY_DIR)/bin</LocalDebuggerEnvironment>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    </Project>")    
endif()

###############################################################################
## installation ###############################################################
###############################################################################

file(GLOB DEPS ${THIRD_PARTY_DIR}/bin/*.dll)
install(FILES ${DEPS} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS ScanLine DESTINATION ${CMAKE_INSTALL_PREFIX})