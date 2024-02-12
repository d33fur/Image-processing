#include <iostream>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

#define w 400

void MyFilledCircle(Mat img, Point center) {
  circle(img,
    center,
    w/4,
    Scalar(220, 220, 220),
    FILLED,
    LINE_8);
}

void MyFilledSquare(Mat img, Point leftUpCorner, Point rightDownCorner) {
  rectangle( img,
    leftUpCorner,
    rightDownCorner,
    Scalar(127, 127, 127),
    FILLED,
    LINE_8 );
}

int main( void ){
  Mat image = Mat::zeros(w, w, CV_8UC3);
  // Mat rook_image = Mat::zeros( w, w, CV_8UC3 );


  MyFilledSquare(image, Point(50, 50), Point(w - 50, w - 50));
  MyFilledCircle(image, Point(w / 2, w / 2));

  vector<Mat> bgr_planes;
  split(image, bgr_planes);
  int histSize = 8;
  float range[] = { 0, 256 }; //the upper boundary is exclusive
  const float* histRange[] = { range };
  bool uniform = true, accumulate = false;
  Mat b_hist, g_hist, r_hist;
  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, histRange, uniform, accumulate );

  int hist_w = 512, hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );
  Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );


  for( int i = 1; i < histSize; i++ ) {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ),
            Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
            Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ),
            Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
            Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ),
            Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
            Scalar( 0, 0, 255), 2, 8, 0  );
  }

  imshow("calcHist Demo", histImage );

  imshow("Drawing 1", image);
  waitKey(0);

  return(0);
}