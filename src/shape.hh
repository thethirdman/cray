#ifndef SHAPE_HH_
# define SHAPE_HH_

#include "ray.hh"
#include "utils.hh"
#include "color.hh"
#include "material.hh"
#include "bbox.hh"

// Abstract class shape
class Shape
{
  public:
    static Shape* parse(tinyxml2::XMLNode* node);
    // Returns the normal to a shape at the point of intersection, or a null
    // pointer otherwise
    virtual bool intersect(Ray ray, cv::Vec3d& intersect, double& dist) = 0;

    // The normal vector to a shape at the intersection point pt
    virtual cv::Vec3d normal(Ray& ray) = 0;
    BBox getBBox() {return bbox_;}

    Material getMaterial(void)
    {
      return material_;
    }
    double refl(void) { return refl_coef_;}

    cv::Vec3d center(void) {return center_;}

    Ray reflect(Ray ray)
    {
      Ray op_ray = ray.op_dir();
      cv::Vec3d normal_dir = this->normal(op_ray);
      // Direction of the reflected vector
      cv::Vec3d refl_dir = ray.dir() - 2.0 * normal_dir * ray.dir().dot(normal_dir);
      //
      // Because of the double imprecision, we have to shift the reflected ray out
      // of the shape. Still, something smells fishy, I think something wrong is
      // done somewhere.
      const double shift = std::numeric_limits<double>::epsilon() * 65536.0;

      return Ray (ray.orig() + normal_dir * shift, normalize(refl_dir));
    }
  protected:
    Shape(Material& mat, double refl) : material_(mat), refl_coef_(refl) {}
    Material& material_;
    double refl_coef_;
    cv::Vec3d center_;
    BBox bbox_;
};

class Sphere : public Shape
{
  public:
    static Sphere* parse(tinyxml2::XMLNode* node);
    Sphere(cv::Vec3d c, Material& mat, double r, double refl)
        : Shape(mat, refl), radius_(r)
    {
      center_ = c;
      cv::Vec3d radVec (radius_, radius_, radius_);
      bbox_ = BBox(center_ - radVec, center_ + radVec);
    }

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

    cv::Vec3d normal(Ray& ray)
    {
      return normalize(ray.orig() - center_);
    }

  private:
    double radius_;
};

class Plane : public Shape
{
  public:
    static Plane* parse(tinyxml2::XMLNode* node);
    Plane(cv::Vec3d pt1, cv::Vec3d dir1, cv::Vec3d dir2, Material& mat, double refl)
    : Shape(mat, refl), pt1_(pt1), dir1_(dir1), dir2_(dir2)
    {
      normal_ = normalize(dir1.cross(dir2));

      cv::Vec3d tdir (1,1,1);
      cv::Vec3d minPt = std::numeric_limits<double>::lowest() * tdir;
      cv::Vec3d maxPt = std::numeric_limits<double>::max() * tdir;

      bbox_ = BBox(minPt, maxPt);
      center_ = pt1;
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
    cv::Vec3d normal(Ray& ray)
    {
      return (normal_.dot(ray.dir()) < 0 ? -normal_ : normal_);
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
    static Triangle* parse(tinyxml2::XMLNode* node);
    Triangle(cv::Vec3d pt1, cv::Vec3d pt2, cv::Vec3d pt3, Material& mat, double refl)
      : Shape(mat, refl), pt1_(pt1), pt2_(pt2), pt3_(pt3)
      {
        normal_ = normalize((pt3_ - pt2_).cross(pt1_ - pt3_));
        bbox_ = BBox(minVec(minVec(pt1_, pt2_), pt3_),
                  maxVec(maxVec(pt1_, pt2_), pt3_));
        center_ = (1./3.) * (pt1 + pt2 + pt3);

        // The edges of the triangles
        e1_ = (pt2_ - pt1_);
        e2_ = (pt3_ - pt1_);

      }


    bool intersect(Ray ray, cv::Vec3d& intersect, double& dist)
    {
      cv::Vec3d p = ray.dir().cross(e2_);
      double det = e1_.dot(p);
      double inv_det = 1. / det;

      cv::Vec3d t = ray.orig() - pt1_;

      double u = t.dot(p) * inv_det;
      if (u < 0 || u > 1)
        return false;

      cv::Vec3d q = t.cross(e1_);
      double v = ray.dir().dot(q) * inv_det;
      if (v < 0 || u + v > 1)
        return false;

      dist = e2_.dot(q) * inv_det;
      intersect = ray.orig() + dist * ray.dir();
      if (dist > 0)
        return true;
      else
        return false;
    }

    cv::Vec3d normal(Ray& ray)
    {
      return (normal_.dot(ray.dir()) < 0 ? -normal_ : normal_);
    }

  private:
    // A triangle is defined by 3 points in the space
    cv::Vec3d pt1_;
    cv::Vec3d pt2_;
    cv::Vec3d pt3_;

    cv::Vec3d e1_;
    cv::Vec3d e2_;

    // The direction of the normal is the same for the whole triangle, so, we
    // precompute it
    cv::Vec3d normal_;
};

#endif
