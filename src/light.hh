#ifndef LIGHT_HH_
# define LIGHT_HH_

// Basic light definition
// It may be interesting to define a light with a dimension in order
// to make soft shadows ?

#include "shape.hh"
#include "kdtree.hh"
#include "vector.hh"
#include <tinyxml2.h>
#include <cmath>

class Light
{
  public:
    static Light parse(tinyxml2::XMLNode* node);

    Light(Vec3d orig, Color color) : orig_(orig), radius_(0), samples_(0), color_(color) {}

    Light(Vec3d orig, float radius, int samples, Color color)
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

    Vec3d orig(void) {return orig_;}

    Color getColor(void) {return color_;}

    Color illumination(Shape& shape, Ray& ray, Vec3d intersection, KDTree& shapes)
    {
      Vec3d cur_orig = orig_;
      Color total_color (0,0,0,0);

      int max = samples_;
      for (int i = 0; i < max * max + 1; i++)
      {
        Vec3d dir_light = normalize(intersection - cur_orig);
        double light_dist = (cur_orig - intersection).norm();
        double shadowed = 0.0;
        Ray light_ray (cur_orig , dir_light);
        const double shift = std::numeric_limits<double>::epsilon() * 2048;
        Ray shadow_ray (intersection + shift * dir_light, -dir_light);

        // If this ray hits a shape, it shadowed.
        Vec3d hit;
        double dist;

        if (shapes.intersect(light_ray, hit, dist))
        {
          if (dist + shift < light_dist)
            shadowed = 1;
        }
        //shadowed = 0;

        const Material& mat = shape.getMaterial();
        double diffcoef = mat.get_diffuse_coef() * clamp_zero(shape.normal(shadow_ray).dot(-light_ray.dir()) - shadowed);

        Ray refl_light = shape.reflect(shadow_ray.op_dir());
        double phong = (diffcoef <= 0 ? 0
            : mat.get_specular_coef() * clamp_zero(refl_light.dir().dot(normalize(ray.orig() - intersection))));

        Color acolor = mat.get_ambient_coef() *
            /*
            mat.color_at(0,0);
            // */
            //*
            shape.getColorAt(intersection);
            // */

        Color dcolor = diffcoef * mat.get_diffuse_coef() *
            /*
            mat.color_at(0,0);
            // */
            //*
            shape.getColorAt(intersection);
            // */

        Color scolor = pow(phong, mat.get_brilliancy()) * Color(1,1,1);

        total_color  = total_color + satSum(satSum(acolor, dcolor), scolor);

        // Normal debug
        //Vec3d normal = shape.normal(shadow_ray);
        //total_color = Color(fabs(normal[0]), fabs(normal[1]), fabs(normal[2]));

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
    Vec3d orig_;
    float radius_;
    int samples_;
    Color color_;

};

#endif
