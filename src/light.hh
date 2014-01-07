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
    Light(cv::Vec3d orig, Color color) : orig_(orig), radius_(0), samples_(0), color_(color) {}
    Light(cv::Vec3d orig, float radius, int samples, Color color)
      : orig_(orig), radius_(radius), color_(color)
    {
      // Because of the smoothing, we want have the square root
      std::clog << samples << std::endl;
      if (samples != 0)
        samples_ = static_cast<int>(sqrt(static_cast<double>(samples)));
      else
        samples_ = 0;
    }
    Light() {} // FIXME

    cv::Vec3d orig(void) {return orig_;}
    Color getColor(void) {return color_;}

    Color illumination(Shape& shape, Ray& ray, cv::Vec3d intersection, std::vector<Shape*>& shapes)
    {
      cv::Vec3d cur_orig = orig_;
      Color total_color (0,0,0,0);

      int max = samples_;
      for (int i = 0; i < max * max + 1; i++)
      {
        cv::Vec3d dir_light = normalize(cur_orig - intersection);
        double light_dist = cv::norm(cur_orig - intersection);
        double shadowed = 0.0;
        Ray light_ray (intersection, dir_light);

        // If this ray hits a shape, it shadowed.
        for (unsigned int s = 0; s < shapes.size(); s++)
        {
          cv::Vec3d hit;
          double dist;

          if (shapes[s]->intersect(light_ray, hit, dist) && &shape != shapes[s])
          {
            if (dist < light_dist)
            {
              shadowed = 1;
              break;
            }
          }
        }

        Material mat = shape.getMaterial();
        //std::cout << "Colors: " << mat.color_at(0,0).r() << std::endl;
        double diffcoef = mat.get_diffuse_coef() * clamp_zero(shape.normal(light_ray).dot(light_ray.dir()) - shadowed);

        Ray refl_light = shape.reflect(light_ray.op_dir());
        double phong = mat.get_specular_coef() * clamp_zero(refl_light.dir().dot(normalize(ray.dir() - light_ray.orig())));

        double ambient = mat.get_ambient_coef();
        total_color = total_color + diffcoef * mat.color_at(0,0) + phong * Color(1,1,1) + ambient * mat.color_at(0,0);

        if (samples_ != 0)
        {
          double shift1 = static_cast<double>(i%max);
          double shift2 = static_cast<double>(i/max);
          double invmax = 1/static_cast<double>(max);
          cur_orig = randSphere(orig_, radius_, shift1 * invmax, shift2 * invmax, invmax);
        }
    }

      return total_color * color_;
    }
    int samples(void) {return samples_;}

  private:
    cv::Vec3d orig_;
    float radius_;
    int samples_;
    Color color_;

};

#endif
