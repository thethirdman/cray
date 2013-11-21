#ifndef COLOR_HH8
# define COLOR_HH_

#include <cv.h>

// This files describe how a color is represented in our raytracer.
// For simplicity purposes, color values are represented as a double between
// 0 and 1

class Color
{
  public:
    Color(int r, int g, int b)
    {
      r_ = r/255.;
      g_ = g/255.;
      b_ = b/255.;
    }
    Color (double r, double g, double b) : r_(r), g_(g), b_(b) { }
    Color () {} // FIXME: remove ?

    double r() {return r_;}
    double g() {return g_;}
    double b() {return b_;}

    // Conversion to the color representation of OpenCV
    cv::Vec3b toBgr()
    {
      int r = r_ * 255;
      int g = g_ * 255;
      int b = b_ * 255;
      return cv::Vec3b(b,g,r);
    }

  private:
    double r_, g_, b_;
};

// Basic arithmetic operations on colors, implementation in color.cc
// @Ttm: I'm not sure yet if we should automatically clamp values, and in what
//       case

Color operator+ (Color a, Color other);
Color operator+(Color rgb, double val);

Color operator-(Color rgb, double val);

Color operator* (Color rgb, double val);
Color operator*(double val, Color rgb);
Color operator* (Color a, Color other);


Color operator/ (Color rgb, double c);
#endif
