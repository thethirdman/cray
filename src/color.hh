#ifndef COLOR_HH8
# define COLOR_HH_

#include <cv.h>

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
    Color () {}

    double r() {return r_;}
    double g() {return g_;}
    double b() {return b_;}
    Color add(Color rgb)
    {
      return Color(r_ + rgb.r(), g_ + rgb.g(), b_ + rgb.b());
    }

    cv::Vec3b toBgr()
    {
      int r = r_ * 255;
      int g = g_ * 255;
      int b = b_ * 255;
      return cv::Vec3b(b,g,r);
    }
    Color operator/ (double c)
    {
      return Color(r_ / c, g_/c, b_/c);
    }
  private:
    double r_, g_, b_;
};

Color operator+ (Color a, Color other);

Color operator* (Color rgb, double val);

Color operator*(double val, Color rgb);
Color operator* (Color a, Color other);

Color operator+(Color rgb, double val);
Color operator-(Color rgb, double val);
#endif
