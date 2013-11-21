#ifndef LIGHT_HH_
# define LIGHT_HH_

// Basic light definition
// It may be interesting to define a light with a dimension in order
// to make soft shadows ?
class Light
{
  public:
    Light(cv::Vec3d orig, Color color) : orig_(orig), color_(color) {}
    Light() {} // FIXME

    cv::Vec3d orig(void) {return orig_;}
    Color getColor(void) {return color_;}

  private:
    cv::Vec3d orig_;
    Color color_;

};

#endif
