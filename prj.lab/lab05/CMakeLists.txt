add_executable(lab05 lab05.cpp)
target_link_libraries(lab05 PRIVATE 
  opencv_core 
  opencv_imgproc 
  opencv_highgui)