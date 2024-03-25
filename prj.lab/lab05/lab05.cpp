#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

void addObjects(cv::Mat& image) {
  std::vector<int> colors {0, 127, 255};

  for(int i = 0; i < colors.size(); i++) {
    int row = 0;
    for(int j = 0; j < colors.size(); j++) {
      if(j != i) {
          int square = colors[(i + row) % 3];
          int circle = colors[(i + 2) % 3];

          cv::Mat object(99, 99, CV_8UC1, 255);
          cv::Rect fullObject(99 * i, 99 * row, 99, 99);

          cv::rectangle(object, 
            cv::Rect(0, 0, 99, 99), 
            cv::Scalar(square), -1);
          
          cv::circle(object, 
            cv::Point(99 / 2, 99 / 2), 99 / 4, 
            cv::Scalar(circle), -1);

          object.copyTo(image(fullObject));
          row++;
      }
    }
  }
}

cv::Mat getFilteredImg(cv::Mat& image, int var) {
  std::vector<cv::Mat> processedImgVec;
  cv::Mat mergedImage(image.rows * 2, image.cols, CV_32F, 255);
  cv::Mat first, second, kernel, invertedKernel;
  image.convertTo(image, CV_32F);

  switch(var) {
    case 1: 
      kernel = (cv::Mat_<double>(2, 2) << -1, 1, -1, 1) / 4.0;
      invertedKernel = (cv::Mat_<double>(2, 2) << 1, 1, -1, -1) / 4.0;
    case 2:
      kernel = (cv::Mat_<double>(2, 2) << 1, 0, 0, -1) / 4.0;
      invertedKernel = (cv::Mat_<double>(2, 2) << 0, 1, -1, 0) / 4.0;
  }

  cv::filter2D(image, first, CV_32F, kernel);
  cv::filter2D(image, second, CV_32F, invertedKernel);
  processedImgVec.push_back(first);
  processedImgVec.push_back(second);

  for(int i = 0; i < processedImgVec.size(); i++) {
    cv::Rect img(0, i * image.rows, image.cols, image.rows);
    processedImgVec[i].copyTo(mergedImage(img));
  }

  return mergedImage;
}

int main() {
  cv::Mat image(99 * 2, 99 * 3, CV_8UC1, 255);
  cv::Mat processedImage(image.rows * 2, image.cols, CV_32F, 255);
  int var = 2;

  addObjects(image);
  processedImage = getFilteredImg(image, var);
  cv::normalize(processedImage, processedImage, 0, 255, cv::NORM_MINMAX, CV_8UC1);

  cv::imshow("image", processedImage);
  cv::waitKey(0);
  return 0;
}
