#include "spectrogram.hpp"


void a2i::Spectrogram::setFrameSize(int size) {
  frame_size = size;
  in.resize(frame_size); // frame size
  out.resize(frame_size / 2); // frame size / 2
  ft_out.resize(frame_size); // frame size
  window_out.resize(frame_size * 2); // frame size * 2
}

void a2i::Spectrogram::setWindowFunc(int type) {
  (this->*windows[type])();
}

void a2i::Spectrogram::windowHann() {
  float ND = (float)(frame_size / 2);

  for(size_t i = 0; i < (size_t)(frame_size / 2); ++i) {
    float nD = static_cast<float>(i);
    float real = 0.5 * (1.0 - cos((2.0 * M_PI * nD) / ND));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::useWindowFunc() {
  for(size_t i = 0; i < frame_size; ++i) {
    in[i] *= window_out[i];
  }
}

// void a2i::Spectrogram::setFurierTransformFunc(int type) {
//   (*FTs[type])();
// }

// void a2i::Spectrogram::fft(const std::vector<std::complex<float>>& in, size_t stride, std::vector<std::complex<float>>& out, size_t n) {
//     assert(n > 0);

//     if (n == 1) {
//         out[0] = in[0];
//         return;
//     }

//     std::vector<std::complex<float>> tempOut(n);
//     fft(in, stride * 2, tempOut, n / 2);
//     fft(std::vector<std::complex<float>>(in.begin() + stride, in.end()), stride * 2, tempOut, n / 2);

//     for (size_t i = 0; i < n / 2; i++) {
//         float t = (float)i / n;
//         std::complex<float> v = std::complex<float>(cexp(-2 * M_PI * t)) * tempOut[i + n / 2];
//         std::complex<float> e = tempOut[i];
//         out[i] = e + v;
//         out[i + n / 2] = e - v;
//     }
// }

// void a2i::Spectrogram::fft() {
//   std::complex<float> inn[frame_size], outt[frame_size];
//   for(size_t i = 0; i < frame_size; i++) {
//     inn[i] = window_out[i];
//   }
//   fft_c(inn, 1, outt, frame_size);
//   for(size_t i = 0; i < frame_size / 2; i++) {
//     ft_out[i] = outt[i];
//   }
// }

// void a2i::Spectrogram::fft_c(std::complex<float> in[], size_t stride, std::complex<float> out[], size_t n) {
//   assert(n > 0);

//   if(n == 1) {
//     out[0] = in[0];
//     return;
//   }

//   fft(in, stride * 2, out, n / 2);
//   fft(in + stride, stride* 2, out + n / 2, n / 2);

//   for(size_t i = 0; i < n / 2; i++) {
//     float t = (float)i / n;
//     std::complex<float> v = (std::complex<float>)cexp(-2 * I * M_PI * t) * out[i + n/2];
//     std::complex<float> e = out[i];
//     out[i] = e + v;
//     out[i + n / 2] = e - v;
//   }
// }

void a2i::Spectrogram::normalize(const int multiplier) {
  for(size_t i = 0; i < frame_size / 2; ++i) {
    out[i] = multiplier * std::log10(std::norm(ft_out[i]) / sample_size);
  }
}

void a2i::Spectrogram::drawGrid(cv::Mat& img) {
  float minDb = -120;
  float maxDb = 0;
  cv::Scalar lineColor(79, 73, 80);
  cv::Scalar textColor(51, 186, 243);
  std::vector<int> freqRisks = {0, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, (int)(sample_rate / 2)};
  int num = freqRisks.size();

  for(int i = 0; i < num; i++) {
    float freq = freqRisks[i];
    float db = minDb + i * (maxDb - minDb) / num;
    float x = std::max(0.0, (std::log2(freqRisks[i]) / std::log2((sample_rate / 2))) * img.cols);
    int y = (1 - (db - minDb) / (maxDb - minDb)) * img.rows;

    cv::line(img, cv::Point(x, 0), cv::Point(x, img.rows), lineColor, 1);

    std::stringstream ss;
    ss << (int)freq;
    cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);

    cv::line(img, cv::Point(0, y), cv::Point(img.cols, y), lineColor, 1);

    std::stringstream sss;
    sss << (int)db;
    cv::putText(img, sss.str(), cv::Point(15, y + 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1);
  }
}

void a2i::Spectrogram::drawSpectrum(cv::Mat& img) {
  float pos = 0;
  // float pos1 = 0;
  int step = 1;
  float maxDb = 190;
  // std::vector<cv::Point> control_points = {cv::Point(0.0, WINDOW_HEIGHT)};
  std::vector<cv::Point> control_points;
  // std::cout << spectrum.size() << std::endl;
  for(size_t i = 0; i < sample_size / 2; i += step) {
    pos = std::max(0., (double)(std::log2(i * (sample_rate / 2) / (sample_size / 2)) / std::log2((sample_rate / 2))) * img.cols);
    int y = (1 - out[i] / maxDb) * img.rows / 2;
    // std::cout << out[i] << std::endl;
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
      if(x > img.cols) x = img.cols;
      if(y > img.rows) y = img.rows;
      if(x < 0) x = 0;
      if(y < 0) y = 0;
      // std::cout << x << " " << y << std::endl;
      img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(255, 255, 255);

      // cv::line(img, cv::Point(x, y), cv::Point(x, WINDOW_HEIGHT), cv::Scalar(79, 73, 80), 1);
    }
    // sstep+=0.001;
  }
}

double a2i::Spectrogram::interpolate(double from ,double to ,float percent) {
  double difference = to - from;
  return from + ( difference * percent );
}

// void a2i::Spectrogram::callback(void *bufferData, unsigned int frames) {
//   if(frames < frame_size) return;

//   a2i::Frame *fs = static_cast<a2i::Frame*>(bufferData);

//   for(size_t i = 0; i < frames; ++i) {
//     in[i] = (fs[i].left + fs[i].right) * hannOut[i];
//   }

  
//   // dft(in, out, frame_size);
//   fft(in, 1, out, sample_rate);


//   for(size_t i = 0; i < frame_size / 2; ++i) {
//     powerOut[i] = 20 * std::log10(std::norm(out[i]) / sample_rate); // можно делить на sample size
//   }
// }