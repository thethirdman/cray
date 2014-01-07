#ifndef UTILS_HH_
# define UTILS_HH_

#include <cv.h>
#include <tinyxml2.h>
#include <iostream>
#include "ray.hh"

// Parsing utility functions

void assert_node(tinyxml2::XMLNode* node, std::string expName);
bool is_named(std::string name, tinyxml2::XMLNode* node);
cv::Vec3d parseVec(tinyxml2::XMLElement* elt);


// Vector utility functions
cv::Vec3d randSphere(cv::Vec3d center, double radius, double start1, double start2, double len);

// Compute a random value between 0 and 1
double normal_rand(void);

double clamp_zero(double val);
double clamp_one(double val);

// Normalizes a vector
cv::Vec3d normalize(cv::Vec3d vec);

#endif
