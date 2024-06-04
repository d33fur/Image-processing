#include "raylib.h"
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include "a2i/spectrogram.hpp"

int WINDOW_WIDTH = 1600;
int WINDOW_HEIGHT = 1200;
unsigned int FRAME_SIZE = 512;
a2i::Spectrogram g;

typedef struct {
  float left;
  float right;
} Frame;

void callback(void *bufferData, unsigned int frames) {
  if(frames < FRAME_SIZE) return;

  Frame *fs = static_cast<Frame*>(bufferData);

  for(size_t i = 0; i < frames; ++i) {
    g.in[i] = (fs[i].left + fs[i].right) / 2;
  }

  // домножить на оконную функцию
  g.addWindow();

  //вызвать fft
  g.fft();

  //нормализовать
  g.normalize();
}


int main(int argc, char** argv) {
  cv::CommandLineParser parser(argc, argv,
  "{ path |  | path to audiofile }");
  // "{ method | 2d_simple_lines | 2d_simple_lines/2d_simple_lines_low/2d_log_lines/2d_log_bezie }");

  auto file = parser.get<std::string>("path");
  const char *file_path = file.c_str();

  cv::namedWindow("Spectrum original", cv::WINDOW_NORMAL);
  cv::resizeWindow("Spectrum original", WINDOW_WIDTH, WINDOW_HEIGHT);

  InitWindow(800, 600, "Spectrum");
  SetTargetFPS(60);

  InitAudioDevice();
  Music music = LoadMusicStream(file_path);
  PlayMusicStream(music);
  SetMusicVolume(music, 0.0f); //0.5f


  g.setAudioInfo(music.stream.sampleRate, music.stream.sampleSize, 2, {0, 190});
  g.setFrameSize(FRAME_SIZE);

  g.setWindowFunc(a2i::SINE);
  // g.setWindowFunc(a2i::HANN);
  // g.setWindowFunc(a2i::HAMMING);
  // g.setWindowFunc(a2i::BLACKMAN);
  // g.setWindowFunc(a2i::NUTTALL);
  // g.setWindowFunc(a2i::BLACKMAN_NUTTALL);
  // g.setWindowFunc(a2i::BLACKMAN_HARRIS)
  // g.setWindowFunc(a2i::FLAT_TOP);
  // g.setWindowFunc(a2i::BARTLETT_HANN);
  // g.setWindowFunc(a2i::HANN_POISSON);

  AttachAudioStreamProcessor(music.stream, callback);
  float time_played = 0.0f;

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

    time_played = GetMusicTimePlayed(music) / GetMusicTimeLength(music);
    if (time_played > 1.0f) {
      break;
    }

    cv::Mat img(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3, cv::Scalar(22, 16, 20));
    
    // g.drawGrid(img, a2i::LOG, 1, {0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4092, 10000}, 10);
    g.drawGrid(img, a2i::LOG, 1);
    // g.drawSpectrum(img);

    // g.draw_lines_simple_2d(img);

    g.draw_log_bezie_2d(img);

    // g.draw_log_lines_2d(img);


    cv::applyColorMap(img, img, cv::COLORMAP_BONE);
    
    cv::imshow("Spectrum original", img);
    cv::waitKey(1);

    BeginDrawing();
    ClearBackground(CLITERAL(Color) {0x18, 0x18, 0x18, 0xFF});
    EndDrawing();
  }

  UnloadMusicStream(music);
  CloseAudioDevice();
  CloseWindow();

  cv::waitKey(0);
  return 0;
}