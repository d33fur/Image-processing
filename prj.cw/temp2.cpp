#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <complex>
#include <iostream>
#include <valarray>
#include <chrono>
#include <algorithm>
#include <float.h>
#include <stdio.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#ifndef __cplusplus
    typedef uint8_t bool;
    #define true 1
    #define false 0
#endif

#ifdef __cplusplus
    #define REINTERPRET_CAST(type, variable) reinterpret_cast<type>(variable)
    #define STATIC_CAST(type, variable) static_cast<type>(variable)
#else
    #define C_CAST(type, variable) ((type)variable)
    #define REINTERPRET_CAST(type, variable) C_CAST(type, variable)
    #define STATIC_CAST(type, variable) C_CAST(type, variable)
#endif

#define RAW_OUT_ON_PLANAR false

int FRAME_SIZE = 1024;
#define NUM_CHANNELS 2
double SAMPLE_RATIO;
int SAMPLE_RATE = 41000;

double FREQ_START = 0.0;
int FREQ_END = 20000;

int WIDTH = 1600;
int HEIGHT = 1200;

using namespace std::literals;

// cv::Mat tempImage;

const double PI = 3.141592653589793238460;

typedef std::complex<double> Complex;
typedef std::vector<Complex> CArray;

std::vector<double> generateHammingCode(std::vector<double> msgBits, int m, int r) {
  std::vector<double> hammingCode(r + m);

  for (int i = 0; i < r; ++i) {
    hammingCode[pow(2, i) - 1] = -1;
  }

  int j = 0;

  for (int i = 0; i < (r + m); i++) {
    if (hammingCode[i] != -1) {
      hammingCode[i] = msgBits[j];
      j++;
    }
  }

  for (int i = 0; i < (r + m); i++) {
    if (hammingCode[i] != -1)
      continue;

    int x = log2(i + 1);
    int one_count = 0;

    for (int j = i + 2; j <= (r + m); ++j) {
      if (j & (1 << x)) {
        if (hammingCode[j - 1] == 1) {
          one_count++;
        }
      }
    }

    if (one_count % 2 == 0) {
      hammingCode[i] = 0;
    }
    else {
      hammingCode[i] = 1;
    }
  }

  return hammingCode;
}
 
void findHammingCode(std::vector<double>& msgBit) {
  int m = msgBit.size();
  int r = 1;

  while (pow(2, r) < (m + r + 1)) {
    r++;
  }

  msgBit = generateHammingCode(msgBit, m, r);
}

double interpolate(double from ,double to ,float percent) {
  double difference = to - from;
  return from + ( difference * percent );
}

void draw_bezie(cv::Mat& img, std::vector<double>& spectrum, double maxDb, int step, double pos) {
  std::vector<cv::Point> control_points = {cv::Point(0., HEIGHT)};
  // std::cout << spectrum.size() << std::endl;
  for(size_t i = 0; i < spectrum.size(); i += step) {
    pos = std::max(0., (std::log2(i * FREQ_END / spectrum.size()) / std::log2(FREQ_END)) * WIDTH);
    int y = (1 - spectrum[i] / maxDb) * HEIGHT / 2;

    control_points.push_back(cv::Point(pos, y));
  }

  for(size_t i = 2; i < control_points.size() - 1; i+=3) {
    cv::Point p0 = control_points[i - 2];
    cv::Point p1 = control_points[i - 1];
    cv::Point p2 = control_points[i];
    cv::Point p3 = control_points[i + 1];

    for(float i = 0; i < 1; i += 0.01){
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
      if(x > WIDTH)
        std::cout << x << " WIDTH" << std::endl;
      if(y > HEIGHT)
        std::cout << y << " HEIGHT" << std::endl;
      img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(255, 255, 255);
      // std::cout << 'p' << std::endl;
    }
  }
}

void draw_lines(cv::Mat& img, std::vector<double>& spectrum, double maxDb, int step, double pos, double pos1) {
  // spectrum.insert(spectrum.begin(), 0);
  for(size_t i = 0; i < spectrum.size(); i += step) {
    pos = std::max(0., (i / (double)(spectrum.size()-1)) * WIDTH);
    double y1;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << (i * SAMPLE_RATE / 2) / spectrum.size();
    if(spectrum[i] < 0) {
      y1 = (HEIGHT / 2) + ((std::abs(spectrum[i]) / maxDb)) * (HEIGHT / 2);
      cv::line(img, cv::Point(pos, (HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
      // cv::putText(img, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

      // y1 = (HEIGHT / 2) + (1 - (std::abs(spectrum[i]) / maxDb)) * (HEIGHT / 2);
      // cv::line(img, cv::Point(pos, HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
    else {
      y1 = (1 - (spectrum[i] / maxDb)) * (HEIGHT / 2);
      cv::line(img, cv::Point(pos, (HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
      // cv::putText(img, ss.str(), cv::Point(pos, y1 - 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }

    // int temp = (i * SAMPLE_RATE / 2) / spectrum.size();
    // if(temp >= FREQ_START && temp <= FREQ_END) {
    //   pos = std::max(0., (std::log2(temp) / std::log2(SAMPLE_RATE / 2)) * WIDTH);

    //   if(spectrum[i] > maxDb) std::cout << spectrum[i] << " >maxDb"  << std::endl;
    //   // if(spectrum[i] < 0) std::cout << spectrum[i] << " <0" << std::endl;
      
    //   double y1;
    //   std::stringstream ss;
    //   ss << std::fixed << std::setprecision(1) << temp;
    //   if(spectrum[i] < 0) {
    //     y1 = (HEIGHT / 2) + ((std::abs(spectrum[i]) / maxDb)) * (HEIGHT / 2);
    //     cv::line(img, cv::Point(pos, (HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    //     cv::putText(img, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

    //     // y1 = (HEIGHT / 2) + (1 - (std::abs(spectrum[i]) / maxDb)) * (HEIGHT / 2);
    //     // cv::line(img, cv::Point(pos, HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    //   }
    //   else {
    //     y1 = (1 - (spectrum[i] / maxDb)) * (HEIGHT / 2);
    //     cv::line(img, cv::Point(pos, (HEIGHT / 2)), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    //     cv::putText(img, ss.str(), cv::Point(pos, y1 - 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    //   }


    //   // pos = std::max(FREQ_START, (std::log2(i * FREQ_END / (spectrum.size()-1)) / std::log2(FREQ_END)) * WIDTH);
    //   // pos1 = std::max(0., (std::log2((i + step) * FREQ_END / (spectrum.size()-1)) / std::log2(FREQ_END)) * WIDTH);

    //   // std::stringstream ss;
    //   // ss << std::fixed << std::setprecision(1) << spectrum[i];
  
    //   // int y1 = (1 - (spectrum[i] / maxDb)) * HEIGHT;

    //   // if (i + step < spectrum.size()) {
    //   //   int y2 = (1 - (spectrum[i + step] / maxDb)) * HEIGHT;

    //   //   cv::Point vertices[4];
    //   //   vertices[0] = cv::Point(pos, y1);
    //   //   vertices[1] = cv::Point(pos1, y2);
    //   //   vertices[2] = cv::Point(pos1, HEIGHT);
    //   //   vertices[3] = cv::Point(pos, HEIGHT);

    //   //   cv::Scalar transparentColor(65, 56, 63);
    //   //   cv::fillConvexPoly(img, vertices, 4, transparentColor, cv::LINE_8);

    //   //   cv::line(img, cv::Point(pos, y1), cv::Point(pos1, y2), cv::Scalar(255, 255, 255), 1);
    //   //   cv::putText(img, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    //   // }

    //   // cv::putText(img, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1);
    //   // cv::line(img, cv::Point(pos, HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);

      
    // }
  }
}

void drawLogScaleLines(cv::Mat& img) {
  double minDb = -120;
  double maxDb = 0;
  cv::Scalar lineColor(79, 73, 80);
  cv::Scalar textColor(51, 186, 243);
  std::vector<int> freqRisks = {0, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, (int)(SAMPLE_RATE / 2)};
  int num = freqRisks.size();

  for(int i = 0; i < num; i++) {
    double freq = freqRisks[i];
    double db = minDb + i * (maxDb - minDb) / num;
    double x = std::max(0., (std::log2(freqRisks[i]) / std::log2((SAMPLE_RATE / 2))) * WIDTH);
    int y = (1 - (db - minDb) / (maxDb - minDb)) * HEIGHT;

    cv::line(img, cv::Point(x, 0), cv::Point(x, HEIGHT), lineColor, 1);

    std::stringstream ss;
    ss << (int)freq;
    cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

    cv::line(img, cv::Point(0, y), cv::Point(WIDTH, y), lineColor, 1);

    std::stringstream sss;
    sss << (int)db;
    cv::putText(img, sss.str(), cv::Point(15, y + 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);
  }
}

// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void fft(CArray &x) {
	// DFT
	unsigned int N = x.size(), k = N, n;
	double thetaT = 3.14159265358979323846264338328L / N;
	Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
	while (k > 1) {
		n = k;
		k >>= 1;
		phiT = phiT * phiT;
		T = 1.0L;
		for(unsigned int l = 0; l < k; l++) {
			for(unsigned int a = l; a < N; a += n) {
				unsigned int b = a + k;
				Complex t = x[a] - x[b];
				x[a] += x[b];
				x[b] = t * T;
			}
			T *= phiT;
		}
	}
	// Decimate
	unsigned int m = (unsigned int)log2(N);
	for(unsigned int a = 0; a < N; a++) {
		unsigned int b = a;
		// Reverse bits
		b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
		b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
		b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
		b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
		b = ((b >> 16) | (b << 16)) >> (32 - m);
		if(b > a) {
			Complex t = x[a];
			x[a] = x[b];
			x[b] = t;
		}
	}
	// // Normalize (This section make it not working correctly)
	// Complex f = 1.0 / sqrt(N);
	// for (unsigned int i = 0; i < N; i++)
	// 	x[i] *= f;
}

void dft(const std::vector<std::complex<double>>& input, std::vector<std::complex<double>>& output) {
  auto M_I_2PI_DL = -(6.28318530718i / (double)input.size());

  for(size_t k = 0; k < output.size(); ++k) {
    output[k] = 0;
    for(size_t n = 0; n < input.size(); ++n)
      output[k] += input[n] * pow(2.718281828459045, M_I_2PI_DL * (double)k * (double)n);
  }
}

void generateSpectrum(std::vector<double>& audioData, std::vector<double>& out) {
  // findHammingCode(audioData);

  // CArray input(audioData.begin(), audioData.end());
  // fft(input);

  std::vector<std::complex<double>> input(audioData.begin(), audioData.end());
  std::vector<std::complex<double>> output(input.size());
  dft(input, output);

  for(size_t i = 0; i < output.size() / 2; i++) {
    // output[i].imag(0);
    // double temp = 20 * std::log10(std::norm(output[i]));
    // if(std::norm(temp) < - 30.) {
    //   out.push_back(0.01);
    // }
    // else {
    //   out.push_back((temp + 30.) / 30.); // возможно делить не на размер а на bit depth
    // }

    out.push_back(10 * std::log10(std::norm(output[i]) / output.size())); // возможно делить не на размер а на bit depth

  }
}

float getSample(const AVCodecContext* codecCtx, uint8_t* buffer, int sampleIndex) {
    int64_t val = 0;
    float ret = 0;
    int sampleSize = av_get_bytes_per_sample(codecCtx->sample_fmt);
    switch(sampleSize) {
    case 1:
        // 8bit samples are always unsigned
        val = REINTERPRET_CAST(uint8_t*, buffer)[sampleIndex];
        // make signed
        val -= 127;
        break;

    case 2:
        val = REINTERPRET_CAST(int16_t*, buffer)[sampleIndex];
        break;

    case 4:
        val = REINTERPRET_CAST(int32_t*, buffer)[sampleIndex];
        break;

    case 8:
        val = REINTERPRET_CAST(int64_t*, buffer)[sampleIndex];
        break;

    default:
        fprintf(stderr, "Invalid sample size %d.\n", sampleSize);
        return 0;
    }

    val = (val >> (sampleSize * 8 - codecCtx->bits_per_raw_sample));

    // Check which data type is in the sample.
    switch(codecCtx->sample_fmt) {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_U8P:
        case AV_SAMPLE_FMT_S16P:
        case AV_SAMPLE_FMT_S32P:
            // integer => Scale to [-1, 1] and convert to float.
            ret = val / STATIC_CAST(float, ((1 << (sampleSize*8-1))-1));
            break;

        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            // float => reinterpret
            ret = *REINTERPRET_CAST(float*, &val);
            break;

        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_DBLP:
            // double => reinterpret and then static cast down
            ret = STATIC_CAST(float, *REINTERPRET_CAST(double*, &val));
            break;

        default:
            fprintf(stderr, "Invalid sample format %s.\n", av_get_sample_fmt_name(codecCtx->sample_fmt));
            return 0;
    }

    return ret;
    // ...
}

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

  if(avcodec_open2(codecContext, codec, nullptr) < 0) {
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    std::cout << "Could not open the decoder" << std::endl;
    return 1;
  }

  int framesPerStack = 1;
  FRAME_SIZE = codecContext->frame_size;
  SAMPLE_RATE = codecContext->sample_rate;
  // FREQ_END = SAMPLE_RATE / 2;
  auto durationDouble = (double)formatContext->duration / AV_TIME_BASE;
  auto frame_duration = (double)FRAME_SIZE / SAMPLE_RATE;
  auto total_frames = (int)(durationDouble / frame_duration);
  
  std::cout << "Total duration: " << durationDouble << std::endl;
  std::cout << "Total duration(different): " << (FRAME_SIZE * 1000) / codecContext->sample_rate  << std::endl;
  std::cout << "Data format: " << av_get_sample_fmt_name(codecContext->sample_fmt) << std::endl;
  std::cout << "Channels(SOURCE): " << codecContext->channels << std::endl;
  std::cout << "Sample rate(SOURCE): " << codecContext->sample_rate << std::endl;
  // std::cout << "Sample rate: " << SAMPLE_RATE << std::endl;
  std::cout << "Frame size(SOURCE): " << FRAME_SIZE << std::endl;
  // std::cout << "Frame size: " << FRAME_SIZE << std::endl;
  std::cout << "Frame duration: " << frame_duration << std::endl;
  std::cout << "Total frames: " << total_frames << std::endl;
  std::cout << "WIDTH: " << WIDTH << std::endl;
  std::cout << "HEIGHT: " << HEIGHT << std::endl;
  

  cv::namedWindow("Spectrum original", cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
  cv::resizeWindow("Spectrum original", WIDTH, HEIGHT);

  

  AVPacket packet;

  auto duration = std::chrono::duration<double>(durationDouble);
  auto frame_duration_chrono = std::chrono::duration<double>(frame_duration);
  auto local = std::chrono::system_clock::now();
  auto stopTimeAll = local + duration;
  int num = 0;

  std::vector<double> framesStack;

  while(av_read_frame(formatContext, &packet) >= 0) {
    // if(num != 0) {
    //   WIDTH = cv::getWindowImageRect("Spectrum original").width;
    //   HEIGHT = cv::getWindowImageRect("Spectrum original").height;
    //   // std::cout << WIDTH << ' ' << HEIGHT << std::endl;
    // }
    cv::Mat spectrumImage(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
    // std::cout << spectrumImage.cols << ' ' << spectrumImage.rows << std::endl;
    drawLogScaleLines(spectrumImage);

    num++;
    if(num % 10 == 0) std::cout << "FRAME " << num << std::endl;

    if(packet.stream_index == audioStream->index) {




      int frameFinished = avcodec_receive_frame(codecContext, frame);

      avcodec_send_packet(codecContext, &packet);
      avcodec_receive_frame(codecContext, frame);
      
      if(frameFinished) {



        // if(av_sample_fmt_is_planar(codecContext->sample_fmt) == 1) {
        //   // This means that the data of each channel is in its own buffer.
        //   // => frame->extended_data[i] contains data for the i-th channel.
        //   for(int s = 0; s < frame->nb_samples; ++s) {
        //     for(int c = 0; c < codecContext->channels; ++c) {
        //       framesStack.push_back(getSample(codecContext, frame->extended_data[c], s));
        //       // fwrite(&sample, sizeof(float), 1, outFile);
        //     }
        //   }
        // } else {
        //   // This means that the data of each channel is in the same buffer.
        //   // => frame->extended_data[0] contains data of all channels.
        //   if(RAW_OUT_ON_PLANAR) {
        //     std::cout << "RAW_OUT_ON_PLANAR" << std::endl;
        //     std::cout << frame->linesize[0] << std::endl;
        //     // fwrite(frame->extended_data[0], 1, frame->linesize[0], "out.txt");
        //   } else {
        //     for(int s = 0; s < frame->nb_samples; ++s) {
        //       for(int c = 0; c < codecContext->channels; ++c) {
        //         framesStack.push_back(getSample(codecContext, frame->extended_data[0], s*codecContext->channels+c));
        //         // fwrite(&sample, sizeof(float), 1, outFile);
        //       }
        //     }
        //   }
        // }




        frame_duration = (double)frame->nb_samples / SAMPLE_RATE;

        // std::vector<uint8_t> audioData((uint8_t*)frame->data, (uint8_t*)(frame->data + num_samples));
        // uint8_t* audioData = frame->extended_data[0]; //data[0] левое, data[1] правое

        
        for(int i = 0; i < frame->nb_samples; i++) {
          // std::cout << (float)audioData[i] << std::endl;
          // framesStack.push_back((audioData[i]));
          framesStack.push_back(getSample(codecContext, frame->extended_data[0], i*codecContext->channels));
        }
        
        std::cout << framesStack.size() << ' ' << codecContext->channels << std::endl;
        if(num % framesPerStack == 0) {
          // cv::waitKey(0);

          local = std::chrono::system_clock::now();
          auto stopTime = local + framesPerStack * frame_duration_chrono;
          if(local >= stopTimeAll) {
            std::cout << "TIME STOP" << std::endl;
          }
          
          std::vector<double> spectrum;
          generateSpectrum(framesStack, spectrum);

          // tempImage = spectrumImage.clone();
          int step = 1;
          double maxDb = 160.0;

          // draw_bezie(spectrumImage, spectrum, maxDb, step, 0.0);
          draw_lines(spectrumImage, spectrum, maxDb, step, 0.0, 0.0);
          
          // cv::Mat resizedImage;
          // cv::resize(tempImage, resizedImage, cv::Size(WIDTH, HEIGHT), 0, 0, cv::INTER_LINEAR);
          cv::imshow("Spectrum original", spectrumImage);
          // cv::imshow("Spectrum original", spectrumImage);



          auto wait = std::max(1, (int)(1000 * std::chrono::duration<double>(stopTime - std::chrono::system_clock::now()).count()));
          cv::waitKey(wait);

          framesStack = {};

          if(num == total_frames) {
            break;
          }
        }        
      }
    }

    av_packet_unref(&packet);
  }

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