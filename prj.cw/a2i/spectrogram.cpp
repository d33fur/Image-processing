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

void a2i::Spectrogram::setFreqRange(std::pair<unsigned int, unsigned int> audio_freq_range)
{
  freq_range = audio_freq_range;
}

void a2i::Spectrogram::setFrameSize(int size) 
{
  frame_size = size;
  in.resize(frame_size);
  in_in.resize(frame_size);
  out.resize(frame_size / 2);
  ft_out.resize(frame_size);
  window_out.resize(frame_size);
}

void a2i::Spectrogram::setWindowFunc(int type) 
{
  (this->*windows[type])();
}

void a2i::Spectrogram::windowSine()
{
  window_correction = 2.0;
  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = sin(M_PI * i / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowHann() 
{
  window_correction = 2.0;
  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = 0.5 * (1.0 - cos((2.0 * M_PI * i) / (frame_size)));
  }
}

void a2i::Spectrogram::windowHamming()
{
  window_correction = 1.85;
  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = (25. / 46) * (1.0 - cos((2.0 * M_PI * i) / (frame_size * 2)));
  }
}

void a2i::Spectrogram::windowBlackman()
{
  window_correction = 2.38;
  float alfa = 0.16;
  float a0 = (1. - alfa) / 2;
  float a1 = 1 / 2;
  float a2 = alfa / 2;

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowNuttall()
{
  window_correction = 2.812;
  float a0 = 0.355768;
  float a1 = 0.487396;
  float a2 = 0.144232;
  float a3 = 0.012604;

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowBlackmanNuttall()
{
  window_correction = 2.776;
  float a0 = 0.3635819;
  float a1 = 0.4891775;
  float a2 = 0.1365995;
  float a3 = 0.0106411;

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowBlackmanHarris()
{
  window_correction = 2.79;
  float a0 = 0.35875;
  float a1 = 0.48829;
  float a2 = 0.14128;
  float a3 = 0.01168;

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowFlatTop()
{
  window_correction = 4.63;
  float a0 = 0.21557895;
  float a1 = 0.41663158;
  float a2 = 0.277263158;
  float a3 = 0.083578947;
  float a4 = 0.006947368;

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = a0 - a1 * cos((2.0 * M_PI * i) / (frame_size * 2)) + a2 * cos((4.0 * M_PI * i) / (frame_size * 2)) - a3 * cos((6.0 * M_PI * i) / (frame_size * 2)) + a4 * cos((8.0 * M_PI * i) / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowBartlettHann()
{
  window_correction = 1.55;
  float a0 = 0.62;
  float a1 = 0.48;
  float a2 = 0.38;

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = a0 - a1 * abs(static_cast<double>(i) / (frame_size * 2) - 1 / 2) - a2 * cos((2.0 * M_PI * i) / (frame_size * 2));
  }
}

void a2i::Spectrogram::windowHannPoisson()
{
  window_correction = 2.0;
  float a = 2; // >= 2

  for(size_t i = 0; i < static_cast<size_t>(frame_size); ++i)
  {
    window_out[i] = (1. / 2) *(1. - cos((2.0 * M_PI * i) / (frame_size * 2))) * pow(M_E, -a * abs(static_cast<double>(frame_size * 2 - 2 * i) / (frame_size * 2)));
  }
}

void a2i::Spectrogram::addWindow() 
{
  for(size_t i = 0; i < frame_size; ++i) 
  {
    in[i] *= window_out[i];
  }
}

void a2i::Spectrogram::fftw1()
{
  double in1[frame_size];
  fftw_complex out1[frame_size];
  fftw_plan p;

  for(size_t i = 0; i < frame_size; i++)
  {
    in1[i] = in[i];
  }

  p = fftw_plan_dft_r2c_1d(frame_size, in1, out1, FFTW_ESTIMATE);

  fftw_execute(p);

  for(size_t i = 0; i < frame_size / 2; i++)
  {
    ft_out[i] = std::complex<float>(out1[i][0], out1[i][1]);
  }

  fftw_destroy_plan(p);
  fftw_cleanup();



}

void a2i::Spectrogram::normalize(const int multiplier)
{
  for(size_t i = 0; i < frame_size / 2; ++i)
  {
    float db_value = multiplier * std::log10(std::norm(ft_out[i]) / frame_size + 1e-10);

    if (db_value < min_max_db.first) db_value = min_max_db.first + 1;
    if (db_value > min_max_db.second) db_value = min_max_db.first - 1;

    out[i] = db_value;
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
    freq_risks = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000};
  }
  else
  {
    for(const auto& freq : freqs)
    {      
      freq_risks.push_back(freq);
    }
  }

  int freq_size = freq_risks.size();

  float x = 0.0f;

  cv::line(img, cv::Point(x, 0), cv::Point(x, img.rows), line_color, 1);

  if(enable_text)
  {
    std::stringstream ss;
    ss << static_cast<int>(freq_risks[0]);
    cv::putText(img, ss.str(), cv::Point(x, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, text_color, 1);
  }

  for(int i = 1; i < freq_size; i++)
  {
    x = type ? 
      std::max(0., ((std::log2(freq_risks[i]) - std::log2(freq_risks[0])) / (std::log2(freq_range.second) - std::log2(freq_risks[0]))) * img.cols)
    : std::max(0., static_cast<double>(freq_risks[i] - freq_risks[0])) * img.cols * 2 / sample_rate;

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
  // std::vector<cv::Point> control_points = {cv::Point(0.0, img.rows)};

  int y_first;
  double x_first = 0.0;

  bool first_point = true;

  std::vector<cv::Point> control_points;
  for(size_t i = 0; i < frame_size / 2; i += 1) {
    double pos = std::min(static_cast<double>(img.cols), std::max(0., (double)((std::log2(i * (sample_rate / 2) / (frame_size / 2)) - std::log2(freq_range.first)) / (std::log2(freq_range.second) - std::log2(freq_range.first))) * img.cols));
    int y;

    double freq = i * (sample_rate / 2) / (frame_size / 2);

    if(out[i] < 0) {
      y = (1 - (std::abs(min_max_db.first - out[i]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
    }
    else {
      y = (1 - ((std::abs(min_max_db.first) + out[i]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
    }

    if(freq >= freq_range.first && freq <= freq_range.second)
    {
      if(first_point)
      {
        control_points.push_back(cv::Point(x_first, y_first));
        first_point = false;
      }
      control_points.push_back(cv::Point(pos, y));
      
    }
    else
    { 
      y_first = y;

      if(!first_point)
      {
        control_points.push_back(cv::Point(img.cols, y));
      }
    }
  }

  for(size_t i = 2; i < control_points.size() - 1; i+=3) {
    cv::Point p0 = control_points[i - 2];
    cv::Point p1 = control_points[i - 1];
    cv::Point p2 = control_points[i];
    cv::Point p3 = control_points[i + 1];

    double sstep = 0.6 / std::max(static_cast<double>(abs(p0.x - p3.x)), 0.0001);

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
      if(x >= img.cols) x = img.cols - 1;
      if(y >= img.rows) y = img.rows - 1;
      if(x <= 0) x = 1;
      if(y <= 0) y = 1;
      // std::cout << x << " " << y << std::endl;
      img.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(255, 255, 255);

      // cv::line(img, cv::Point(x, y), cv::Point(x, img.rows), cv::Scalar(79, 73, 80), 1); // одним цветом заливка
    
      int gradient_color = static_cast<int>((1 - y / static_cast<double>(img.rows)) * 127); // 255
      cv::line(img, cv::Point(x, y), cv::Point(x, img.rows), cv::Scalar(gradient_color, gradient_color, gradient_color), 1);
  

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
      y1 = (1 - (std::abs(min_max_db.first - out[i]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
      cv::line(img, cv::Point(pos, img.rows), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
    else {
      y1 = (1 - ((std::abs(min_max_db.first) + out[i]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
      cv::line(img, cv::Point(pos, img.rows), cv::Point(pos, y1), cv::Scalar(127, 127, 127), 1);
    }
  }
}

void a2i::Spectrogram::draw_log_lines_2d(cv::Mat& img) {
  out.insert(out.begin(), min_max_db.first);
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
        y1 = (1 - (std::abs(min_max_db.first - out[i]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
      }
      else {
        y1 = (1 - ((std::abs(min_max_db.first) + out[i]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
      }
      
      if (i + 1 < frame_size/2) {
        int y2;

        if(out[i + 1] < 0) {
          y2 = (1 - (std::abs(min_max_db.first - out[i + 1]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
        }
        else {
          y2 = (1 - ((std::abs(min_max_db.first) + out[i + 1]) / std::abs(min_max_db.second - min_max_db.first))) * img.rows;
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