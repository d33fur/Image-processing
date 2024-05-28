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

find_package(OpenCV CONFIG REQUIRED)

find_package(raylib QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.0
        GIT_SHALLOW 1
    )
    FetchContent_MakeAvailable(raylib)
endif()

# find_package(raylib_cpp QUIET)
# if (NOT raylib_cpp_FOUND)
#     if (NOT DEFINED RAYLIB_CPP_VERSION)
#         set(RAYLIB_CPP_VERSION v5.0.1)
#     endif()
#     include(FetchContent)
#     FetchContent_Declare(
#         raylib_cpp
#         GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
#         GIT_TAG ${RAYLIB_CPP_VERSION}
#     )
#     FetchContent_MakeAvailable(raylib_cpp)
# endif()

add_executable(cw main.cpp a2i/spectrogram.cpp a2i/spectrogram.hpp)

set_target_properties(cw PROPERTIES CXX_STANDARD 11)
target_link_libraries(cw PUBLIC 
  raylib 
  # raylib_cpp
  opencv_core 
  opencv_imgproc 
  opencv_highgui
)

add_executable(test test.cpp)

set_target_properties(test PROPERTIES CXX_STANDARD 11)
target_link_libraries(test PUBLIC 
  raylib 
  # raylib_cpp
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
