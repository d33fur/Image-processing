#include <opencv2/opencv.hpp> 
#include <iostream> 
#include <vector>
 
using cv::Mat;
using std::cout;
using std::vector;

void checkVals(const Mat& image, const std::string& label) {
  vector<Mat> channels(3);
  split(image, channels);

  cv::Scalar meanB = mean(channels[0]);
  cv::Scalar meanG = mean(channels[1]);
  cv::Scalar meanR = mean(channels[2]);

  cout << label << "B: " << meanB[0] << " G: " << meanG[0] << " R: " << meanR[0] << std::endl;
}

void grayWorldCorrection (Mat& image) {
  vector<cv::Mat> channels(3);
  cv::split(image, channels);

  cv::Scalar meanB = cv::mean(channels[0]);
  cv::Scalar meanG = cv::mean(channels[1]);
  cv::Scalar meanR = cv::mean(channels[2]);

  double meanGray = (meanB[0] + meanG[0] + meanR[0]) / 3.0;

  double scaleB = meanGray / meanB[0];
  double scaleG = meanGray / meanG[0];
  double scaleR = meanGray / meanR[0];

  channels[0] = channels[0] * scaleB;
  channels[1] = channels[1] * scaleG;
  channels[2] = channels[2] * scaleR;

  cv::merge(channels, image);
}

int main(int argc, char** argv) {
  cv::String keys = "{@||}";
  cv::CommandLineParser commandlineparser(argc, argv, keys);

  std::string imagePath = commandlineparser.get<std::string>(0);

  Mat image = cv::imread(imagePath);
  cv::imshow("Preprocessed", image);

  checkVals(image, "Before correction");

  grayWorldCorrection(image);

  cv::imwrite("lab09result.jpg", image);
  cv::imshow("result", image);

  checkVals(image, "After correction");

  cv::waitKey(0);
}