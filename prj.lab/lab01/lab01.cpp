#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

void generateGammaCorrectedGradient(cv::Mat& grad, double& gamma) {
  for (auto col = 0.0; col < grad.cols; ++col) {
    auto intensity = pow(col / grad.cols, 1.0 / gamma);
    grad.col(col).setTo(cv::Scalar(255 * intensity, 255 * intensity, 255 * intensity));
  }
}

int main(int argc, char* argv[]) {
  cv::CommandLineParser parser(argc, argv,
    "{ s | 3 | width }"
    "{ h | 30 | height }"
    "{ gamma | 2.4 | gamma correction }"
    "{ output | | output filename }");

  auto s = parser.get<int>("s");
  auto h = parser.get<int>("h");
  auto g = parser.get<double>("gamma");
  auto filename = parser.get<std::string>("output");

  cv::Mat grad(h, s * 256, CV_8UC3);
  for (int col = 0; col < grad.cols; ++col) {
    auto intensity = static_cast<float>(col) / grad.cols;
    grad.col(col).setTo(cv::Scalar(255 * intensity, 255 * intensity, 255 * intensity));
  }

  cv::Mat gradGammaCorrected(h, s * 256, CV_8UC3);
  generateGammaCorrectedGradient(gradGammaCorrected, g);

  cv::Mat combined;
  cv::vconcat(grad, gradGammaCorrected, combined);

  if (!filename.empty()) {
    filename += ".jpg";
    cv::imwrite(filename, combined);
    std::cout << "Image saved" << std::endl;
  } else {
    cv::imshow("lab01", combined);
    cv::waitKey(0);
  }

  return 0;
}
