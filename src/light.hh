#ifndef LIGHT_HH_
# define LIGHT_HH_

class Light
{
  public:
    Light(cv::Vec3d orig, Color color) : orig_(orig), color_(color) {}
    Light() {}

    cv::Vec3d orig(void) {return orig_;}
    Color getColor(void) {return color_;}

  private:
    cv::Vec3d orig_;
    Color color_;

};

#endif
