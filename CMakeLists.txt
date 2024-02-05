cmake_minimum_required( VERSION 3.27.7 )

project(image-processing LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)

# include_directories(${OpenCV_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    opencv_core
    opencv_highgui
    opencv_imgproc
    opencv_imgcodecs
    opencv_videoio
    opencv_features2d
    opencv_calib3d
    opencv_objdetect
    opencv_flann
)