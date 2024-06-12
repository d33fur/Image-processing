#include "raylib.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include "a2i/spectrogram.hpp"

int WINDOW_WIDTH = 1600;
int WINDOW_HEIGHT = 1200;
unsigned int FRAME_SIZE = 512;
a2i::Spectrogram g;

// int counter = 0;
int multiplier = 20;
bool show = false;


typedef struct 
{
  float left;
  float right;
} Frame;

void callback(
  void *bufferData, 
  unsigned int frames) 
{
  if(frames < 512) return;

  Frame *fs = static_cast<Frame*>(bufferData);

  if(g.in.size() == FRAME_SIZE)
  {
    g.in.erase(g.in.begin(), g.in.begin() + frames);
  }

  for(size_t i = 0 ; i < frames; ++i) 
  {
    g.in.push_back((fs[i].left + fs[i].right) / 2);
  }

  if(g.in.size() == FRAME_SIZE)
  {
    g.addWindow();
    g.fft();
    g.normalize(multiplier);
    show = true;
  }
  else
  {
    show = false;
  }
}

cv::Scalar parseColor(const std::string& colorStr) 
{
  std::vector<int> values;
  std::stringstream ss(colorStr);
  std::string item;

  while (std::getline(ss, item, ',')) 
  {
    try 
    {
        values.push_back(std::stoi(item));
    } catch (const std::invalid_argument& e) 
    {
      throw std::invalid_argument("Invalid color component: " + item);
    }
  }

  if (values.size() != 3) 
  {
    throw std::invalid_argument("Invalid color string: " + colorStr);
  }

  return cv::Scalar(values[0], values[1], values[2]);
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

int main(int argc, char** argv) 
{
  cv::CommandLineParser parser(argc, argv,
  "{ p          |               | path to audiofile             }"
  "{ a          |    -90,6      | amplitude range               }"
  "{ w          |       0       | window function(0-9)          }"
  "{ l          |       0       | line type(0-2)                }"
  "{ g          |       1       | graph mode(0-1)               }"
  "{ m          |       20      | mormalize multiplier          }"
  "{ mic        |               | use microphone                }"
  "{ f          |      512      | frame size(>=512)             }"
  "{ n          |       3       | number of prev frames         }"
  "{ size       |   1200,1600   | window size(height,width)     }"
  "{ grad       |       18      | colormap(0-21)                }"
  "{ fill       |       2       | fill type(0-2)                }"
  "{ border     |               | border line                   }"
  "{ line       |  255,255,255  | line color(255,255,255)       }"
  "{ underline  |  127,127,127  | underline color(127,127,127)  }"
  "{ grad_coef  |      127      | gradient coefficient(0-255)   }"
  "{ video_path |               | path to save video            }");

  auto file = parser.get<std::string>("p");
  const char *file_path = file.c_str();
  auto amp = parseRange(parser.get<std::string>("a"));
  auto window_function = parser.get<int>("w");
  auto line_type = parser.get<int>("l");
  auto graph_mode = parser.get<int>("g");
  multiplier = parser.get<int>("m");
  auto use_mic = parser.has("mic");
  FRAME_SIZE = parser.get<unsigned int>("f");
  auto num_frames = parser.get<int>("n");
  auto window_size = parseRange(parser.get<std::string>("size"));
  WINDOW_HEIGHT = window_size.first;
  WINDOW_WIDTH = window_size.second;
  auto colormap = parser.get<int>("grad");
  auto fill_type = parser.get<int>("fill");
  auto border_line = parser.has("border");
  auto line_color = parseColor(parser.get<std::string>("line"));
  auto underline_color = parseColor(parser.get<std::string>("underline"));
  auto grad_coefficient = parser.get<int>("grad_coef");
  auto video_path = parser.get<std::string>("video_path");
  // auto generate_only = parser.has("video_path");

  bool stop = true;

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
    SetMusicVolume(music, 0.8f);
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
  g.drawGrid(grid, graph_mode, 1, {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000}, 10);

  cv::VideoWriter video;

  if(!video_path.empty()) 
  {
    video.open(video_path, cv::VideoWriter::fourcc('a', 'v', 'c', '1'), 60, cv::Size(WINDOW_WIDTH, WINDOW_HEIGHT));
  }

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

      int key;

      if(!video.isOpened())
      {
        key = cv::waitKey(1);
      
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
    }

    if(show)
    {
      img = cv::Mat(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
      cv::add(grid, img, img);
      cur_img = cv::Mat::zeros(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);

      g.drawSpectrum(cur_img, line_type, graph_mode, fill_type, border_line, line_color, underline_color, grad_coefficient);

      for(int i = num_frames - 1; i >= 0; --i)
      {
        // cv::addWeighted(img, 1.0, prev_frames[i], 0.3 / (i+1), 0.0, img);
      }

      cv::addWeighted(img, 1.0, cur_img, 1.0, 0.0, img);
      cv::applyColorMap(img, img, colormap);
      

      if(video.isOpened()) 
      {
        video.write(img);
      }
      else
      {
        cv::imshow("Spectrum original", img);
      }

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

  if(video.isOpened()) 
  {
    video.release();
  }

  return 0;
}