add_executable(lab03 lab03.cpp)
target_link_libraries(lab03 PRIVATE 
  opencv_core 
  opencv_imgproc 
  opencv_highgui)