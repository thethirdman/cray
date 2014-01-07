#include "utils.hh"
#include <boost/math/constants/constants.hpp>

// Parsing utilities

void assert_node(tinyxml2::XMLNode* node, std::string expName)
{
  if (node)
  {
    const char* nodeName = node->ToElement()->Name();

    if (expName.compare(nodeName) != 0)
    {
      std::cerr << "Error: invalid node " << nodeName << ", expected node " << expName << std::endl;
      exit(1);
    }
  }
  else
    std::cerr << "Error: expecting node " << expName << std::endl;
}

bool is_named(std::string name, tinyxml2::XMLNode* node)
{
  return (name.compare(node->ToElement()->Name()) == 0);
}

cv::Vec3d parseVec(tinyxml2::XMLElement* elt)
{
  double x = nan("");
  double y = nan("");
  double z = nan("");

  elt->QueryDoubleAttribute("x", &x);
  elt->QueryDoubleAttribute("y", &y);
  elt->QueryDoubleAttribute("z", &z);

  if (isnan(x) || isnan(y) || isnan(z))
  {
    std::cerr << "Error: missing attribute in vec, either x, y or z" << std::endl;
    exit(1);
  }
  return cv::Vec3d(x,y,z);
}

cv::Vec3d randSphere(cv::Vec3d center, double radius, double start1, double start2, double len)
{
  double u = normal_rand() * len + start1;
  double v = normal_rand() * len + start2;

  double theta = 2 * boost::math::constants::pi<double>() * u;
  double phi = acos(2 * v - 1);

  return center
    + cv::Vec3d (radius * sin(phi) * cos(theta),
                radius *  cos(phi) * sin(theta),
                radius * cos(phi));
}


// Vector utilities
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
