#include <iostream>
#include <vector>
#include <random>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

Mat drawHistogram(const Mat& noisyImage) {
  Mat histImage;
  int histSize = 256;
  float range[] = {0, 256};
  const float *histRange[] = {range};

  Mat hist;
  calcHist(&noisyImage, 1, 0, Mat(), hist, 1, &histSize, histRange, 1, 0);

  int w = 256;
  int bin_w = cvRound((double) w / histSize);
  histImage = Mat(w, w, CV_8UC1, Scalar(255));
  normalize(hist, hist, 0, 230, NORM_MINMAX, -1, Mat());

  for (int i = 1; i < histSize; i++) {
    rectangle(histImage,
      Point(bin_w * (i - 1), w),
      Point(bin_w * i, w - cvRound(hist.at<float>(i - 1))),
      Scalar(0),
      FILLED);
  }

  return histImage;
}

Mat generateNoiseImage(const Mat& image, const double& c) {
  random_device rd{};
  mt19937 gen{rd()};
  normal_distribution<> distribution{1., c};
  Mat noisyImage = image.clone();

  for (int i = 0; i < noisyImage.rows; i++) {
    for (int j = 0; j < noisyImage.cols; j++) {
      auto noise = round(distribution(gen));
      auto diff = static_cast<int>(noisyImage.at<uchar>(i, j)) + noise;
      
      diff < 0 || diff > 255 ? 
        noisyImage.at<uchar>(i, j) -= static_cast<uchar>(noise) : 
        noisyImage.at<uchar>(i, j) += static_cast<uchar>(noise);
    }
  }

  return noisyImage;
}

Mat generateHistograms(const Mat& image) {
  Mat canvas = Mat::zeros(256 * 7, 256, CV_8UC1);
  image.copyTo(canvas(Rect(0, 0, 256, 256)));

  int i = 1;
  for(const auto& c : {3., 7., 15.}) {
    Mat noisyImage = generateNoiseImage(image, c);
    noisyImage.copyTo(canvas(Rect(0, 256 * i++, 256, 256)));
    drawHistogram(noisyImage).copyTo(canvas(Rect(0, 256 * i++, 256, 256)));
  }
  
  return canvas;
}

Mat generateImage(const vector<int>& c) {
  Mat image(256, 256, CV_8UC1, Scalar(c[0]));
  auto p1 = (256-209) / 2;
  auto p2 = 256 - p1;

  rectangle(image,
    Point(p1, p1),
    Point(p2, p2),
    Scalar(c[1]),
    FILLED,
    LINE_8);

  circle(image,
    Point(128, 128),
    83,
    Scalar(c[2]),
    FILLED,
    LINE_8);
    
  return generateHistograms(image);
}

int main() {
  Mat canvas = Mat::zeros(256 * 7, 256 * 4, CV_8UC1);
  vector<vector<int>> colors = {
    {0, 127, 255},
    {20, 127, 235},
    {55, 127, 200},
    {90, 127, 165}
  };
  
  int i = 0;
  for(const auto& c : colors) {
    generateImage(c).copyTo(canvas(Rect(i++ * 256, 0, 256, 256 * 7)));
  }

  namedWindow("Image", WINDOW_NORMAL);
  resizeWindow("Image", 256 * 4, 256 * 7);
  imshow("Image", canvas);
  waitKey(0);

  return 0;
}
