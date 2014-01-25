#ifndef RAY_HH_
# define RAY_HH_

#include <cv.h>
#include "vector.hh"

class Ray
{
  public:
    Ray(Vec3d orig, Vec3d dir) : orig_(orig), dir_(dir) {}
    Ray() {}
    Vec3d orig() const {return orig_;}
    Vec3d dir() const {return dir_;}
    Ray op_dir() const {return Ray(orig_, -dir_);}
  private:
    // A ray as an origin and a direction
    Vec3d orig_;
    Vec3d dir_;
};

#endif
