#ifndef LIGHT_HH_
# define LIGHT_HH_

// Basic light definition
// It may be interesting to define a light with a dimension in order
// to make soft shadows ?

#include "shape.hh"
#include <tinyxml2.h>
#include <cmath>

class Light
{
  public:
    static Light parse(tinyxml2::XMLNode* node);
    Light(cv::Vec3d orig, Color color) : orig_(orig), color_(color) {}
    Light() {} // FIXME

    cv::Vec3d orig(void) {return orig_;}
    Color getColor(void) {return color_;}
    Color illumination(Shape& shape, Ray& ray, Ray& light_ray, double shadowed)
    {
      // Lambert light diffusion method.
      // The dot product defines a "lightning" coefficient which ponderates the
      // intensity of the color of the shape.
      // Shadows apply only to the diffuse light vaue
      //shadowed = 0;
      double diffval = clamp_zero(shape.normal(light_ray).dot(light_ray.dir()) - shadowed);

      // Phong specular light
      Ray tmp = light_ray.op_dir();
      Ray refl_light = shape.reflect(tmp);
      double phong = clamp_zero(refl_light.dir().dot(normalize(ray.dir() - light_ray.orig())));

      // Ambient light
      double ambient = 0.1;
      Material mat = shape.getMaterial();
      Color color = satSum(
                      satSum((ambient * mat.get_ambient_coef()),
                             (diffval * mat.get_diffuse_coef())),
                      pow(phong, mat.get_brilliancy()) * mat.get_specular_coef());

      return color * color_;
    }

  private:
    cv::Vec3d orig_;
    Color color_;

};

#endif
