#ifndef CAMERA_HH_
# define CAMERA_HH_

#include <vector>
#include <cv.h>
#include "ray.hh"
#include "utils.hh"
#include <math.h>

#define _USE_MATH_DEFINES

class Camera
{
  public:
    Camera(cv::Vec3i pos, cv::Vec3d dir, cv::Vec3d up, int x, int y)
      : pos_(pos), dir_(dir), up_(up), x_(x), y_(y)
    {
      double fov = M_PI_4;
      fovx_ = tan(fov);
      fovy_ = ((double) y_)/((double) x_) * M_PI_4;
    }

    std::vector<Ray>& getRays(void)
    {
      std::vector<Ray>* rayMat = new std::vector<Ray> (x_ * y_);
      cv::Vec3d cam_right = normalize(up_.cross(dir_));
      cv::Vec3d cam_up    = normalize(dir_.cross(cam_right));
      double xd = (double) x_;
      double yd = (double) y_;


      fovx_ = 1;
      fovy_ = 1;
      cv::Vec3d center = pos_ + dir_;
      for (int j = 0; j < y_; j++)
        for (int i = 0; i < x_; i++)
        {
          double normal_i = (((double) i)/xd - 0.5) * fovx_;
          double normal_j = (((double) j)/yd - 0.5) * fovy_;

          cv::Vec3d pt = normal_i * cam_right + normal_j * cam_up + center;
          cv::Vec3d pt_dir = normalize(pt - pos_);

          rayMat->at(j * x_ + i) = Ray(pt, pt_dir);
        }
      return *rayMat;
    }
  private:
    int x_;
    int y_;
    double fovx_;
    double fovy_;
    cv::Vec3d pos_;
    cv::Vec3d dir_;
    cv::Vec3d up_;
};

#undef _USE_MATH_DEFINES

#endif
