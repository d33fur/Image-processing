#include <iostream>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

void makeHistogram(const Mat& image, Mat& histImage) {
  vector<Mat> bgr_planes;
  split(image, bgr_planes);
  int histSize = 16;
  float range[] = {0, 256};
  const float *histRange[] = {range};
  bool uniform = true, accumulate = false;
  Mat b_hist, g_hist, r_hist;
  calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate);
  calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate);
  calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate);

  int hist_w = 512, hist_h = 400;
  int bin_w = cvRound((double) hist_w / histSize);
  histImage = Mat(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

  for (int i = 1; i < histSize; i++) {
    rectangle(histImage,
      Point(bin_w * (i - 1), hist_h),
      Point(bin_w * i, hist_h - cvRound(b_hist.at<float>(i - 1))),
      Scalar(255, 255, 255)
      // ,
      // FILLED
      );
    rectangle(histImage,
      Point(bin_w * (i - 1), hist_h),
      Point(bin_w * i, hist_h - cvRound(g_hist.at<float>(i - 1))),
      Scalar(255, 255, 255)      
      // ,
      // FILLED
      );
    rectangle(histImage,
      Point(bin_w * (i - 1), hist_h),
      Point(bin_w * i, hist_h - cvRound(r_hist.at<float>(i - 1))),
      Scalar(255, 255, 255)
      // ,
      // FILLED
      );
  }
}

int main(int argc, char* argv[]) {
  cv::CommandLineParser parser(argc, argv,
    "{ w | 800 | side of a square }"
    "{ c1 | 40 | color of big square }"
    "{ c2 | 127 | color small square }"
    "{ c3 | 220 | color of circle }");

  auto w = parser.get<int>("w");
  auto w1 = (w - sqrt(w * w * 2 / 3)) / 2;
  auto w3 = sqrt(w * w / (3 * 3.14));
  auto c1 = parser.get<int>("c1");
  auto c2 = parser.get<int>("c2");
  auto c3 = parser.get<int>("c3");

  Mat originalImage(w, w, CV_8UC3, Scalar(c1, c1, c1));

  rectangle(originalImage,
    Point(w1, w1),
    Point(w - w1, w - w1),
    Scalar(c2, c2, c2),
    FILLED,
    LINE_8);

  circle(originalImage,
    Point(w / 2, w / 2),
    w3,
    Scalar(c3, c3, c3),
    FILLED,
    LINE_8);

  Mat noise(originalImage.size(), CV_8UC3);
  randn(noise, Scalar::all(0), Scalar::all(25));
  
  Mat noisyImage;
  add(originalImage, noise, noisyImage);

  Mat histImageOriginal;
  makeHistogram(originalImage, histImageOriginal);

  Mat histImageNoisy;
  makeHistogram(noisyImage, histImageNoisy);

  imshow("Original Image", originalImage);
  imshow("Noisy Image", noisyImage);
  imshow("Histogram of Original Image", histImageOriginal);
  imshow("Histogram of Noisy Image", histImageNoisy);

  waitKey(0);

  return 0;
}
