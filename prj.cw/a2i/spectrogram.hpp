#ifndef SPECTROGRAM_HPP
#define SPECTROGRAM_HPP

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>

namespace a2i {

    typedef struct {
      float left;
      float right;
    } Frame;

    enum graphModes {
      LIN = 0, 
      LOG = 1
    };

    enum lineTypes {
      LINES = 0,
      BEZIE = 1
    };

    enum windowFunctions {
      // RECTANGULAR,
      // B-SPLINE,
      HANN = 0
    };

    // enum fourierTransformFunctions {
    //   //DFT
    //   FFT = 0
    // };


  class Spectrogram {

  public:
    Spectrogram() {};
    ~Spectrogram() {};

    void setFrameSize(int size);
    void setWindowFunc(int type);
    void windowHann();
    // остальные оконные функции

    void useWindowFunc();

    // void setFurierTransformFunc(int type);
    void fft(std::vector<std::complex<float>>& in, size_t stride, std::vector<std::complex<float>>& out, size_t n);
    void fft_c(std::complex<float> in[], size_t stride, std::complex<float> out[], size_t n);
    // остальные фурье функции

    void normalize(const int multiplier = 20);
    // void useFourierFunc();

    // void dft(std::complex<float> in[], std::complex<float> out[], size_t n);
      
    // void drawSpectrum(cv::Mat& image, graphMode mode, lineType type);
    // void drawGrid(cv::Mat& img, graphMode mode);
    

    using windowFuncType = void (Spectrogram::*)();

    std::vector<windowFuncType> windows = {
        &Spectrogram::windowHann
    };

    // void drawGrid(cv::Mat& img, a2i::LOG);
    void drawGrid(cv::Mat& img);

    void drawSpectrum(cv::Mat& img);
    double interpolate(double from ,double to ,float percent);

    // void draw_log_bezie_2d(cv::Mat& img);
    // double interpolate(double from ,double to ,float percent);
    // void draw_log_lines_2d(cv::Mat& img);
    // void draw_lines_simple_low_2d(cv::Mat& img);
    // void draw_lines_simple_2d(cv::Mat& img);

    unsigned int dimension = 2;
    unsigned int frame_size = 512;
    unsigned int sample_rate = 41000;
    unsigned int sample_size = 16;

    // const float freq_start = 20.0;
    // const float freq_end = 20000.0;
    // float max_db = 190;

    std::vector<std::complex<float>> in; // frame size
    std::vector<float> out; // frame size / 2
    std::vector<std::complex<float>> ft_out; // frame size
    std::vector<std::complex<float>> window_out; // frame size * 2
  };

};



#endif // SPECTROGRAM_HPP