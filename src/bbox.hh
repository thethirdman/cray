#ifndef BBOX_HH_
# define BBOX_HH_

#include <cv.h>
#include "ray.hh"
#include "utils.hh"

class BBox
{
  public:
    BBox() {};

    BBox(cv::Vec3d min, cv::Vec3d max) : minpt(min), maxpt(max) { }

    BBox merge(BBox other)
    {
      cv::Vec3d minp = minVec(minVec(minpt, other.minpt), minVec(maxpt, other.maxpt));
      cv::Vec3d maxp = maxVec(maxVec(minpt, other.minpt), maxVec(maxpt, other.maxpt));

      minpt = minp;
      maxpt = maxp;

      return BBox(minp, maxp);
    }

    bool mustShoot(Ray& ray)
    {
      double tmin = 0;
      double tmax = std::numeric_limits<double>::max();
      for (int i = 0; i < 3; i++)
      {
        if (ray.dir()[i] == 0)
        {
          if (ray.orig()[i] < minpt[i] || ray.orig()[i] > maxpt[i])
            return false;
        }
        else
        {

          double inv = (1./ray.dir()[i]);
          double close = (minpt[i] - ray.orig()[i]) * inv;
          double far = (maxpt[i] - ray.orig()[i]) * inv;

          if (close > far)
            std::swap(close, far);

          tmin = std::max(tmin, close);

          tmax = std::min(tmax, far);

          if (tmin > tmax)
            return false;
        }
      }
      return true;
    }

    cv::Vec3d minpt;
    cv::Vec3d maxpt;
};

#endif // BBOX
