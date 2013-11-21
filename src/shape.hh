#ifndef SHAPE_HH_
# define SHAPE_HH_

#include "ray.hh"
#include "utils.hh"
#include "color.hh"

// Abstract class shape
class Shape
{
  public:
    // Returns the normal to a shape at the point of intersection, or a null
    // pointer otherwise
    virtual bool intersect(Ray ray, cv::Vec3d& intersect, double& dist) = 0;

    // The normal vector to a shape at the intersection point pt
    virtual cv::Vec3d normal(cv::Vec3d pt) = 0;

    Color getColor(void)
    {
      return color_;
    }
    double refl(void) { return refl_coef_;}
  protected:
    Shape(Color col, double refl) : color_(col), refl_coef_(refl) {}
    Color color_;
    double refl_coef_;
};

class Sphere : public Shape
{
  public:
    Sphere(cv::Vec3d c, Color color, double r, double refl) : center_(c), Shape(color, refl), radius_(r) {}

    bool intersect(Ray ray, cv::Vec3d& intersect, double& dist)
    {
      double a = ray.dir().dot(ray.dir());
      cv::Vec3d o_c = ray.orig() - center_; // O-C
      double b = 2. * ray.dir().dot(o_c);
      double c = o_c.dot(o_c) - radius_ * radius_;

      double delta = b * b - 4 * a *c;

      // If there is an intersection
      if (delta >= 0)
      {
        double sq_delta = sqrt(delta);
        double t1 = (-b - sq_delta) / (2*a);
        double t2 = (-b + sq_delta) / (2*a);
        double mint;
        // If the root is negative, that means that the solution is "behind"
        // the origin of the ray
        if (t1 > 0 && t2 > 0)
          mint = std::min(t1,t2);
        else if (t1 > 0)
          mint = t1;
        else if (t2 > 0)
          mint = t2;
        else
          return NULL;

        // FIXME: for now, only one of the two solutiosn is returned
        // it is the smallest distance. Not sure if this is a good idea
        intersect = ray.orig() + mint * ray.dir();
        dist = mint;
        return true;
      }
      else
        return false;
    }

    cv::Vec3d normal(cv::Vec3d pt)
    {
      return normalize(pt - center_);
    }

  private:
    cv::Vec3d center_;
    double radius_;
};

class Plane : public Shape
{
  public:
    Plane(cv::Vec3d pt1, cv::Vec3d dir1, cv::Vec3d dir2, Color color, double refl)
    : pt1_(pt1), dir1_(dir1), dir2_(dir2), Shape(color, refl)
    {
      normal_ = normalize(dir1.cross(dir2));
    }

    bool intersect(Ray ray, cv::Vec3d& intersect, double& dist)
    {
      // We first compute the intersection between the ray and the plane in
      // which the triangle is included
      // FIXME: name
      double k = normal_.dot(pt1_);

      double t = (k - ray.orig().dot(normal_))/(ray.dir().dot(normal_));
      intersect = ray.orig() + t * ray.dir();
      dist = t;
      if (t > 0)
        return true;
      else
        return false;
    }
    cv::Vec3d normal(cv::Vec3d pt)
    {
      return normal_;
    }

  protected:
    cv::Vec3d pt1_;
    cv::Vec3d dir1_;
    cv::Vec3d dir2_;
    cv::Vec3d normal_;
};

class Triangle : public Shape
{
  public:
    Triangle(cv::Vec3d pt1, cv::Vec3d pt2, cv::Vec3d pt3, Color color, double refl)
      : Shape(color, refl), pt1_(pt1), pt2_(pt2), pt3_(pt3)
      {
        normal_ = normalize((pt3_ - pt2_).cross(pt1_ - pt3_));
      }


    bool intersect(Ray ray, cv::Vec3d& intersect, double& dist)
    {
      // The edges of the triangles
      cv::Vec3d e1 = (pt2_ - pt1_);
      cv::Vec3d e2 = (pt3_ - pt1_);

      cv::Vec3d p = ray.dir().cross(e2);
      double det = e1.dot(p);
      double inv_det = 1. / det;

      cv::Vec3d t = ray.orig() - pt1_;

      double u = t.dot(p) * inv_det;
      if (u < 0 || u > 1)
        return false;

      cv::Vec3d q = t.cross(e1);
      double v = ray.dir().dot(q) * inv_det;
      if (v < 0 || u + v > 1)
        return false;
      dist = e2.dot(q) * inv_det;
      intersect = ray.orig() + dist * ray.dir();
      if (dist > 0)
        return true;
      else
        return false;
    }

    cv::Vec3d normal(cv::Vec3d pt)
    {
      return normal_;
    }

  private:
    // A triangle is defined by 3 points in the space
    cv::Vec3d pt1_;
    cv::Vec3d pt2_;
    cv::Vec3d pt3_;

    // The direction of the normal is the same for the whole triangle, so, we
    // precompute it
    cv::Vec3d normal_;
};

#endif
