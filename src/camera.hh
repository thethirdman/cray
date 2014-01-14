#ifndef CAMERA_HH_
# define CAMERA_HH_

#include <vector>
#include <cv.h>
#include "vector.hh"
#include "ray.hh"
#include "utils.hh"
#include <math.h>

#define _USE_MATH_DEFINES

// This file contains the camera definition, that is the class
// that describes the viewer location in the scene, and how the rays
// are launched from this location

class Camera
{
  public:
    Camera(Vec3d pos, Vec3d dir, Vec3d up, int x, int y)
      : x_(x), y_(y), pos_(pos), dir_(dir), up_(up)
    {
      double fov = M_PI_4;
      fovx_ = tan(fov);
      fovy_ = ((double) y_)/((double) x_) * M_PI_4;
    }

    static Camera* parse(tinyxml2::XMLNode* node, int x, int y);

    std::vector<Ray>& getRays(void)
    {
      std::vector<Ray>* rayMat = new std::vector<Ray> (x_ * y_);

      // Those two vector define the tangent plane to the camera direction
      // that we use to cast the rays
      Vec3d cam_right = normalize(up_.cross(dir_));
      Vec3d cam_up    = normalize(dir_.cross(cam_right));
      double xd = static_cast<double>(x_);
      double yd = static_cast<double>(y_);

      Vec3d center = pos_ + dir_;

      for (int j = 0; j < y_; j++)
        for (int i = 0; i < x_; i++)
        {
          // The pixel (i,j) is normalised, centered, and the affected by the
          // FOV
          double normal_i = (static_cast<double>(i)/xd - 0.5) * fovx_;
          double normal_j = (static_cast<double>(j)/yd - 0.5) * fovy_;

          // it is then projected on the tangent plane
          Vec3d pt = normal_i * cam_right + normal_j * cam_up + center;
          Vec3d pt_dir = normalize(pt - pos_);

          rayMat->at(j * x_ + i) = Ray(pt, pt_dir);
        }
      return *rayMat;
    }
  private:
    // The dimension of the image generated
    int x_;
    int y_;
    // The size of the field of view
    double fovx_;
    double fovy_;

    // The position of the camera in the scene
    Vec3d pos_;
    // Its viewing direction
    Vec3d dir_;
    // Its vertical orientation
    Vec3d up_;
};

#undef _USE_MATH_DEFINES

#endif
