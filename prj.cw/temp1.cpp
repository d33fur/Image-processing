#include <iostream>
#include <string>

#include <vector>
#include <cmath>
#include <complex>
#include <iostream>
#include <valarray>
#include <chrono>
#include <algorithm>
#include <thread>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <fftw3.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#define FRAME_SIZE 1024
#define NUM_CHANNELS 2
double SAMPLE_RATIO;
int SAMPLE_RATE = 41000;

#define FREQ_START 20
#define FREQ_END 20000

int WIDTH = 1600;
int HEIGHT = 1600;

cv::Mat spectrumImage(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
cv::Mat tempImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);

typedef struct {
  double* in;
  double* out;
  fftw_plan plan;
  // int start = std::ceil(SAMPLE_RATIO * FREQ_START);
  // int size = min(
  //   std::ceil(SAMPLE_RATIO * FREQ_END),
  //   FRAME_SIZE / 2.0
  //   ) - start;
  int start;
  int size;
} AudioDataFrame;

static AudioDataFrame* spectrogramFrame;

static inline float min(float a, float b) {
  return a < b ? a : b;
}

double interpolate(double from ,double to ,float percent) {
  double difference = to - from;
  return from + ( difference * percent );
}

// void draw_bezie(double maxDb, int step) {
//   std::vector<cv::Point> control_points = {cv::Point(0., HEIGHT)};
//   // double sampleRatio = FRAME_SIZE / SAMPLE_RATE;
//   // auto start = std::ceil(sampleRatio * FREQ_START);
//   // auto size = min(std::ceil(sampleRatio * FREQ_END), FRAME_SIZE) - start;

//   for(int i = 0; i < FRAME_SIZE; i += step) {
//     // double proportion = std::pow(i / (double)WIDTH, 6);
//     // int freq = spectrum[(int)(start + proportion * size)];

//     double pos = std::max(0., (std::log2(i * 20000 / FRAME_SIZE) / std::log2(20000)) * WIDTH);
//     int y1 = (1 - spectrogramFrame->out[i] / maxDb) * HEIGHT;

//     control_points.push_back(cv::Point(pos, y1));
//   }

//   for(int i = 2; i < FRAME_SIZE - 1; i+=3) {
//     cv::Point p0 = control_points[i - 2];
//     cv::Point p1 = control_points[i - 1];
//     cv::Point p2 = control_points[i];
//     cv::Point p3 = control_points[i + 1];

//     for(float j = 0; j < 1; j += 0.01){
//       // The Green Lines

//       auto xa = interpolate(p0.x, p1.x ,j);
//       auto ya = interpolate(p0.y, p1.y ,j);
//       auto xb = interpolate(p1.x, p2.x ,j);
//       auto yb = interpolate(p1.y, p2.y ,j);
//       auto xc = interpolate(p2.x, p3.x ,j);
//       auto yc = interpolate(p2.y, p3.y ,j);

//       // The Blue Line

//       auto xm = interpolate(xa, xb, j);
//       auto ym = interpolate(ya, yb, j);
//       auto xn = interpolate(xb, xc, j);
//       auto yn = interpolate(yb, yc, j);

//       // The Black Dot

//       auto x = interpolate(xm ,xn ,j);
//       auto y = interpolate(ym ,yn ,j);

//       // std::cout << x << ' ' << y << std::endl;
//       tempImage.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(255, 255, 255);
//       // std::cout << 'p' << std::endl;
//     }
//   }
// }

void draw_lines(double maxDb, int step) {
  // spectrum.insert(spectrum.begin(), 0);
  // for(int i = 0; i < FRAME_SIZE; i += step) {
  //   double pos = std::max(0., (std::log2(i * 20000 / (FRAME_SIZE-1)) / std::log2(20000)) * WIDTH);
  //   // pos1 = std::max(0., (std::log2((i + step) * 20000 / (spectrum.size()-1)) / std::log2(20000)) * WIDTH);

  //   std::stringstream ss;
  //   ss << std::fixed << std::setprecision(1) << spectrogramFrame->out[i];
  //   int y1 = (1 - std::abs(spectrogramFrame->out[i]) / maxDb) * HEIGHT;

  //   // if (i + step < spectrum.size()) {
  //   //   int y2 = (1 - std::abs(spectrum[i + step]) / maxDb) * HEIGHT;

  //   //   cv::Point vertices[4];
  //   //   vertices[0] = cv::Point(pos, y1);
  //   //   vertices[1] = cv::Point(pos1, y2);
  //   //   vertices[2] = cv::Point(pos1, HEIGHT);
  //   //   vertices[3] = cv::Point(pos, HEIGHT);

  //   //   cv::Scalar transparentColor(65, 56, 63);
  //   //   cv::fillConvexPoly(tempImage, vertices, 4, transparentColor, cv::LINE_8);

  //   //   cv::line(tempImage, cv::Point(pos, y1), cv::Point(pos1, y2), cv::Scalar(255, 255, 255), 1);
  //   //   cv::putText(tempImage, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
  //   // }
  //   cv::putText(tempImage, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1);
  //   cv::line(tempImage, cv::Point(pos, HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
  // }

    for(int i = 0; i < WIDTH; i += step) {
    double proportion = std::pow(i / (double)WIDTH, 4);
    double freq = spectrogramFrame->out[(int)(spectrogramFrame->start
            + proportion * spectrogramFrame->size)];
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << spectrogramFrame->out[i];
    // std::cout << freq << std::endl;

    cv::putText(tempImage, ss.str(), cv::Point(i, freq + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1);
    cv::line(tempImage, cv::Point(i, HEIGHT), cv::Point(i, freq), cv::Scalar(127, 127, 127), 1);
  }
}

void drawScaleLines() {
  auto minFreq = 0.;
  auto maxFreq = 20000.;
  double minDb = -110;
  double maxDb = 0;
  cv::Scalar lineColor(79, 73, 80);
  cv::Scalar textColor(51, 186, 243);
  std::vector<double> freqRisks = {0, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
  int num = 11;

  for(int i = 0; i < num; ++i) {
    double freq = freqRisks[i];
    double db = minDb + i * (maxDb - minDb) / num;
    double x = std::max(minFreq, (std::log2(freqRisks[i]) / std::log2(maxFreq)) * WIDTH);
    int y = (1 - (db - minDb) / (maxDb - minDb)) * HEIGHT;

    cv::line(spectrumImage, cv::Point(x, 0), cv::Point(x, HEIGHT), lineColor, 1);

    std::stringstream ss;
    ss << (int)freq;
    cv::putText(spectrumImage, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

    cv::line(spectrumImage, cv::Point(0, y), cv::Point(WIDTH, y), lineColor, 1);

    std::stringstream sss;
    sss << (int)db;
    cv::putText(spectrumImage, sss.str(), cv::Point(15, y + 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);
  }
}

// void new_fft() {
//   fftw_plan fftw_plan_r2r_1d(int n, double *in, double *out,
//                            fftw_r2r_kind kind, unsigned flags);
// }

// void generateSpectrum() {
//   fftw_execute(spectrogramFrame->plan);
//   // fft();

//   // out.resize(complexIn.size());
//   // for (size_t i = 0; i < complexIn.size(); ++i) {
//   //   out[i] = 10 * std::log10(std::norm(complexIn[i]) / complexIn.size());
//   //   // std::cout << spectrum[i] << std::endl;
//   // }
//   // std::cout << 'b' << std::endl;

//   // for (auto& s : spectrum) {
//   //   std::cout << s << std::endl;
//   //   s = 10 * std::log10(s);// можно * 2 и/или s / maxAmplitude
    
//   // }
// }




int main(int argc, char** argv) {
  if(argc != 2) {
    std::cout << "Usage: ./program_name file_path" << std::endl;
    return 1;
  }

  std::string filePath = argv[1];

  av_log_set_level(AV_LOG_INFO);
  avformat_network_init();

  AVFrame* frame = av_frame_alloc();
  if(!frame) {
    std::cout << "Error allocating the frame" << std::endl;
    return 1;
  }

  AVFormatContext* formatContext = nullptr;
  if(avformat_open_input(&formatContext, filePath.c_str(), nullptr, nullptr) != 0) {
    av_frame_free(&frame);
    std::cout << "Error opening the file" << std::endl;
    return 1;
  }

  if(avformat_find_stream_info(formatContext, nullptr) < 0) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Error finding the stream info" << std::endl;
    return 1;
  }


  AVStream* audioStream = nullptr;
  for(unsigned int i = 0; i < formatContext->nb_streams; ++i) {
    if(formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
      audioStream = formatContext->streams[i];
      break;
    }
  }

  if(!audioStream) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Could not find any audio stream in the file" << std::endl;
    return 1;
  }

  const AVCodec* codec = avcodec_find_decoder(audioStream->codecpar->codec_id);
  if(!codec) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Couldn't find a proper decoder" << std::endl;
    return 1;
  }

  AVCodecContext* codecContext = avcodec_alloc_context3(codec);
  if(!codecContext) {
    av_frame_free(&frame);
    avformat_close_input(&formatContext);
    std::cout << "Could not allocate the decoder context" << std::endl;
    return 1;
  }

  if(avcodec_parameters_to_context(codecContext, audioStream->codecpar) < 0) {
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    std::cout << "Could not copy the codec parameters to the decoder context" << std::endl;
    return 1;
  }

  codecContext->frame_size = FRAME_SIZE;

  if(avcodec_open2(codecContext, codec, nullptr) < 0) {
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    std::cout << "Could not open the decoder" << std::endl;
    return 1;
  }

  

  SAMPLE_RATE = codecContext->sample_rate;
  auto durationDouble = (double)formatContext->duration / AV_TIME_BASE;
  auto frame_duration = static_cast<double>(FRAME_SIZE) / SAMPLE_RATE;
  auto total_frames = (int)(durationDouble / frame_duration);
  
  std::cout << "Total duration: " << durationDouble << std::endl;
  std::cout << "Data format: " << av_get_sample_fmt_name(codecContext->sample_fmt) << std::endl;
  std::cout << "Channels(SOURCE): " << codecContext->channels << std::endl;
  std::cout << "Sample rate(SOURCE): " << codecContext->sample_rate << std::endl;
  // std::cout << "Sample rate: " << SAMPLE_RATE << std::endl;
  std::cout << "Frame size: " << FRAME_SIZE << std::endl;
  std::cout << "Frame duration: " << frame_duration << std::endl;
  std::cout << "Total frames: " << total_frames << std::endl;



  SAMPLE_RATIO = (double)FRAME_SIZE / SAMPLE_RATE;

  spectrogramFrame = (AudioDataFrame*)malloc(sizeof(AudioDataFrame));
  spectrogramFrame->in = (double*)malloc(sizeof(double) * FRAME_SIZE);
  spectrogramFrame->out = (double*)malloc(sizeof(double) * FRAME_SIZE);
  spectrogramFrame->plan = fftw_plan_r2r_1d(
    FRAME_SIZE, spectrogramFrame->in, spectrogramFrame->out,
    FFTW_R2HC, FFTW_ESTIMATE
  );
  spectrogramFrame->start = std::ceil(SAMPLE_RATIO * FREQ_START);
  spectrogramFrame->size = min(
    std::ceil(SAMPLE_RATIO * FREQ_END),
    FRAME_SIZE / 2.0
  ) - spectrogramFrame->start;



  cv::namedWindow("Spectrum original", cv::WINDOW_NORMAL);
  cv::resizeWindow("Spectrum original", WIDTH, HEIGHT);



  // AVAudioResampleContext *avr = avresample_alloc_context();
  // av_opt_set_int(avr, "in_channel_layout", AV_CH_LAYOUT_5POINT1, 0);
  // av_opt_set_int(avr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
  // av_opt_set_int(avr, "in_sample_rate", codecContext->sample_rate, 0);
  // av_opt_set_int(avr, "out_sample_rate", SAMPLE_RATE, 0);
  // av_opt_set_int(avr, "in_sample_fmt", codecContext->sample_fmt, 0);
  // av_opt_set_int(avr, "out_sample_fmt", codecContext->sample_fmt, 0);

  drawScaleLines();

  AVPacket packet;



  auto duration = std::chrono::duration<double>(durationDouble);
  auto frame_duration_chrono = std::chrono::duration<double>(frame_duration);
  auto local = std::chrono::system_clock::now();
  auto stopTimeAll = local + duration;
  int num = 0;




  while(av_read_frame(formatContext, &packet) >= 0) {
    local = std::chrono::system_clock::now();
    auto stopTime = local + frame_duration_chrono;
    if(local >= stopTimeAll) {
      std::cout << "TIME STOP" << std::endl;
    }

    num++;
    // std::cout << "FRAME " << num << std::endl;

    if(packet.stream_index == audioStream->index) {
      int frameFinished = avcodec_receive_frame(codecContext, frame);

      avcodec_send_packet(codecContext, &packet);
      avcodec_receive_frame(codecContext, frame);

      if(frameFinished) {
        spectrogramFrame->in = (double*)frame->data[2];
        fftw_execute(spectrogramFrame->plan);
        tempImage = spectrumImage;

        

        int step = 1;
        double maxDb = 110;

        // draw_bezie(maxDb, step);
        draw_lines(maxDb, step);

        

        cv::imshow("Spectrum original", tempImage);
        
        WIDTH = cv::getWindowImageRect("Spectrum original").width;
        HEIGHT = cv::getWindowImageRect("Spectrum original").height;

        auto wait = std::max(1, (int)(1000 * std::chrono::duration<double>(stopTime - std::chrono::system_clock::now()).count()));
        cv::waitKey(wait);

        if(num == total_frames) {
          break;
        }
      }
    }

    av_packet_unref(&packet);
  }

  fftw_destroy_plan(spectrogramFrame->plan);
  fftw_free(spectrogramFrame->in);
  fftw_free(spectrogramFrame->out);
  free(spectrogramFrame);

  if(codecContext->codec->capabilities & AV_CODEC_CAP_DELAY) {
    int frameFinished = 0;
    while(avcodec_receive_frame(codecContext, frame) >= 0 && frameFinished) {
    }
  }

  av_frame_free(&frame);
  avcodec_free_context(&codecContext);
  avformat_close_input(&formatContext);

  cv::waitKey(0);
  return 0;
}
