#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <cmath>
#include <complex>
#include <iostream>
#include <valarray>
#include <chrono>
#include <algorithm>

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


const double PI = 3.141592653589793238460;

typedef std::complex<double> Complex;
typedef std::vector<Complex> CArray;

cv::Mat spectrumImage(HEIGHT, WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
cv::Mat tempImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);

typedef struct {
  std::vector<double> in;
  std::vector<double> out;
  // int start = std::ceil(SAMPLE_RATIO * FREQ_START);
  // int size = min(
  //   std::ceil(SAMPLE_RATIO * FREQ_END),
  //   FRAME_SIZE / 2.0
  //   ) - start;
  int start;
  int size;
} AudioDataFrame;

static AudioDataFrame spectrogramFrame;


double interpolate(double from ,double to ,float percent) {
  double difference = to - from;
  return from + ( difference * percent );
}

void draw_bezie(double maxDb, int step, double pos) {
  std::vector<cv::Point> control_points = {cv::Point(0., HEIGHT)};

  for(int i = 0; i < FRAME_SIZE; i += step) {
    pos = std::max(0., (std::log2(i * 20000 / FRAME_SIZE) / std::log2(20000)) * WIDTH);
    int y = (1 - spectrogramFrame.out[i] / maxDb) * HEIGHT;

    control_points.push_back(cv::Point(pos, y));
  }

  for(int i = 2; i < FRAME_SIZE - 1; i+=3) {
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
      tempImage.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(255, 255, 255);
      // std::cout << 'p' << std::endl;
    }
  }
}

void draw_lines(double maxDb, int step, double pos, double pos1) {
  spectrogramFrame.out.insert(spectrogramFrame.out.begin(), 0);
  for(int i = 0; i < FRAME_SIZE + 1; i += step) {
    pos = std::max(0., (std::log2(i * 20000 / (FRAME_SIZE - 1)) / std::log2(20000)) * WIDTH);
    pos1 = std::max(0., (std::log2((i + step) * 20000 / (FRAME_SIZE - 1)) / std::log2(20000)) * WIDTH);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << spectrogramFrame.out[i];
    int y1 = (1 - std::abs(spectrogramFrame.out[i]) / maxDb) * HEIGHT;

    if (i + step < FRAME_SIZE) {
      int y2 = (1 - std::abs(spectrogramFrame.out[i + step]) / maxDb) * HEIGHT;

      cv::Point vertices[4];
      vertices[0] = cv::Point(pos, y1);
      vertices[1] = cv::Point(pos1, y2);
      vertices[2] = cv::Point(pos1, HEIGHT);
      vertices[3] = cv::Point(pos, HEIGHT);

      cv::Scalar transparentColor(65, 56, 63);
      cv::fillConvexPoly(spectrumImage, vertices, 4, transparentColor, cv::LINE_8);

      cv::line(spectrumImage, cv::Point(pos, y1), cv::Point(pos1, y2), cv::Scalar(255, 255, 255), 1);
      cv::putText(spectrumImage, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    }
    // cv::putText(spectrumImage, ss.str(), cv::Point(pos, y1 + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(127, 127, 127), 1);
    // cv::line(spectrumImage, cv::Point(pos, HEIGHT), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
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
		for (unsigned int l = 0; l < k; l++) {
			for (unsigned int a = l; a < N; a += n) {
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

void generateSpectrum() {
  CArray input(spectrogramFrame.in.begin(), spectrogramFrame.in.end());
  fft(input);

  for (size_t i = 0; i < input.size(); ++i) {
    spectrogramFrame.out[i] = 10 * std::log10(std::norm(input[i]) / input.size());
    // std::cout << spectrum[i] << std::endl;
  }
  // std::cout << 'b' << std::endl;

  // for (auto& s : spectrum) {
  //   std::cout << s << std::endl;
  //   s = 10 * std::log10(s);// можно * 2 и/или s / maxAmplitude
    
  // }

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

  cv::namedWindow("Spectrum original", cv::WINDOW_NORMAL);
  cv::resizeWindow("Spectrum original", WIDTH, HEIGHT);

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
        spectrogramFrame.in.assign((double*)frame->data[0], (double*)frame->data[0] + frame->nb_samples);
        // spectrogramFrame.in = (double*)frame->data[1];
        generateSpectrum();
        tempImage = spectrumImage;

        

        int step = 1;
        double maxDb = 110;
        draw_bezie(maxDb, step, 0);
        // draw_lines(maxDb, step, 0, 0);

        

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