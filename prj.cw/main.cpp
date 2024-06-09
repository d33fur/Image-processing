#include "raylib.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "a2i/spectrogram.hpp"

int WINDOW_WIDTH = 1600;
int WINDOW_HEIGHT = 1200;
unsigned int FRAME_SIZE = 512;
a2i::Spectrogram g;

int counter = 0;
int multiplier = 20;
bool first = true;
bool stop = true;

typedef struct 
{
  float left;
  float right;
} Frame;

void callback(void *bufferData, unsigned int frames) 
{
  if(frames < 512) return;

  Frame *fs = static_cast<Frame*>(bufferData);

  if(counter < static_cast<int>(FRAME_SIZE / 512) - 1 && FRAME_SIZE != 512)
  {
    for(size_t i = 0 ; i < frames / 2; ++i) 
    {
      g.in[i + counter * frames] = (fs[i].left + fs[i].right) / 2;
    }
    if(counter == static_cast<int>(FRAME_SIZE / 512) - 2)
    {
      first = false;
    }
    counter++;
  }
  else
  {
    counter = 0;

    for(size_t i = 0; i < frames / 2; ++i) 
    {
      g.in[i + counter * frames] = (fs[i].left + fs[i].right) / 2;
    }

    // домножить на оконную функцию
    g.addWindow();

    //вызвать fft
    // g.fft();
    g.fftw1();

    //нормализовать
    g.normalize(multiplier);

    FRAME_SIZE == 512 ? first = false : first = true;    
  }

}

std::pair<int, int> parseRange(const std::string& str) 
{
  std::istringstream iss(str);
  int first, second;
  char comma;

  if(!(iss >> first >> comma >> second) || (comma != ',')) 
  {
    throw std::invalid_argument("Invalid string format");
  }

  return std::make_pair(first, second);
}

// void blendImages(const cv::Mat& src, cv::Mat& dst, const cv::Mat& mask) 
// {
//   src.copyTo(dst, mask);
// }

int main(int argc, char** argv) 
{
  cv::CommandLineParser parser(argc, argv,
  "{ p    |               | path to audiofile         }"
  "{ a    |    -90,6      | amplitude range           }"
  "{ w    |       0       | window function           }"
  "{ m    |       20      | mormalize multiplier      }"
  "{ mic  |               | use microphone            }"
  "{ f    |      512      | frame size                }"
  "{ n    |       3       | number of prev frames     }"
  "{ size |   1200,1600   | window size(height,width) }");

  multiplier = parser.get<int>("m");
  auto amp = parseRange(parser.get<std::string>("a"));
  auto window_function = parser.get<int>("w");
  auto use_mic = parser.has("mic");
  FRAME_SIZE = parser.get<unsigned int>("f");
  auto num_frames = parser.get<int>("n");
  auto window_size = parseRange(parser.get<std::string>("size"));
  WINDOW_HEIGHT = window_size.first;
  WINDOW_WIDTH = window_size.second;
  auto file = parser.get<std::string>("p");
  const char *file_path = file.c_str();

  InitAudioDevice();
  AudioStream stream;
  Music music;

  if(use_mic) 
  {
    stream = LoadAudioStream(44100, 16, 1);
    PlayAudioStream(stream);
  } 
  else 
  {
    stream = LoadAudioStream(44100, 16, 2);
    music = LoadMusicStream(file_path);
    PlayMusicStream(music);
    SetMusicVolume(music, 0.1f);
    AttachAudioStreamProcessor(music.stream, callback);
  }

  g.setAudioInfo(music.stream.sampleRate, music.stream.sampleSize, 2, amp);
  g.setFreqRange({20, 20000});
  g.setFrameSize(FRAME_SIZE);
  g.setWindowFunc(window_function);

  cv::namedWindow("Spectrum original", cv::WINDOW_NORMAL);
  cv::resizeWindow("Spectrum original", WINDOW_WIDTH, WINDOW_HEIGHT);

  cv::Mat img(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
  std::vector<cv::Mat> prev_frames(num_frames, cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3));
  cv::Mat cur_img = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
  cv::Mat grid = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
  cv::Mat mask = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC1);
  g.drawGrid(grid, a2i::LOG, 1, {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000}, 10);

  while(true) 
  {
    if(use_mic) 
    {
      unsigned int frames = FRAME_SIZE;
      Frame *buffer = (Frame *)malloc(frames * sizeof(Frame));
      if(IsAudioStreamProcessed(stream))
      {
        UpdateAudioStream(stream, buffer, frames);
        callback(buffer, frames);
      }
      free(buffer);
    } 
    else
    {
      UpdateMusicStream(music);

      int key = cv::waitKey(1);

      if(key == ' ') 
      {
        if(stop) PauseMusicStream(music);
        else ResumeMusicStream(music);
        stop = !stop;
      }

      if(key == 27)  // Esc 
      {
        break;
      }
      if(key == 83)  // Right arrow 
      {
        float current_time = GetMusicTimePlayed(music);
        SeekMusicStream(music, current_time + 5.0f);
      }
      if(key == 81)  // Left arrow 
      {
        float current_time = GetMusicTimePlayed(music);
        SeekMusicStream(music, current_time - 5.0f);
      }
    }

    if(!first)
    {
      img = cv::Mat(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
      cv::add(grid, img, img);

      // g.drawSpectrum(img);

      // g.draw_lines_simple_2d(img);

      cur_img = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
      g.draw_log_bezie_2d(cur_img);

      // g.draw_log_lines_2d(cur_img);

      // cv::cvtColor(cur_img, mask, cv::COLOR_BGR2GRAY);
      // cv::threshold(mask, mask, 1, 255, cv::THRESH_BINARY_INV);
      

      for(int i = num_frames - 1; i >= 0; --i)
      {
        // blendImages(prev_frames[i], img, mask);
        cv::addWeighted(img, 1.0, prev_frames[i], 0.5 / (i + 2), 0.0, img);
      }

      cv::addWeighted(img, 1.0, cur_img, 1.0, 0.0, img);

      cv::applyColorMap(img, img, cv::COLORMAP_TWILIGHT);//COLORMAP_TWILIGHT или COLORMAP_TWILIGHT_SHIFTED
      
      cv::imshow("Spectrum original", img);

      for(int i = num_frames - 1; i > 0; --i)
      {
        prev_frames[i] = prev_frames[i - 1].clone();
      }
      cur_img.copyTo(prev_frames[0]);
    }
  }

  if(use_mic) 
  {
    StopAudioStream(stream);
    UnloadAudioStream(stream);
  } 
  else 
  {
    UnloadMusicStream(music);
  }
  CloseAudioDevice();

  return 0;
}