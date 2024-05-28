#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <complex.h>
#include <math.h>
#include <algorithm>

#include "raylib.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

int FRAME_SIZE = 1024;
int NUM_CHANNELS = 2;
float SAMPLE_RATE = 41000.0;
int SAMPLE_SIZE = 16;

const float FREQ_ZERO = 0.0;
const float FREQ_START = 20.0;
const float FREQ_END = 20000.0;

int WINDOW_WIDTH = 1600;
int WINDOW_HEIGHT = 1200;


float maxDb = 190;
float pos = 0;
float pos1 = 0;
int step = 1;

#define N 512
std::complex<float> in[N];
std::complex<float> out[N];
std::complex<float> hannOut[N*2];
float powerOut[N / 2];


typedef struct {
  float left;
  float right;
} Frame;

void draw_lines_simple_3d(cv::Mat& img) {
  for(size_t i = 0; i < N / 2; i += step) {
    pos = std::max(0., (i / (double)(N/2)) * WINDOW_WIDTH);
    double y1;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << (i * SAMPLE_RATE / 2) / (N / 2);
    if(powerOut[i] < 0) {
      y1 = (WINDOW_HEIGHT / 2) + ((std::abs(powerOut[i]) / maxDb)) * (WINDOW_HEIGHT / 2);
      cv::line(img, cv::Point(pos, (WINDOW_HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
    else {
      y1 = (1 - (powerOut[i] / maxDb)) * (WINDOW_HEIGHT / 2);
      cv::line(img, cv::Point(pos, (WINDOW_HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
  }
}

void draw_lines_simple_2d(cv::Mat& img) {
  for(size_t i = 0; i < N / 2; i += step) {
    pos = std::max(0., (i / (double)(N/2)) * WINDOW_WIDTH);
    double y1;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << (i * SAMPLE_RATE / 2) / (N / 2);
    if(powerOut[i] < 0) {
      y1 = (WINDOW_HEIGHT / 2) + ((std::abs(powerOut[i]) / maxDb)) * (WINDOW_HEIGHT / 2);
      cv::line(img, cv::Point(pos, (WINDOW_HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
    else {
      y1 = (1 - (powerOut[i] / maxDb)) * (WINDOW_HEIGHT / 2);
      cv::line(img, cv::Point(pos, (WINDOW_HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
  }
}

void draw_lines_simple_low_2d(cv::Mat& img) {
  for(size_t i = 0; i < N / 2; i += step) {
    pos = std::max(0., (i / (double)(N/2)) * WINDOW_WIDTH);
    double y1;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << (i * SAMPLE_RATE / 2) / (N / 2);
    if(powerOut[i] > 0) {
      y1 = (1 - (powerOut[i] / maxDb)) * WINDOW_HEIGHT;
      cv::line(img, cv::Point(pos, WINDOW_HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
  }
}

void draw_log_lines_2d(cv::Mat& img) {
  // spectrum.insert(spectrum.begin(), 0);
  for(size_t i = 0; i < N / 2; i += step) {
    // int temp = (i * SAMPLE_RATE / 2) / (N/2);
    // if(temp >= FREQ_START && temp <= FREQ_END) {
    //   if(powerOut[i] > maxDb) std::cout << powerOut[i] << " >maxDb"  << std::endl;

      pos = std::max(0., (double)(std::log2(i * (SAMPLE_RATE / 2) / (N/2)) / std::log2(SAMPLE_RATE / 2)) * WINDOW_WIDTH);
      pos1 = std::max(0., (double)(std::log2((i + step) * (SAMPLE_RATE / 2) / (N/2)) / std::log2(SAMPLE_RATE / 2)) * WINDOW_WIDTH);

      std::stringstream ss;
      ss << std::fixed << std::setprecision(1) << powerOut[i];
  
      int y1 = (1 - (powerOut[i] / maxDb)) * WINDOW_HEIGHT;

      if (i + step < N/2) {
        int y2 = (1 - (powerOut[i + step] / maxDb)) * WINDOW_HEIGHT;

        cv::Point vertices[4];
        vertices[0] = cv::Point(pos, y1);
        vertices[1] = cv::Point(pos1, y2);
        vertices[2] = cv::Point(pos1, WINDOW_HEIGHT);
        vertices[3] = cv::Point(pos, WINDOW_HEIGHT);

        cv::Scalar transparentColor(65, 56, 63);
        cv::fillConvexPoly(img, vertices, 4, transparentColor, cv::LINE_8);

        cv::line(img, cv::Point(pos, y1), cv::Point(pos1, y2), cv::Scalar(255, 255, 255), 1);
        cv::putText(img, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
      }
      // cv::putText(img, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1);
      // cv::line(img, cv::Point(pos, HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    // }
  }
}

double interpolate(double from ,double to ,float percent) {
  double difference = to - from;
  return from + ( difference * percent );
}

void draw_log_bezie_2d(cv::Mat& img) {
  // std::vector<cv::Point> control_points = {cv::Point(0.0, WINDOW_HEIGHT)};
  std::vector<cv::Point> control_points;
  // std::cout << spectrum.size() << std::endl;
  for(size_t i = 0; i < N / 2; i += step) {
    pos = std::max(0., (double)(std::log2(i * (SAMPLE_RATE / 2) / (N / 2)) / std::log2((SAMPLE_RATE / 2))) * WINDOW_WIDTH);
    int y = (1 - powerOut[i] / maxDb) * WINDOW_HEIGHT / 2;

    control_points.push_back(cv::Point(pos, y));
  }
  float sstep = 0.01;

  for(size_t i = 2; i < control_points.size() - 1; i+=3) {
    if(i < 3) {
      sstep = 0.0001;
    }
    else if(i < 6) {
      sstep = 0.001;
    }
    else if(i < 55) {
      sstep = 0.01;
    }
    else {
      sstep = 0.1;
    }

    cv::Point p0 = control_points[i - 2];
    cv::Point p1 = control_points[i - 1];
    cv::Point p2 = control_points[i];
    cv::Point p3 = control_points[i + 1];

    for(float i = 0; i < 1; i += sstep){
      // The Green Lines

      auto xa = interpolate(p0.x, p1.x ,i);
      auto ya = interpolate(p0.y, p1.y ,i);
      auto xb = interpolate(p1.x, p2.x ,i);
      auto yb = interpolate(p1.y, p2.y ,i);
      auto xc = interpolate(p2.x, p3.x ,i);
      auto yc = interpolate(p2.y, p3.y ,i);

      // The Blue Line

      auto xm = interpolate(xa, xb, i);
      auto ym = interpolate(ya, yb, i);
      auto xn = interpolate(xb, xc, i);
      auto yn = interpolate(yb, yc, i);

      // The Black Dot

      auto x = interpolate(xm ,xn ,i);
      auto y = interpolate(ym ,yn ,i);

      // std::cout << x << ' ' << y << std::endl;
      if(x > WINDOW_WIDTH) x = WINDOW_WIDTH;
      if(y > WINDOW_HEIGHT) y = WINDOW_HEIGHT;
      if(x < 0) x = 0;
      if(y < 0) y = 0;
      // std::cout << x << " " << y << std::endl;
      img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(255, 255, 255);

      // cv::line(img, cv::Point(x, y), cv::Point(x, WINDOW_HEIGHT), cv::Scalar(79, 73, 80), 1);
    }
    // sstep+=0.001;
  }
}

void drawLogScaleLines(cv::Mat& img) {
  float minDb = -120;
  float maxDb = 0;
  cv::Scalar lineColor(79, 73, 80);
  cv::Scalar textColor(51, 186, 243);
  std::vector<int> freqRisks = {0, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, (int)(SAMPLE_RATE / 2)};
  int num = freqRisks.size();

  for(int i = 0; i < num; i++) {
    float freq = freqRisks[i];
    float db = minDb + i * (maxDb - minDb) / num;
    float x = std::max(0.0, (std::log2(freqRisks[i]) / std::log2((SAMPLE_RATE / 2))) * WINDOW_WIDTH);
    int y = (1 - (db - minDb) / (maxDb - minDb)) * WINDOW_HEIGHT;

    cv::line(img, cv::Point(x, 0), cv::Point(x, WINDOW_HEIGHT), lineColor, 1);

    std::stringstream ss;
    ss << (int)freq;
    cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

    cv::line(img, cv::Point(0, y), cv::Point(WINDOW_WIDTH, y), lineColor, 1);

    std::stringstream sss;
    sss << (int)db;
    cv::putText(img, sss.str(), cv::Point(15, y + 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);
  }
}

void windowHann(std::complex<float> out[], size_t n) {
  float ND = static_cast<float>(n);

  for(size_t i = 0; i < n; ++i) {
    float nD = static_cast<float>(i);
    float real = 0.5 * (1.0 - cos((2.0 * M_PI * nD) / ND));
    out[i] = std::complex<float>(real, 0.0);
  }
}

void dft(std::complex<float> in[], std::complex<float> out[], size_t n) {
  std::complex<float> sum;

  for(size_t i = 0; i < n; ++i) {
    sum = std::complex<float>(0.0, 0.0);

    for(size_t j = 0; j < n; ++j) {
      float real = cos(((2*M_PI)/n) * i * j);
      float imag = sin(((2*M_PI)/n) * i * j);
      sum += in[j] * std::complex<float>(real, -imag);
    }

    out[i] = (sum);
  }
}

void fft(std::complex<float> in[], size_t stride, std::complex<float> out[], size_t n) {
  assert(n > 0);

  if(n == 1) {
    out[0] = in[0];
    return;
  }

  fft(in, stride * 2, out, n / 2);
  fft(in + stride, stride* 2, out + n / 2, n / 2);

  for(size_t i = 0; i < n / 2; i++) {
    float t = (float)i / n;
    std::complex<float> v = (std::complex<float>)cexp(-2 * I * M_PI * t) * out[i + n/2];
    std::complex<float> e = out[i];
    out[i] = e + v;
    out[i + n / 2] = e - v;
  }
}

// void callbackX2(void *bufferData, unsigned int frames) {
//   if(frames < N) return;

//   Frame *fs = static_cast<Frame*>(bufferData);

//   for(size_t i = 0; i < frames; ++i) {
//     in[i] = (fs[i].left + fs[i].right) * hannOut[i];
//   }

  
//   // dft(in, out, N);
//   fft(in, 1, out, N);


//   for(size_t i = 0; i < N / 2; ++i) {
//     powerOut[i] = 20 * std::log10(std::norm(out[i]) / SAMPLE_SIZE); // можно делить на sample size
//   }
// }

void callback(void *bufferData, unsigned int frames) {
  if(frames < N) return;

  Frame *fs = static_cast<Frame*>(bufferData);

  for(size_t i = 0; i < frames; ++i) {
    in[i] = (fs[i].left + fs[i].right) * hannOut[i];
  }

  
  // dft(in, out, N);
  fft(in, 1, out, N);


  for(size_t i = 0; i < N / 2; ++i) {
    powerOut[i] = 20 * std::log10(std::norm(out[i]) / SAMPLE_SIZE); // можно делить на sample size
  }
}


int main(int argc, char** argv) {
  // if(argc != 3) {
  //   std::cout << "Usage: ./program_name file_path" << std::endl;
  //   return 1;
  // }
  cv::CommandLineParser parser(argc, argv,
  "{ path |  | path to image }"
  "{ method | 2d_simple_lines | 2d_simple_lines/2d_simple_lines_low/2d_log_lines/2d_log_bezie }");

  void (*draw)(cv::Mat&);

  auto method = parser.get<std::string>("method");

  if(method == "2d_simple_lines") {
    draw = draw_lines_simple_2d;
  }
  else if(method == "2d_simple_lines_low") {
    draw = draw_lines_simple_low_2d;
  }
  else if(method == "2d_log_lines") {
    draw = draw_log_lines_2d;
  }
  else if(method == "2d_log_bezie") {
    draw = draw_log_bezie_2d;
  }
  else return 0;

    //2d_draw_log_bezie(spectrumImage);
    // 2d_draw_log_lines(spectrumImage);
    // 2d_draw_lines_simple(spectrumImage);
    // 2d_draw_lines_simple_low(spectrumImage);

  auto file = parser.get<std::string>("path");
  const char *filePath = file.c_str();

  cv::namedWindow("Spectrum original", cv::WINDOW_NORMAL);
  cv::resizeWindow("Spectrum original", WINDOW_WIDTH, WINDOW_HEIGHT);
  // cv::imshow("Spectrum original", spectrumImage);

  windowHann(hannOut, N*2);

  InitWindow(800, 600, "Spectrum");
  SetTargetFPS(60);

  InitAudioDevice();
  // SetAudioStreamBufferSizeDefault(1024); не меняет
  Music music = LoadMusicStream(filePath);
  PlayMusicStream(music);
  SetMusicVolume(music, 0.5f);
  SAMPLE_RATE = music.stream.sampleRate;
  SAMPLE_SIZE = music.stream.sampleSize;

  AttachAudioStreamProcessor(music.stream, callback);

  float timePlayed = 0.0f;

  while(!WindowShouldClose()) {
    UpdateMusicStream(music);

    if(IsKeyPressed(KEY_SPACE)) {
      if(IsMusicStreamPlaying(music)) {
        PauseMusicStream(music);
      }
      else {
        ResumeMusicStream(music);
      }
    }

    if(IsKeyPressed(KEY_P)) {
      if(IsMusicStreamPlaying(music)) {
        StopMusicStream(music);
        PlayMusicStream(music);
      }
    }

    timePlayed = GetMusicTimePlayed(music)/GetMusicTimeLength(music);
    if (timePlayed > 1.0f) {
      break;
    }
    cv::Mat spectrumImage(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
    drawLogScaleLines(spectrumImage);

    draw(spectrumImage);
    // 2d_draw_log_bezie(spectrumImage);
    // 2d_draw_log_lines(spectrumImage);
    // 2d_draw_lines_simple(spectrumImage);
    // 2d_draw_lines_simple_low(spectrumImage);
    
    cv::imshow("Spectrum original", spectrumImage);
    cv::waitKey(1);

    // int w = GetRenderWidth();
    // int h = GetRenderHeight();
    BeginDrawing();
    ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
    // float cell_width = (float)w/N;
    // for(size_t i = 0; i < N / 2; ++i) {
    //   // std::cout << out[i] << std::endl;
    //   float t = powerOut[i];
    //   DrawRectangle(i*cell_width, h/2 - h/2*t, 1, h/2*t, RED);
    // }
    EndDrawing();

  }



  UnloadMusicStream(music);
  CloseAudioDevice();
  CloseWindow();

  cv::waitKey(0);
  return 0;
}

