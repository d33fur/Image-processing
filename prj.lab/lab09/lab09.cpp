#include <opencv2/opencv.hpp> 
#include <iostream> 
#include <vector>
 
using cv::Mat;
using std::cout;
using std::vector;

void convertToLinRGB(Mat& image) {
  image.convertTo(image, CV_32F, 1.0 / 255.0);
  cv::pow(image, 2.2, image);
}

void convertToSRGB(Mat& image) {
  cv::pow(image, 1.0 / 2.2, image);
  image.convertTo(image, CV_8U, 255.0);
}

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

void labelSquares(Mat& image, int rows, int cols) {
  int squareHeight = image.rows / rows;
  int squareWidth = image.cols / cols;

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      int x = c * squareWidth;
      int y = r * squareHeight;
      cv::rectangle(image, cv::Point(x, y), cv::Point(x + squareWidth, y + squareHeight), cv::Scalar(0, 0, 0), 2);
      std::string label = std::to_string(r * cols + c + 1);
      cv::putText(image, label, cv::Point(x + 5, y + 25), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
    }
  }
}

void checkSquareColors(const Mat& image, const std::string& label, int rows, int cols) {
  int squareHeight = image.rows / rows;
  int squareWidth = image.cols / cols;

  vector<Mat> channels(3);
  split(image, channels);

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      int x = c * squareWidth;
      int y = r * squareHeight;
      cv::Rect squareRegion(x, y, squareWidth, squareHeight);
      cv::Scalar meanB = mean(channels[0](squareRegion));
      cv::Scalar meanG = mean(channels[1](squareRegion));
      cv::Scalar meanR = mean(channels[2](squareRegion));
      cout << label << " Square " << (r * cols + c + 1) << " - B: " << meanB[0] << " G: " << meanG[0] << " R: " << meanR[0] << std::endl;
    }
  }
}

int main(int argc, char** argv) {
  cv::String keys = "{@||}";
  cv::CommandLineParser commandlineparser(argc, argv, keys);

  std::string imagePath = commandlineparser.get<std::string>(0);

  Mat image = cv::imread(imagePath);
  Mat ref = cv::imread("./../prj.lab/lab09/ref1.png");

  if (image.empty() || ref.empty()) {
      cout << "Can't open image or ref" << std::endl;
      return -1;
  }

  cv::imshow("Preprocessed", image);
  cv::imshow("Preprocessed ref", ref);

  labelSquares(ref, 4, 6);
  cv::imshow("Labeled ref", ref);

  checkVals(image, "Before correction");
  checkSquareColors(ref, "Before correction ref", 4, 6);

  convertToLinRGB(image);
  grayWorldCorrection(image);
  convertToSRGB(image);

  convertToLinRGB(ref);
  grayWorldCorrection(ref);
  convertToSRGB(ref);

  cv::imwrite("lab09result.jpg", image);
  cv::imshow("result", image);
  cv::imshow("result ref", ref);

  checkVals(image, "After correction");
  checkSquareColors(ref, "After correction ref", 4, 6);

  cv::waitKey(0);
  return 0;
}