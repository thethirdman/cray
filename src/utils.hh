#ifndef UTILS_HH_
# define UTILS_HH_

#include <cv.h>
#include "ray.hh"

// Utility functions

// Compute a random value between 0 and 1
double normal_rand(void);

double clamp_zero(double val);
double clamp_one(double val);

// Normalizes a vector
cv::Vec3d normalize(cv::Vec3d vec);

#endif
