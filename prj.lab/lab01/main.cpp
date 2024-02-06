#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {
  int width = 1000;
  int height = 200;

  Mat image = Mat::zeros(height, width, CV_8UC3);
  // Mat image(3, 30 * 30, CV_8UC3, Scalar(255, 255, 255));
  for (int col = 0; col < image.cols; ++col) {
      float intensity = static_cast<float>(col) / image.cols;
      image.col(col).setTo(cv::Scalar(255 * intensity, 255 * intensity, 255 * intensity));
  }

  // for (int i = 0; i < 256; ++i) {
  //     Rect rect(i * 3, 0, 3, 30);
  //     rectangle(image, rect, Scalar(i, i, i), -1);
  // }

  String windowName = "window";

  namedWindow(windowName);

  imshow(windowName, image);

  waitKey(0);

  // destroyWindow(windowName);

  return 0;
}