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
#include <map>

#include <opencv2/opencv.hpp>
#include <fftw3.h>

namespace a2i {

    enum graphModes {
      LIN = 0, 
      LOG = 1
    };

    enum lineTypes {
      LINES = 0,
      BEZIE = 1
    };

    enum fillTypes {
      NOT_FILLED = 0, 
      ONE_COLOR = 1,
      GRADIENT = 2
    };

    enum windowFunctions {
      SINE = 0,
      HANN = 1,
      HAMMING = 2,
      BLACKMAN = 3,
      NUTTALL = 4,
      BLACKMAN_NUTTALL = 5,
      BLACKMAN_HARRIS = 6,
      FLAT_TOP = 7,
      BARTLETT_HANN = 8,
      HANN_POISSON = 9
    };


  // доделать 3д, 
  // сделать типы заполнений градиент, 
  // 1 цвет на выбор или нет цвета, 
  // выбор отрисовки через енамы, добавить парочку функций фурье
  
  class Spectrogram {

  public:
    Spectrogram() {};
    ~Spectrogram() {};

    void setAudioInfo(
      unsigned int audio_sample_rate, 
      unsigned int audio_sample_size, 
      unsigned int audio_channels,
      std::pair<int, int> audio_min_max_db = {-90, 6});

    void setFreqRange(std::pair<unsigned int, unsigned int> audio_freq_range = {0, 20000});
    void setFrameSize(int size);
    void setWindowFunc(int type);
    void addWindow();

    void fftw1();
    
    // остальные фурье функции
    // void dft(std::complex<float> in[], std::complex<float> out[], size_t n);


    void normalize(const int multiplier = 20);
    void drawGrid(
      cv::Mat& img, 
      const int type, 
      const bool enable_text = true, 
      const std::vector<unsigned int>& freqs = {}, 
      const int number_of_db_risks = 10,
      const cv::Scalar line_color = cv::Scalar(79, 73, 80), 
      const cv::Scalar text_color = cv::Scalar(51, 186, 243));

    void drawSpectrum(
      cv::Mat& img, 
      const int line_type = 0, 
      const int graph_mode = 1, 
      const int fill_type = 1, 
      const cv::Scalar line_color = cv::Scalar(255, 255, 255), 
      const cv::Scalar underline_color = cv::Scalar(127, 127, 127));


    void draw_log_bezie_2d(cv::Mat& img);
    void draw_lines_simple_2d(cv::Mat& img);
    void draw_log_lines_2d(cv::Mat& img);

    // const float freq_start = 20.0;
    // const float freq_end = 20000.0;

    std::vector<float> in; // frame size
    std::vector<float> in_in; // frame size
    std::vector<float> out; // frame size / 2
    std::vector<std::complex<float>> ft_out; // frame size
    std::vector<float> window_out; // frame size * 2
  
  private:
    double interpolate(double from ,double to ,float percent);
    // double align(double angle, double period);
    // void correctFrequency(double shiftsPerFrame);

    void windowSine();
    void windowHann();
    void windowHamming();
    void windowBlackman();
    void windowNuttall();
    void windowBlackmanNuttall();
    void windowBlackmanHarris();
    void windowFlatTop();
    void windowBartlettHann();
    void windowHannPoisson();


    using windowFuncType = void (Spectrogram::*)();

    std::vector<windowFuncType> windows = {
      &Spectrogram::windowSine,
      &Spectrogram::windowHann,
      &Spectrogram::windowHamming,
      &Spectrogram::windowBlackman,
      &Spectrogram::windowNuttall,
      &Spectrogram::windowBlackmanNuttall,
      &Spectrogram::windowBlackmanHarris,
      &Spectrogram::windowFlatTop,
      &Spectrogram::windowBartlettHann,
      &Spectrogram::windowHannPoisson
    };


    unsigned int channels;
    unsigned int frame_size;
    unsigned int sample_rate;
    unsigned int sample_size;
    std::pair<int, int> min_max_db;
    float window_correction;
    std::pair<unsigned int, unsigned int> freq_range;
    std::map<double, double> freq_dictionary;
  };
};



#endif // SPECTROGRAM_HPP