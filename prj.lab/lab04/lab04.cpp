#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

// void generate(cv::Mat& canvas, int& a, int& b, std::pair<double, double>& s, std::pair<double, double>& c) {
//   int rows = std::sqrt(a);
  
// }

// std::pair<double, double> getPair(const std::string& str) {
//   double l, r;
//   std::istringstream iss(str);
//   char separator;
//   if(!(iss >> l >> separator >> r) || separator != '-') {
//     throw std::invalid_argument("Invalid string format");
//   }
//   return {l, r};
// }

int main(int argc, char* argv[]) {
  // cv::CommandLineParser parser(argc, argv,
  //   "{ a | 100 | amount of objects(must be square number) }"
  //   "{ s | 3.0-7.0 | borders of object's size(mm) }"
  //   "{ c | 5.0-25.0 | borders of contrast ratio(HU) }"
  //   "{ b | 0.0 | blur }");

  // const auto a = parser.get<int>("a");
  // if(a <= 0) std::cerr << "amount of objects(a) must be positive" << std::endl;
  // if(std::pow((int)std::sqrt(a), 2) != a)  std::cerr << "amount of objects(a) must be square number" << std::endl;
  // const auto b = parser.get<int>("b");
  // if(b < 0) std::cerr << "blur(b) must not be negative" << std::endl;

  // std::pair<double, double> s, c;
  // try {
  //   s = getPair(parser.get<std::string>("s"));
  //   c = getPair(parser.get<std::string>("c"));
  // } catch(const std::invalid_argument& e) {
  //   std::cerr << "error while parsing variables: " << e.what() << std::endl;
  // }
  

  // cv::Mat canvas(512, 512, CV_8UC1, cv::Scalar(126));;
  // generate(canvas, a, b, s, c);

  // cv::namedWindow("Image", cv::WINDOW_NORMAL);
  // cv::resizeWindow("Image", 1200, 1200);
  // cv::imshow("Image", canvas);
  // cv::waitKey(0);

  return 0;
}