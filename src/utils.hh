#ifndef UTILS_HH_
# define UTILS_HH_

#include <tinyxml2.h>
#include <iostream>
#include <ostream>
#include "ray.hh"

// Parsing utility functions

void assert_node(tinyxml2::XMLNode* node, std::string expName);
bool is_named(std::string name, tinyxml2::XMLNode* node);
Vec3d parseVec(tinyxml2::XMLElement* elt);

// Vector utility functions
Vec3d randSphere(Vec3d center, double radius, double start1, double start2, double len);

// Compute a random value between 0 and 1
double normal_rand(void);

double clamp_zero(double val);
double clamp_one(double val);

// Normalizes a vector
Vec3d normalize(Vec3d vec);

Vec3d minVec(Vec3d a, Vec3d b);
Vec3d maxVec(Vec3d a, Vec3d b);

std::ostream& operator<<(std::ostream& out, Vec3d vec);

bool fequals(double a, double b) noexcept;

# define PARSE_ERROR_IF(condition, str) \
    do { \
        if (condition) { \
            std::cerr << "\033[1m\033[31mParse error:\033[0m " << str << std::endl; \
            std::exit(1); \
        } \
    } while (0)

#endif
