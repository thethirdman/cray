#include "utils.hh"

double normal_rand(void)
{
  return (double) (rand())/static_cast<double>(RAND_MAX);
}

double clamp_zero(double val)
{
  return (val < 0 ? 0 : val);
}

double clamp_one(double val)
{
  return (val > 1 ? 1 : val);
}

cv::Vec3d normalize(cv::Vec3d vec)
{
  if (vec[0] == 0 && vec[1] == 0 && vec[2] == 0)
    return vec;
  return (1.0/cv::norm(vec)) * vec;
}
