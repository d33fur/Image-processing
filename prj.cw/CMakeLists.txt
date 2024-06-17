cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)

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
    target_include_directories(a2i PRIVATE ${FFTW_INCLUDE_DIRS})
    target_link_libraries(a2i PRIVATE ${FFTW_LIBRARIES})
else()
    message(FATAL_ERROR "FFTW not found")
endif()

find_package(OpenCV CONFIG REQUIRED)
if(OpenCV_FOUND)
    message("OpenCV found")
    target_include_directories(a2i PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(a2i PRIVATE ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "OpenCV not found")
endif()

target_include_directories(a2i PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

set_target_properties(a2i PROPERTIES
    EXPORT_NAME a2i
    INTERFACE_LINK_LIBRARIES "${FFTW_LIBRARIES}")
    # INTERFACE_LINK_LIBRARIES "a2i")

install(TARGETS a2i EXPORT a2iConfig
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${LIB_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/a2i)
install(EXPORT a2iConfig DESTINATION share/a2i/cmake)
export(TARGETS a2i FILE a2iConfig.cmake)


# add_subdirectory(app)