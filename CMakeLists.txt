#cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

#define project name
PROJECT(ryzenadj)

include(GNUInstallDirs)

set(CMAKE_C_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(PREFER_STATIC_LINKING "Prefer static linking for the ryzenadj executable" OFF)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#Enable LTO
include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)
if( supported )
    message(STATUS "IPO / LTO enabled")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

AUX_SOURCE_DIRECTORY(./ SRC_DIR)

if(WIN32)
    # PawnIO configuration
    set(OS_SOURCE lib/win32/osdep_win32.cpp)
    
    # PawnIO requires the following setup:
    # 1. PawnIO must be installed on the system
    # 2. Set PAWNIO_ROOT environment variable or CMake variable to PawnIO installation directory
    # 3. Example: cmake -DPAWNIO_ROOT="C:\Program Files\PawnIO" ..
    
    if(NOT PAWNIO_ROOT)
        # Try common installation paths
        if(EXISTS "C:/Program Files/PawnIO")
            set(PAWNIO_ROOT "C:/Program Files/PawnIO")
        elseif(EXISTS "C:/Program Files (x86)/PawnIO")
            set(PAWNIO_ROOT "C:/Program Files (x86)/PawnIO")
        elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/pawnio")
            set(PAWNIO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/pawnio")
        else()
            message(FATAL_ERROR 
                "PawnIO not found. Please set PAWNIO_ROOT environment variable or pass -DPAWNIO_ROOT=<path> to cmake\n"
                "PawnIO can be downloaded from: https://github.com/namazso/PawnIO"
            )
        endif()
    endif()
    
    message(STATUS "PawnIO Root: ${PAWNIO_ROOT}")
    
    # Include PawnIO headers
    set(PAWNIO_INCLUDE_DIR "${PAWNIO_ROOT}/include")
    if(NOT EXISTS "${PAWNIO_INCLUDE_DIR}")
        message(FATAL_ERROR "PawnIO include directory not found at ${PAWNIO_INCLUDE_DIR}")
    endif()
    
    # Find PawnIOLib
    find_library(PAWNIO_LIB NAMES PawnIOLib
        PATHS "${PAWNIO_ROOT}/lib" "${PAWNIO_ROOT}"
        NO_DEFAULT_PATH
    )
    
    if(NOT PAWNIO_LIB)
        message(FATAL_ERROR 
            "PawnIOLib library not found in ${PAWNIO_ROOT}/lib\n"
            "Make sure PawnIO is properly installed"
        )
    endif()
    
    message(STATUS "PawnIOLib found: ${PAWNIO_LIB}")
    
    # Create interface library for PawnIO
    add_library(PawnIO INTERFACE)
    target_include_directories(PawnIO INTERFACE "${PAWNIO_INCLUDE_DIR}")
    target_link_libraries(PawnIO INTERFACE "${PAWNIO_LIB}")
    
    set(OS_LINK_LIBRARY PawnIO)
    set(OS_STATIC_LINK_LIBRARY PawnIO)
    
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS_SOURCE lib/linux/osdep_linux.c lib/linux/osdep_linux_mem.c lib/linux/osdep_linux_smu_kernel_module.c)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBPCI REQUIRED libpci)

    add_library(ryzenadj_libpci INTERFACE)
    add_library(RyzenAdj::Libpci ALIAS ryzenadj_libpci)
    target_include_directories(ryzenadj_libpci INTERFACE ${LIBPCI_INCLUDE_DIRS})
    target_compile_options(ryzenadj_libpci INTERFACE ${LIBPCI_CFLAGS_OTHER})
    target_link_libraries(ryzenadj_libpci INTERFACE ${LIBPCI_LDFLAGS})

    add_library(ryzenadj_libpci_static INTERFACE)
    add_library(RyzenAdj::LibpciStatic ALIAS ryzenadj_libpci_static)
    target_include_directories(ryzenadj_libpci_static INTERFACE ${LIBPCI_STATIC_INCLUDE_DIRS})
    target_compile_options(ryzenadj_libpci_static INTERFACE ${LIBPCI_STATIC_CFLAGS_OTHER})
    target_link_libraries(ryzenadj_libpci_static INTERFACE ${LIBPCI_STATIC_LDFLAGS})

    set(OS_LINK_LIBRARY RyzenAdj::Libpci)
    set(OS_STATIC_LINK_LIBRARY RyzenAdj::LibpciStatic)
else()
    message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}")
endif()

set(COMMON_SOURCES lib/nb_smu_ops.c lib/api.c lib/cpuid.c)
add_definitions(-D_LIBRYZENADJ_INTERNAL)

ADD_EXECUTABLE(${PROJECT_NAME} ${OS_SOURCE} ${COMMON_SOURCES} argparse.c main.c)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND (PREFER_STATIC_LINKING OR NOT BUILD_SHARED_LIBS))
    if(PREFER_STATIC_LINKING)
        set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -static")
    endif()
    target_link_libraries(${PROJECT_NAME} ${OS_STATIC_LINK_LIBRARY})
else()
    target_link_libraries(${PROJECT_NAME} ${OS_LINK_LIBRARY})
endif()
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
ADD_LIBRARY (libryzenadj ${OS_SOURCE} ${COMMON_SOURCES})
set_target_properties(libryzenadj PROPERTIES PREFIX "")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT BUILD_SHARED_LIBS)
    target_link_libraries(libryzenadj ${OS_STATIC_LINK_LIBRARY})
else()
    target_link_libraries(libryzenadj ${OS_LINK_LIBRARY})
endif()
#SET_TARGET_PROPERTIES(libryzenadj PROPERTIES LINKER_LANGUAGE C)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
