#include "spectrogram.hpp"


void a2i::Spectrogram::setAudioInfo(
  unsigned int audio_sample_rate, 
  unsigned int audio_sample_size, 
  unsigned int audio_channels,
  std::pair<int, int> audio_min_max_db) 
{
  sample_rate = audio_sample_rate;
  sample_size = audio_sample_size;
  channels = audio_channels;
  min_max_db = audio_min_max_db;
}

void a2i::Spectrogram::setFrameSize(int size) 
{
  frame_size = size;
  in.resize(frame_size);
  out.resize(frame_size / 2);
  ft_out.resize(frame_size);
  window_out.resize(frame_size * 2);
}

void a2i::Spectrogram::setWindowFunc(int type) 
{
  (this->*windows[type])();
}

void a2i::Spectrogram::windowSine()
{
  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = sin(M_PI * i / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowHann() 
{
  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = 0.5 * (1.0 - cos((2.0 * M_PI * i) / (frame_size * 2)));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowHamming()
{
  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = (25. / 46) * (1.0 - cos((2.0 * M_PI * i) / (frame_size * 2)));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowBlackman()
{
  float alfa = 0.16;
  float a0 = (1. - alfa) / 2;
  float a1 = 1 / 2;
  float a2 = alfa / 2;

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowNuttall()
{
  float a0 = 0.355768;
  float a1 = 0.487396;
  float a2 = 0.144232;
  float a3 = 0.012604;

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowBlackmanNuttall()
{
  float a0 = 0.3635819;
  float a1 = 0.4891775;
  float a2 = 0.1365995;
  float a3 = 0.0106411;

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowBlackmanHarris()
{
  float a0 = 0.35875;
  float a1 = 0.48829;
  float a2 = 0.14128;
  float a3 = 0.01168;

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowFlatTop()
{
  float a0 = 0.21557895;
  float a1 = 0.41663158;
  float a2 = 0.277263158;
  float a3 = 0.083578947;
  float a4 = 0.006947368;

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2)) + a4 * cos((8.0 * M_PI * i) / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowBartlettHann()
{
  float a0 = 0.62;
  float a1 = 0.48;
  float a2 = 0.38;

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = a0 - a1 * abs(static_cast<double>(i) / (frame_size * 2) - 1 / 2) - a2 * cos((2.0 * M_PI * i) / (frame_size * 2));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::windowHannPoisson()
{
  float a = 2; // >= 2

  for(size_t i = 0; i < static_cast<size_t>(frame_size * 2); ++i)
  {
    float real = (1. / 2) *(1. - cos((2.0 * M_PI * i) / (frame_size * 2))) * pow(M_E, -a * abs(static_cast<double>(frame_size * 2 - 2 * i) / (frame_size * 2)));
    window_out[i] = std::complex<float>(real, 0.0);
  }
}

void a2i::Spectrogram::addWindow() 
{
  for(size_t i = 0; i < frame_size; ++i) 
  {
    in[i] *= window_out[i];
  }
}

void a2i::Spectrogram::fft() 
{
  std::complex<float> inn[frame_size], outt[frame_size];
  for(size_t i = 0; i < frame_size; i++)
  {
    inn[i] = in[i];
  }
  fft_c(inn, 1, outt, frame_size);
  for(size_t i = 0; i < frame_size / 2; i++)
  {
    ft_out[i] = outt[i];
  }
}

void a2i::Spectrogram::fft_c(std::complex<float> in1[], size_t stride1, std::complex<float> out1[], size_t n1)
{
  assert(n1 > 0);

  if(n1 == 1)
  {
    out1[0] = in1[0];
    return;
  }

  fft_c(in1, stride1 * 2, out1, n1 / 2);
  fft_c(in1 + stride1, stride1* 2, out1 + n1 / 2, n1 / 2);

  for(size_t i = 0; i < n1 / 2; i++)
  {
    float t = (float)i / n1;
    std::complex<float> v = (std::complex<float>)cexp(-2 * I * M_PI * t) * out1[i + n1/2];
    std::complex<float> e = out1[i];
    out1[i] = e + v;
    out1[i + n1 / 2] = e - v;
  }
}

void a2i::Spectrogram::normalize(const int multiplier)
{
  for(size_t i = 0; i < frame_size / 2; ++i)
  {
    out[i] = multiplier * std::log10(std::norm(ft_out[i]) / sample_size);
  }
}

void a2i::Spectrogram::drawGrid(
  cv::Mat& img, 
  const int type, 
  const bool enable_text, 
  const std::vector<unsigned int>& freqs, 
  const int number_of_db_risks,
  const cv::Scalar line_color, 
  const cv::Scalar text_color) 
{

  // добавить проверку валидности переменных

  std::vector<unsigned int> freq_risks;

  if(freqs.size() == 0)
  {
    freq_risks = {0, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, static_cast<unsigned int>(sample_rate / 2)};
  }
  else
  {
    unsigned int last_freq = static_cast<unsigned int>(sample_rate / 2);

    for(const auto& freq : freqs)
    {      
      if(freq < last_freq)
      {
        freq_risks.push_back(freq);
      }
    }

    freq_risks.push_back(static_cast<unsigned int>(sample_rate / 2));
  }

  int freq_size = freq_risks.size();

  for(int i = 0; i < freq_size; i++)
  {
    float x = type ? 
      std::max(0., (std::log2(freq_risks[i]) / std::log2((sample_rate / 2))) * img.cols)
    : std::max(0., static_cast<double>(freq_risks[i]) * img.cols * 2 / sample_rate);

    cv::line(img, cv::Point(x, 0), cv::Point(x, img.rows), line_color, 1);

    if(enable_text)
    {
      std::stringstream ss;
      ss << static_cast<int>(freq_risks[i]);
      cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
    }
  }

  for(int i = 0; i < number_of_db_risks; i++)
  {
    float db = min_max_db.first + i * (min_max_db.second - min_max_db.first) / number_of_db_risks;
    float y = (1 - (db - min_max_db.first) / (min_max_db.second - min_max_db.first)) * img.rows;

    cv::line(img, cv::Point(0, y), cv::Point(img.cols, y), line_color, 1);

    if(enable_text)
    {
      std::stringstream ss;
      ss << static_cast<int>(db);
      cv::putText(img, ss.str(), cv::Point(15, y + 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
    }
  }
}

void a2i::Spectrogram::drawSpectrum(
  cv::Mat& img, 
  const int line_type, 
  const int graph_mode, 
  const int fill_type, 
  const cv::Scalar line_color, 
  const cv::Scalar underline_color) {
  

}

double a2i::Spectrogram::interpolate(double from ,double to ,float percent) {
  double difference = to - from;
  return from + ( difference * percent );
}

void a2i::Spectrogram::draw_log_bezie_2d(cv::Mat& img) {
  // std::vector<cv::Point> control_points = {cv::Point(0.0, WINDOW_HEIGHT)};
  std::vector<cv::Point> control_points;
  // std::cout << spectrum.size() << std::endl;
  for(size_t i = 0; i < frame_size / 2; i += 1) {
    double pos = std::max(0., (double)(std::log2(i * (sample_rate / 2) / (frame_size / 2)) / std::log2((sample_rate / 2))) * img.cols);
    int y;
    if(out[i] < 0) {
      y = (img.rows / 2) + ((std::abs(out[i]) / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
    }
    else {
      y = (1 - (out[i] / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
    }

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
      sstep = 0.01;
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

void a2i::Spectrogram::draw_lines_simple_2d(cv::Mat& img) {
  for(size_t i = 0; i < frame_size / 2; i += 1) {
    int pos = std::max(0., (i / (double)(frame_size/2)) * img.cols);
    double y1;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << (i * sample_rate / 2) / (frame_size / 2);
    if(out[i] < 0) {
      y1 = (img.rows / 2) + ((std::abs(out[i]) / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
      cv::line(img, cv::Point(pos, img.rows), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
    else {
      y1 = (1 - (out[i] / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
      cv::line(img, cv::Point(pos, img.rows), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
  }
}

void a2i::Spectrogram::draw_log_lines_2d(cv::Mat& img) {
  // spectrum.insert(spectrum.begin(), 0);
  for(size_t i = 0; i < frame_size / 2; i += 1) {
    // int temp = (i * SAMPLE_RATE / 2) / (N/2);
    // if(temp >= FREQ_START && temp <= FREQ_END) {
    //   if(powerOut[i] > maxDb) std::cout << powerOut[i] << " >maxDb"  << std::endl;

      int pos = std::max(0., (double)(std::log2(i * (sample_rate / 2) / (frame_size/2)) / std::log2(sample_rate / 2)) * img.cols);
      int pos1 = std::max(0., (double)(std::log2((i + 1) * (sample_rate / 2) / (frame_size/2)) / std::log2(sample_rate / 2)) * img.cols);

      std::stringstream ss;
      ss << std::fixed << std::setprecision(1) << out[i];
  
      int y1;

      if(out[i] < 0) {
        y1 = (img.rows / 2) + ((std::abs(out[i]) / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
      }
      else {
        y1 = (1 - (out[i] / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
      }
      
      if (i + 1 < frame_size/2) {
        int y2;

        if(out[i + 1] < 0) {
          y2 = (img.rows / 2) + ((std::abs(out[i + 1]) / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
        }
        else {
          y2 = (1 - (out[i + 1] / (min_max_db.second - min_max_db.first))) * (img.rows / 2);
        }

        cv::Point vertices[4];
        vertices[0] = cv::Point(pos, y1);
        vertices[1] = cv::Point(pos1, y2);
        vertices[2] = cv::Point(pos1, img.rows);
        vertices[3] = cv::Point(pos, img.rows);

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