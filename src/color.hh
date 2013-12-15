#ifndef COLOR_HH_
# define COLOR_HH_

#include <cv.h>

// This files describe how a color is represented in our raytracer.
// For simplicity purposes, color values are represented as a double between
// 0 and 1
// We add max_val_ which is the maximum possible parameter in order to simplify
// normalisation. A created color has a max_val of 1, the result of rgb1 + rgb2
// has 2; An "empty" color has a max_val_ of 0 (for instance, the color value
// when a launched ray hit nothing).

class Color
{
  public:
    Color(int r, int g, int b)
    {
      r_ = r/255.;
      g_ = g/255.;
      b_ = b/255.;
      max_val_ = 1;
    }
    Color (double r, double g, double b) : r_(r), g_(g), b_(b), max_val_(1) { }
    Color (double r, double g, double b, double max_val) : r_(r), g_(g), b_(b), max_val_(max_val) { }
    Color () : r_(0), g_(0), b_(0), max_val_(0) {} // FIXME: remove ?

    double r() {return r_;}
    double g() {return g_;}
    double b() {return b_;}
    double max() {return max_val_;}

    // Conversion to the color representation of OpenCV
    cv::Vec3b toBgr()
    {
      if (max_val_ == 0)
        return cv::Vec3b(0,0,0);
      int r = (r_/max_val_) * 255;
      int g = (g_/max_val_) * 255;
      int b = (b_/max_val_) * 255;
      return cv::Vec3b(b,g,r);
    }

  private:
    double r_, g_, b_, max_val_;
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
