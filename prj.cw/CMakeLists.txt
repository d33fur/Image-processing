set(CMAKE_CXX_STANDARD 20)

cmake_minimum_required(VERSION 3.14)

set(CMAKE_BUILT_TYPE DEBUG)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror")

# set(A2I_DIR ${CMAKE_SOURCE_DIR}/a2i)

# FILE(GLOB_RECURSE SOURCES
#     ${A2I_DIR}/*.hpp
#     ${A2I_DIR}/*.cpp
#     main.cpp)

# include_directories(${A2I_DIR})



add_executable(cw main.cpp a2i/spectrogram.cpp a2i/spectrogram.hpp)


find_package(PkgConfig REQUIRED)
pkg_search_module(FFTW REQUIRED fftw3 IMPORTED_TARGET)
if(FFTW_FOUND)
    message("FFTW found")
    # target_link_libraries(cw INTERFACE PkgConfig::FFTW)
    target_link_libraries(cw PUBLIC PkgConfig::FFTW)
else()  # some systems do not have fftw3 registered in pkg-config, in this case it falls back to good old -lfftw3 which can fail later
    if(NOT DEFINED FFTW_LIBRARIES)
        message(WARNING "FFTW not detected by pkg-config, using fftw3 or define -DFFTW_LIBRARIES (e.g. =-lfftw3) and -DFFTW_INCLUDE_DIRS (e.g. =/usr/include)")
        target_link_libraries(cw INTERFACE fftw3)
        target_include_directories(cw INTERFACE ${FFTW_INCLUDE_DIRS})
    else()
        target_link_libraries(cw INTERFACE ${FFTW_LIBRARIES})
        target_include_directories(cw INTERFACE ${FFTW_INCLUDE_DIRS})
    endif()
    check_cxx_symbol_exists(fftw_plan_dft_1d fftw3.h FFTW3_CHECKED)
    if(FFTW_FOUND)
    else()
        message(FATAL_ERROR "FFTW not found")
    endif()
endif()


find_package(OpenCV CONFIG REQUIRED)

find_package(raylib QUIET)
if (NOT raylib_FOUND)
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
    message("Downloaded raylib")
    message("raylib found")
    target_link_libraries(cw PUBLIC raylib)
else()
  message("raylib found")
  target_link_libraries(cw PUBLIC raylib)
endif()

set_target_properties(cw PROPERTIES CXX_STANDARD 11)
target_link_libraries(cw PUBLIC 
  raylib 
  opencv_core 
  opencv_imgproc 
  opencv_highgui
)

add_executable(test test.cpp)

set_target_properties(test PROPERTIES CXX_STANDARD 11)
target_link_libraries(test PUBLIC 
  raylib 
  opencv_core 
  opencv_imgproc 
  opencv_highgui
)

# if (${PLATFORM} STREQUAL "Web")
#     set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

#     target_link_options(${PROJECT_NAME} PRIVATE -sEXPORTED_FUNCTIONS=['_main','_malloc'] -sEXPORTED_RUNTIME_METHODS=ccall -sUSE_GLFW=3)
# endif()



























# set(CMAKE_CXX_STANDARD 20)

# cmake_minimum_required(VERSION 3.14)

# set(CMAKE_BUILT_TYPE DEBUG)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror")

# set(A2I_DIR ${CMAKE_SOURCE_DIR}/a2i)
# include_directories(${A2I_DIR})

# find_package(OpenCV CONFIG REQUIRED)

# find_package(raylib QUIET)
# if (NOT raylib_FOUND)
#     include(FetchContent)
#     FetchContent_Declare(
#         raylib
#         GIT_REPOSITORY https://github.com/raysan5/raylib.git
#         GIT_TAG 5.0
#         GIT_SHALLOW 1
#     )
#     FetchContent_MakeAvailable(raylib)
# endif()

# # find_package(raylib_cpp QUIET)
# # if (NOT raylib_cpp_FOUND)
# #     if (NOT DEFINED RAYLIB_CPP_VERSION)
# #         set(RAYLIB_CPP_VERSION v5.0.1)
# #     endif()
# #     include(FetchContent)
# #     FetchContent_Declare(
# #         raylib_cpp
# #         GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
# #         GIT_TAG ${RAYLIB_CPP_VERSION}
# #     )
# #     FetchContent_MakeAvailable(raylib_cpp)
# # endif()

# # set(SOURCES main.cpp)
# add_executable(cw test.cpp)
# set_target_properties(cw PROPERTIES CXX_STANDARD 11)
# target_link_libraries(cw PUBLIC 
#   raylib 
#   # raylib_cpp
#   opencv_core 
#   opencv_imgproc 
#   opencv_highgui
# )

# if (${PLATFORM} STREQUAL "Web")
#     set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")

#     target_link_options(${PROJECT_NAME} PRIVATE -sEXPORTED_FUNCTIONS=['_main','_malloc'] -sEXPORTED_RUNTIME_METHODS=ccall -sUSE_GLFW=3)
# endif()
