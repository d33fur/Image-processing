set(CMAKE_CXX_STANDARD 20)

cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILT_TYPE DEBUG) # RELEASE
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror")

set(SOURCE_DIR "a2i/source")
set(INCLUDE_DIR "a2i/include")

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE LIB_HEADERS CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.hpp")
include_directories(${INCLUDE_DIR})

include(GNUInstallDirs)

add_library(a2i STATIC ${LIB_SOURCES})

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
if(FFTW_FOUND)
    message("FFTW found")
    target_link_libraries(a2i PUBLIC PkgConfig::FFTW)
else()
    if(NOT DEFINED FFTW_LIBRARIES)
        message(WARNING "FFTW not detected by pkg-config, using fftw3 or define -DFFTW_LIBRARIES (e.g. =-lfftw3) and -DFFTW_INCLUDE_DIRS (e.g. =/usr/include)")
        target_link_libraries(a2i INTERFACE fftw3)
        target_include_directories(a2i INTERFACE ${FFTW_INCLUDE_DIRS})
    else()
        target_link_libraries(a2i INTERFACE ${FFTW_LIBRARIES})
        target_include_directories(a2i INTERFACE ${FFTW_INCLUDE_DIRS})
    endif()
    check_cxx_symbol_exists(fftw_plan_dft_1d fftw3.h FFTW3_CHECKED)
    if(FFTW_FOUND)
        message("FFTW found")
    else()
        message(FATAL_ERROR "FFTW not found")
    endif()
endif()

find_package(OpenCV CONFIG REQUIRED)
if(OpenCV_FOUND)
    message("OpenCV found")
    target_link_libraries(a2i PUBLIC ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "OpenCV not found")
endif()

target_include_directories(a2i PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

set_target_properties(a2i PROPERTIES
    EXPORT_NAME a2i
    INTERFACE_LINK_LIBRARIES "a2i")

install(TARGETS a2i EXPORT a2iConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${LIB_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/a2i)
install(EXPORT a2iConfig DESTINATION share/a2i/cmake)
export(TARGETS a2i FILE a2iConfig.cmake)

add_subdirectory(app)