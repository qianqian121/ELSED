#include <iostream>
#include <opencv2/opencv.hpp>
#include <zconf.h>
#include "ELSED.h"

inline void
drawSegments(cv::Mat img,
             upm::Segments segs,
             const cv::Scalar &color,
             int thickness = 1,
             int lineType = cv::LINE_AA,
             int shift = 0) {
  for (const upm::Segment &seg: segs)
    cv::line(img, cv::Point2f(seg[0], seg[1]), cv::Point2f(seg[2], seg[3]), color, thickness, lineType, shift);
}

cv::Mat camera_matrix;
cv::Mat dist_coeffs;
cv::Mat undist_camera_matrix;

void Undistort(cv::Mat& img, cv::Mat& dst) {
  cv::undistort(img, dst, camera_matrix, dist_coeffs, undist_camera_matrix);
}

int main() {
  std::cout << "******************************************************" << std::endl;
  std::cout << "******************* ELSED main demo ******************" << std::endl;
  std::cout << "******************************************************" << std::endl;

  std::string config_file = "/mnt/project/github/ELSED/config/camera_0231_002.yaml";
  cv::FileStorage cameraSettings(config_file, cv::FileStorage::READ);
  if (!cameraSettings.isOpened()) {
    std::cerr << "Failed to open settings file at: " << config_file
              << std::endl;
    return -1;
  }

  cameraSettings["CameraMat"] >> camera_matrix;
  cameraSettings["DistCoeffs"] >> dist_coeffs;
  cameraSettings["UndistCameraMat"] >> undist_camera_matrix;
//  std::string img_path = "../images/P1020829.jpg";
//  std::string img_path = "../images/building.jpg";
  std::string img_path = "../images/image-2021-12-23-00-45-38.png";
//  std::string img_path = "../images/image-2022-01-02-22-33-49.png";
//  std::string img_path = "../images/image-2021-12-22-22-21-13.png";
  // range image, intensity image, 3 outdoor scenes
//  std::string img_path = "../images/range.png";
//  std::string img_path = "/mnt/data/reconstruction/bundlefusion/copyroom/copyroom/frame-000000.color.jpg";

  // Using default parameters (long segments)
  cv::Mat img_src = cv::imread(img_path);
  if (img_src.empty()) {
    std::cerr << "Error reading input image" << std::endl;
    return -1;
  }
  cv::Mat img_dst;
  Undistort(img_src, img_dst);
  cv::Mat img = img_dst.clone();

  upm::ELSED elsed;
  upm::Segments segs = elsed.detect(img);
  std::cout << "ELSED detected: " << segs.size() << " (large) segments" << std::endl;

  drawSegments(img, segs, CV_RGB(0, 255, 0), 1);
  cv::imshow("ELSED long", img);
//  cv::waitKey();

  img = img_dst.clone();
  upm::ELSEDParams params;
  params.listJunctionSizes = {};
  upm::ELSED elsed_short(params);
  segs = elsed_short.detect(img);
  std::cout << "ELSED detected: " << segs.size() << " (short) segments" << std::endl;

  drawSegments(img, segs, CV_RGB(0, 255, 0), 1);
//  cv::imshow("ELSED short", img);
//  cv::waitKey(100000);

//  int _refine = LSD_REFINE_STD, double _scale = 0.8,
//  double _sigma_scale = 0.6, double _quant = 2.0, double _ang_th = 22.5,
//  double _log_eps = 0, double _density_th = 0.7, int _n_bins = 1024
//  cv::Ptr<cv::LineSegmentDetector> ls = cv::createLineSegmentDetector();
  cv::Ptr<cv::LineSegmentDetector> ls = cv::createLineSegmentDetector(cv::LSD_REFINE_STD, 0.8, 0.6, 0.25, 22.5);
  std::vector<cv::Vec4f> lines_std;

  // Detect the lines
  cv::Mat img_gray = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
  if (img_gray.empty()) {
    std::cerr << "Error reading input image" << std::endl;
    return -1;
  }
  cv::Mat img_dst_gray = img_gray;
//  cv::Mat img_dst_gray;
//  Undistort(img_gray, img_dst_gray);
  ls->detect(img_dst_gray, lines_std);
  ls->drawSegments(img_dst_gray, lines_std);
  cv::imshow("cv LSD std refine", img_dst_gray);
  while(1) {
    std::cout << ".";
    sleep(1);
    cv::imshow("ELSED short", img);
  cv::waitKey(100000);
  }
  return 0;
}
