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

    enum graphModes {
      LIN = 0, 
      LOG = 1
    };

    enum lineTypes {
      LINES = 0,
      BEZIE = 1
    };

    enum windowFunctions {
      TRIANGULAR = 0,
      PARZEN = 1,
      WELCH = 2,
      SINE = 3,
      POWER_OF_SINE = 4,
      COSINE_SUM = 5,
      HANN = 6,
      HAMMING = 7,
      BLACKMAN = 8,
      NUTTALL = 9,
      BLACKMAN_NUTTALL = 10,
      BLACKMAN_HARRIS = 11,
      FLAT_TOP = 12,
      GAUSSIAN = 13,
      CONFINED_GAUSSIAN = 14,
      APPROXIMATE_CONFINED_GAUSSIAN = 15, // Generalized adaptive polynomial
      GENERALIZED_NORMAL = 16,
      TUKEY = 17,
      PLANCK_TAPER = 18,
      BARTLETT_HANN = 19,
      HANN_POISSON = 20,
      GAP = 21,
      LANCZOS = 22
    };



  class Spectrogram {

  public:
    Spectrogram() {};
    ~Spectrogram() {};

    void setAudioInfo(unsigned int audio_sample_rate, unsigned int audio_sample_size, unsigned int audio_channels);
    void setFrameSize(int size);
    void setWindowFunc(int type);


    void addWindow();

    void fft();
    void fft_c(std::complex<float> in[], size_t stride, std::complex<float> out[], size_t n);
    // остальные фурье функции
    // void dft(std::complex<float> in[], std::complex<float> out[], size_t n);


    void normalize(const int multiplier = 20);
    void drawGrid(cv::Mat& img, int type);

    void drawSpectrum(cv::Mat& img);
    // void drawSpectrum(cv::Mat& image, graphMode mode, lineType type);

    double interpolate(double from ,double to ,float percent);

    void draw_lines_simple_low_2d(cv::Mat& img);
    void draw_lines_simple_2d(cv::Mat& img);
    // void draw_log_bezie_2d(cv::Mat& img);
    // double interpolate(double from ,double to ,float percent);
    // void draw_log_lines_2d(cv::Mat& img);


    // const float freq_start = 20.0;
    // const float freq_end = 20000.0;
    // float max_db = 190;

    std::vector<std::complex<float>> in; // frame size
    std::vector<float> out; // frame size / 2
    std::vector<std::complex<float>> ft_out; // frame size
    std::vector<std::complex<float>> window_out; // frame size * 2
  
  private:
    void windowHann();
    // остальные оконные функции



    using windowFuncType = void (Spectrogram::*)();

    std::vector<windowFuncType> windows = {
        &Spectrogram::windowHann
    };


    unsigned int channels;
    unsigned int frame_size;
    unsigned int sample_rate;
    unsigned int sample_size;

  };

};



#endif // SPECTROGRAM_HPP