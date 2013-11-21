#ifndef UTILS_HH_
# define UTILS_HH_

#include <cv.h>

// Utility functions

// Compute a random value between 0 and 1
double normal_rand(void);

// Normalizes a vector
cv::Vec3d normalize(cv::Vec3d vec);

#endif
