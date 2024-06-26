#include <iostream>
#include <vector>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <random>
#include "json.hpp"

using json = nlohmann::json;

std::vector<cv::Vec4i> groundTruthBlobs;

cv::Mat canvas;
cv::Mat object;
int a = 100, b = 50, noiseLevel = 18;
std::pair<double, double> s = { 1.0, 8.0 };
std::pair<double, double> c = { 1.0, 25.0 };
int blockSize = 100;

void generateAndProcess(cv::Mat& canvas, int a, int b, std::pair<double, double>& s, std::pair<double, double>& c, int noiseLevel, std::vector<cv::Vec4i>& groundTruthCircles) {
    int rows = std::sqrt(a);
    double sizeStep = (s.second - s.first) / rows;
    double contrastStep = (c.second - c.first) / rows;
    canvas = cv::Mat(1024, 1024, CV_8UC1, cv::Scalar(127));

    for (int i = 0; i < rows; i++) {
        double currentSize = s.first + i * sizeStep;
        for (int j = 0; j < rows; j++) {
            double currentContrast = c.first + j * contrastStep;
            int y = (canvas.cols / rows) * j + canvas.cols / (2 * rows);
            int x = (canvas.rows / rows) * i + canvas.rows / (2 * rows);
            int radius = currentSize * (canvas.cols / 100) / 2;
            int intensity = 154 + currentContrast * (255 / 100);

            cv::circle(canvas, cv::Point(x, y), radius, cv::Scalar(intensity), -1);
            groundTruthCircles.push_back(cv::Vec4i(x, y, radius, intensity));
        }
    }

    if (b % 2 == 0) b += 1;
    if (b < 1) b = 1;
    cv::GaussianBlur(canvas, canvas, cv::Size(b, b), 0);

    cv::Mat mGaussian_noise = cv::Mat(canvas.size(), CV_8UC1);
    cv::randn(mGaussian_noise, 0, noiseLevel);
    canvas += mGaussian_noise;
    normalize(canvas, canvas, 0, 255, cv::NORM_MINMAX, CV_8UC1);
}

cv::Mat makeDoGImage(const cv::Mat& img, int r) {
    double sigma = r / 1.4142;
    double sigma1 = sigma / 1.4142;
    double sigma2 = sigma * 1.4142;

    cv::Mat img_float, first_gaussian, second_gaussian, result;
    img.convertTo(img_float, CV_32FC1);
    cv::GaussianBlur(img_float, first_gaussian, cv::Size(0, 0), sigma1, 0, cv::BORDER_REPLICATE);
    cv::GaussianBlur(img_float, second_gaussian, cv::Size(0, 0), sigma2, 0, cv::BORDER_REPLICATE);
    cv::Mat difference;
    cv::subtract(first_gaussian, second_gaussian, difference);

    cv::normalize(difference, result, 0, 255, cv::NORM_MINMAX, CV_8UC1);
    return difference;
}

void makeDoGDetection(const cv::Mat& img, std::vector<cv::Vec3i>& circles) {
    cv::Mat detection_result;
    cv::cvtColor(img, detection_result, cv::COLOR_GRAY2BGR);

    int radius = 6;

    while (radius <= 40) {
        cv::Mat difference = makeDoGImage(img, radius);

        std::vector<cv::Point> localMaxima;
        int padding = 10;
        for (int y = padding; y < difference.rows - padding; ++y) {
            for (int x = padding; x < difference.cols - padding; ++x) {
                float currentValue = difference.at<float>(y, x);
                if (currentValue > 10) {
                    bool isLocalMaximum = true;
                    for (int dy = -9; dy <= 9; ++dy) {
                        for (int dx = -9; dx <= 9; ++dx) {
                            if (difference.at<float>(y + dy, x + dx) > currentValue) {
                                isLocalMaximum = false;
                                break;
                            }
                        }
                        if (!isLocalMaximum) {
                            break;
                        }
                    }
                    if (isLocalMaximum) {
                        localMaxima.push_back(cv::Point(x, y));
                    }
                }
            }
        }
        std::sort(localMaxima.begin(), localMaxima.end(), [&difference](const cv::Point& a, const cv::Point& b) {
            return difference.at<float>(a.y, a.x) > difference.at<float>(b.y, b.x);
        });
        if (localMaxima.size() > 15) {
            localMaxima.resize(15);
        }

        for (const auto& point : localMaxima) {
            cv::circle(detection_result, point, radius, cv::Scalar(0, 0, 255), 1);
            circles.push_back({ point.x, point.y, radius });
        }
        radius += 2;
    }

    cv::imwrite("detection_result.png", detection_result);
}


void getThreshold(cv::Mat& canvas) {
    cv::medianBlur(canvas, canvas, 15);
    if(blockSize % 2 == 0) blockSize++;
    if(blockSize < 1) blockSize = 1;
    cv::adaptiveThreshold(canvas, canvas, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, blockSize, -8);
}

double IoU(const cv::Vec3i& circleA, const cv::Vec3i& circleB) {
    int xA = circleA[0];
    int yA = circleA[1];
    int rA = circleA[2];
    int xB = circleB[0];
    int yB = circleB[1];
    int rB = circleB[2];

    double distance = std::sqrt((xA - xB) * (xA - xB) + (yA - yB) * (yA - yB));
    if (distance >= rA + rB) {
        return 0.0;
    } else if (distance <= std::abs(rA - rB)) {
        int minRadius = std::min(rA, rB);
        return M_PI * minRadius * minRadius;
    } else {
        double rA2 = rA * rA;
        double rB2 = rB * rB;
        double d2 = distance * distance;
        double alpha = std::acos((rA2 + d2 - rB2) / (2 * rA * distance));
        double beta = std::acos((rB2 + d2 - rA2) / (2 * rB * distance));
        double intersection = rA2 * alpha + rB2 * beta - 0.5 * std::sqrt((-distance + rA + rB) * (distance + rA - rB) * (distance - rA + rB) * (distance + rA + rB));
        double unionArea = M_PI * rA2 + M_PI * rB2 - intersection;
        return intersection / unionArea;
    }
}

void evaluateDetection(const std::vector<cv::Vec4i>& groundTruth, const std::vector<cv::Vec3i>& detectedCircles, std::vector<double>& results) {
    for (double threshold = 0.0; threshold <= 1.0; threshold += 0.1) {
        int TP = 0, FP = 0, FN = 0;
        std::vector<bool> matched(detectedCircles.size(), false);

        for (const auto& gt : groundTruth) {
            bool found = false;
            for (size_t i = 0; i < detectedCircles.size(); i++) {
                if (matched[i]) continue;
                if (IoU(cv::Vec3i(gt[0], gt[1], gt[2]), detectedCircles[i]) > threshold) {
                    TP++;
                    matched[i] = true;
                    found = true;
                    break;
                }
            }
            if (!found) FN++;
        }

        for (size_t i = 0; i < detectedCircles.size(); i++) {
            if (!matched[i]) FP++;
        }

        double jaccardIndex = static_cast<double>(TP) / (TP + FP + FN);
        results.push_back(jaccardIndex);

        std::cout << "Threshold: " << threshold << ", TP: " << TP << ", FP: " << FP << ", FN: " << FN << ", IoU: " << jaccardIndex << std::endl;
    }
}

void saveToJson(const std::string& filename, const cv::Mat& image, const std::vector<cv::Vec4i>& groundTruth, const std::vector<cv::Vec3i>& detectedCircles, const std::vector<double>& results) {
    json j;
    j["data"]["objects"] = json::array();
    for (const auto& circle : groundTruth) {
        json obj;
        obj["p"] = { circle[0], circle[1], circle[2] };
        obj["c"] = circle[3];
        j["data"]["objects"].push_back(obj);
    }

    j["data"]["background"] = {
        {"size", {image.size().width, image.size().height}},
        {"color", 127},
        {"blur", b},
        {"noise", noiseLevel}
    };

    std::ofstream file(filename);
    file << j.dump(4);
}

std::pair<double, double> getPairFromTrackbar(int low, int high) {
    return { static_cast<double>(low) / 10.0, static_cast<double>(high) / 10.0 };
}

bool stop;

void onTrackbarChange(int, void*) {
    if(!stop) {
        s = getPairFromTrackbar(static_cast<int>(s.first * 10), static_cast<int>(s.second * 10));
        c = getPairFromTrackbar(static_cast<int>(c.first * 10), static_cast<int>(c.second * 10));
        groundTruthBlobs.clear();
        generateAndProcess(canvas, a, b, s, c, noiseLevel, groundTruthBlobs);
        cv::imshow("Image", canvas);

        cv::Mat thresholdCanvas = canvas.clone();
        getThreshold(thresholdCanvas);
        cv::imshow("Image threshold", thresholdCanvas);

        std::vector<cv::Vec3i> detectedCircles;
        makeDoGDetection(thresholdCanvas, detectedCircles);

        cv::Mat displayCanvas = canvas.clone();
        for (const auto& circle : detectedCircles) {
            cv::circle(displayCanvas, cv::Point(circle[0], circle[1]), circle[2], cv::Scalar(0, 255, 0), 2);
        }
        cv::imshow("Detected Circles", displayCanvas);

        std::vector<double> results;
        evaluateDetection(groundTruthBlobs, detectedCircles, results);

        saveToJson("config.json", canvas, groundTruthBlobs, detectedCircles, results);
    }
}

void onBlockSizeChange(int pos, void* userdata) {
    blockSize = pos;
    onTrackbarChange(0, nullptr);
}

void onAChange(int pos, void* userdata) {
    a = pos;
    onTrackbarChange(0, nullptr);
}

void onSLowChange(int pos, void* userdata) {
    s = getPairFromTrackbar(pos, static_cast<int>(s.second * 10));
    onTrackbarChange(0, nullptr);
}

void onSHighChange(int pos, void* userdata) {
    s = getPairFromTrackbar(static_cast<int>(s.first * 10), pos);
    onTrackbarChange(0, nullptr);
}

void onCLowChange(int pos, void* userdata) {
    c = getPairFromTrackbar(pos, static_cast<int>(c.second * 10));
    onTrackbarChange(0, nullptr);
}

void onCHighChange(int pos, void* userdata) {
    c = getPairFromTrackbar(static_cast<int>(c.first * 10), pos);
    onTrackbarChange(0, nullptr);
}

void onBChange(int pos, void* userdata) {
    b = pos;
    onTrackbarChange(0, nullptr);
}

void onNoiseLevelChange(int pos, void* userdata) {
    noiseLevel = pos;
    onTrackbarChange(0, nullptr);
}

int main() {
    groundTruthBlobs.clear();
    generateAndProcess(canvas, a, b, s, c, noiseLevel, groundTruthBlobs);

    cv::namedWindow("Image", cv::WINDOW_NORMAL);
    cv::imshow("Image", canvas);

    cv::namedWindow("Image threshold", cv::WINDOW_NORMAL);
    cv::createTrackbar("Block Size", "Image threshold", nullptr, 255, onBlockSizeChange);

    cv::setTrackbarPos("Block Size", "Image threshold", blockSize);

    cv::namedWindow("Settings", cv::WINDOW_NORMAL);
    int sLow = static_cast<int>(s.first * 10);
    int sHigh = static_cast<int>(s.second * 10);
    int cLow = static_cast<int>(c.first * 10);
    int cHigh = static_cast<int>(c.second * 10);
    cv::createTrackbar("A", "Settings", nullptr, 100, onAChange);
    cv::createTrackbar("S Low", "Settings", nullptr, 100, onSLowChange);
    cv::createTrackbar("S High", "Settings", nullptr, 100, onSHighChange);
    cv::createTrackbar("C Low", "Settings", nullptr, 100, onCLowChange);
    cv::createTrackbar("C High", "Settings", nullptr, 100, onCHighChange);
    cv::createTrackbar("B", "Settings", nullptr, 100, onBChange);
    cv::createTrackbar("Noise Level", "Settings", nullptr, 20, onNoiseLevelChange);

    stop = true;
    cv::setTrackbarPos("A", "Settings", a);
    cv::setTrackbarPos("S Low", "Settings", sLow);
    cv::setTrackbarPos("S High", "Settings", sHigh);
    cv::setTrackbarPos("C Low", "Settings", cLow);
    cv::setTrackbarPos("C High", "Settings", cHigh);
    cv::setTrackbarPos("B", "Settings", b);
    cv::setTrackbarPos("Noise Level", "Settings", noiseLevel);
    stop = false;

    cv::waitKey(0);

    return 0;
}
