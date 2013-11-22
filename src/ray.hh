#ifndef RAY_HH_
# define RAY_HH_

#include <cv.h>

class Ray
{
  public:
    Ray(cv::Vec3d orig, cv::Vec3d dir) : orig_(orig), dir_(dir) {}
    Ray() {}
    cv::Vec3d orig() {return orig_;}
    cv::Vec3d dir() {return dir_;}
    Ray op_dir() {return Ray(orig_, -dir_);}
  private:
    // A ray as an origin and a direction
    cv::Vec3d orig_;
    cv::Vec3d dir_;
};

#endif
