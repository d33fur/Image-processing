#include <iostream>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

Mat drawHistogram(const Mat& noisyImage) {
  Mat histImage;
  vector<Mat> bgr_planes;
  split(noisyImage, bgr_planes);

  int histSize = 256;
  float range[] = {0, 256};
  const float *histRange[] = {range};
  bool uniform = true, accumulate = false;

  Mat b_hist, g_hist, r_hist;
  calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate);
  calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate);
  calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate);

  int w = 256;
  int bin_w = cvRound((double) w / histSize);
  histImage = Mat(w, w, CV_8UC3, Scalar(230, 230, 230));
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

  for (int i = 1; i < histSize; i++) {
    rectangle(histImage,
      Point(bin_w * (i - 1), w),
      Point(bin_w * i, w - cvRound(b_hist.at<float>(i - 1))),
      Scalar(0, 0, 0),
      FILLED);
    rectangle(histImage,
      Point(bin_w * (i - 1), w),
      Point(bin_w * i, w - cvRound(g_hist.at<float>(i - 1))),
      Scalar(0, 0, 0),
      FILLED);
    rectangle(histImage,
      Point(bin_w * (i - 1), w),
      Point(bin_w * i, w - cvRound(r_hist.at<float>(i - 1))),
      Scalar(0, 0, 0),
      FILLED);
  }

  return histImage;
}

Mat generateNoiseImage(const Mat& image, const int& c) {
  Mat noise(256, 256, CV_8UC3);
  randn(noise, Scalar::all(0), Scalar::all(c));
  
  Mat noisyImage;
  add(image, noise, noisyImage);

  return noisyImage;
}

Mat generateHistograms(const Mat& image) {
  Mat canvas = Mat::zeros(256 * 7, 256, CV_8UC3);
  image.copyTo(canvas(Rect(0, 0, 256, 256)));

  int i = 1;
  for(const auto& c : {3, 7, 15}) {
    Mat noisyImage = generateNoiseImage(image, c);
    noisyImage.copyTo(canvas(Rect(0, 256 * i++, 256, 256)));
    drawHistogram(noisyImage).copyTo(canvas(Rect(0, 256 * i++, 256, 256)));
  }
  
  return canvas;
}

Mat generateImage(const vector<int>& c) {
  Mat image(256, 256, CV_8UC3, Scalar(c[0], c[0], c[0]));
  auto p1 = (256-209) / 2;
  auto p2 = 256 - p1;

  rectangle(image,
    Point(p1, p1),
    Point(p2, p2),
    Scalar(c[1], c[1], c[1]),
    FILLED,
    LINE_8);

  circle(image,
    Point(128, 128),
    83,
    Scalar(c[2], c[2], c[2]),
    FILLED,
    LINE_8);
    
  return generateHistograms(image);
}

int main() {
  Mat canvas = Mat::zeros(256 * 7, 256 * 4, CV_8UC3);
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

  imshow("Image", canvas);
  waitKey(0);

  return 0;
}