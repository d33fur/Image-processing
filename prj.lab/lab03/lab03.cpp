#include <iostream>
#include <vector>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

std::pair<int, int> minMax = {255, 0};

void getBorders(cv::Mat& image, const std::pair<double, double>& q, 
  std::pair<int, int>& borders) {
  cv::Mat hist;
  int histSize = 256;
  float range[] = {0, 256};
  const float* histRange = {range};
  cv::calcHist(&image, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange, 1, 0);

  float total = cv::sum(hist)[0];
  float sumLeft = 0, sumRight = 0;
  borders = {0, 255};

  for(int i = 0, j = histSize - 1; i < histSize || j >= 0; i++, j--) {
    sumLeft += hist.at<float>(i);
    sumRight += hist.at<float>(j);

    if(q.first <= sumLeft / total && i < histSize) {
      borders.first = i;
      i = histSize;
    }
    if(1 - q.second <= sumRight / total && j >= 0) {
      borders.second = j;
      j = -1;
    }
  }

  minMax = {std::min(borders.first, minMax.first), std::max(borders.second, minMax.second)};
}


// для чб
void autocontrast1(cv::Mat& image, cv::Mat& canvas, 
  const std::pair<double, double>& q) {
  cv::Mat grayImage;
  cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
  std::pair<int, int> borders;
  getBorders(grayImage, q, borders);
  cv::normalize(grayImage, canvas, borders.first - 127, borders.second - 127, cv::NORM_MINMAX);
}

// для цветного
void autocontrast3(cv::Mat& image, cv::Mat& canvas, 
  const std::pair<double, double>& q) {

  std::vector<cv::Mat> imageVec, combined(3), separated(3);
  cv::split(image, imageVec);

  std::vector<std::pair<int, int>> borders(3);
  for(int i = 0; i < 3; i++) {
    getBorders(imageVec[i], q, borders[i]);
    std::cout << borders[i].first << ' ' << borders[i].second << std::endl;
  }

  for(int i = 0; i < 3; i++) {
    cv::normalize(imageVec[i], combined[i], minMax.first - 127, minMax.second - 127, cv::NORM_MINMAX);
    cv::normalize(imageVec[i], separated[i], borders[i].first - 127, borders[i].second - 127, cv::NORM_MINMAX);
  }

  std::vector<cv::Mat> canvases(2);
  cv::merge(combined, canvases[0]);
  cv::merge(separated, canvases[1]);
  cv::vconcat(canvases, canvas);
}

// определение метода
void chooseMethod(cv::Mat image, std::function<void(cv::Mat&, cv::Mat&, 
  const std::pair<double, double>&)>& method) {
  if(image.channels() == 1) {
    method = autocontrast1;
  } else {
    bool is1Channel = true;
    for(int i = 0; i < image.rows; i++) {
      for(int j = 0; j < image.cols; j++) {
        cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
        if(pixel[0] != pixel[1] || pixel[0] != pixel[2]) {
          is1Channel = false;
          break;
        }
      }
    }

    is1Channel ? method = autocontrast1 : method = autocontrast3;
  }
}


int main(int argc, char* argv[]) {
  cv::CommandLineParser parser(argc, argv,
    "{ path |  | path to image }"
    "{ q1 | 0.0 | left quantile }"
    "{ q2 | 1.0 | right quantile }");

  if(!parser.has("path")) {
    std::cerr << "No image path" << std::endl;
    return 0;
  }

  const auto path = parser.get<std::string>("path");
  const std::pair<double, double> q = {parser.get<double>("q1"), parser.get<double>("q2")};

  if(q.first > q.second) {
    std::cerr << "Left quantile is bigger than right quantile" << std::endl;
    return 0;
  }

  std::vector<cv::Mat> canvases(2);
  try {
    canvases[0] = cv::imread(path);  
  } catch(const std::exception& e) {
    std::cerr << "Error while reading image: " << e.what() << std::endl;
    return 0;  
  }

  std::function<void(cv::Mat&, cv::Mat&, const std::pair<double, double>&)> autocontrast;
  chooseMethod(canvases[0], autocontrast);
  autocontrast(canvases[0], canvases[1], q);

  cv::Mat canvas;
  cv::vconcat(canvases, canvas);

  cv::namedWindow("Image", cv::WINDOW_NORMAL);
  cv::resizeWindow("Image", 1200, 1200);
  cv::imshow("Image", canvas);
  cv::waitKey(0);

  return 0;
}
