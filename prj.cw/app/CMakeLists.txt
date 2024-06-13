cmake_minimum_required(VERSION 3.27.7)

project(cw LANGUAGES CXX)

add_executable(${PROJECT_NAME} main.cpp)

find_package(raylib QUIET)
if(NOT raylib_FOUND)
    message("raylib not found")
    message("Trying to download raylib...")
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib)
    message("raylib downloaded")
    target_link_libraries(${PROJECT_NAME} PUBLIC raylib)
else()
  message("raylib found")
  target_link_libraries(${PROJECT_NAME} PUBLIC raylib)
endif()

find_package(a2i REQUIRED)
message("a2i found")
find_package(OpenCV REQUIRED)
message("OpenCV found")

find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
if(FFTW_FOUND)
    message("FFTW found")
    target_link_libraries(${PROJECT_NAME} PUBLIC PkgConfig::FFTW)
else()
    if(NOT DEFINED FFTW_LIBRARIES)
        message(WARNING "FFTW not detected by pkg-config, using fftw3 or define -DFFTW_LIBRARIES (e.g. =-lfftw3) and -DFFTW_INCLUDE_DIRS (e.g. =/usr/include)")
        target_link_libraries(${PROJECT_NAME} INTERFACE fftw3)
        target_include_directories(${PROJECT_NAME} INTERFACE ${FFTW_INCLUDE_DIRS})
    else()
        target_link_libraries(${PROJECT_NAME} INTERFACE ${FFTW_LIBRARIES})
        target_include_directories(${PROJECT_NAME} INTERFACE ${FFTW_INCLUDE_DIRS})
    endif()
    check_cxx_symbol_exists(fftw_plan_dft_1d fftw3.h FFTW3_CHECKED)
    if(FFTW_FOUND)
        message("FFTW found")
    else()
        message(FATAL_ERROR "FFTW not found")
    endif()
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC 
  raylib
  ${OpenCV_LIBS}
  a2i)
