#ifndef SHAPE_HH_
# define SHAPE_HH_

#include "ray.hh"
#include "utils.hh"
#include "color.hh"

class Shape
{
  public:
    // Returns the normal to a shape at the point of intersection, or a null
    // pointer otherwise
    virtual std::pair<cv::Vec3d, double>* intersect(Ray ray) = 0;
    virtual cv::Vec3d normal(cv::Vec3d pt) = 0;
    Color getColor(void)
    {
      return color_;
    }
  protected:
    Shape(Color col) : color_(col) {}
    Color color_;
};

class Sphere : public Shape
{
  public:
    Sphere(cv::Vec3i c, Color color, int r) : center_(c), Shape(color), radius_(r) {}

    std::pair<cv::Vec3d, double>* intersect(Ray ray)
    {
      double a = ray.dir().dot(ray.dir());
      cv::Vec3d o_c = ray.orig() - center_; // O-C
      double b = 2. * ray.dir().dot(o_c);
      double c = o_c.dot(o_c) - radius_ * radius_;

      double delta = b * b - 4 * a *c;

      if (delta >= 0)
      {
        double sq_delta = sqrt(delta);
        double t1 = (-b - sq_delta) / (2*a);
        double t2 = (-b + sq_delta) / (2*a);
        double mint;
        if (t1 > 0 && t2 > 0)
          mint = std::min(t1,t2);
        else if (t1 > 0)
          mint = t1;
        else if (t2 > 0)
          mint = t2;
        else
          return NULL;

        /// FIXME: for now, only one of the two
        cv::Vec3d intersect = ray.orig() + mint * ray.dir();
        return new std::pair<cv::Vec3d, double>(intersect, mint);
      }
      else
        return NULL;
    }

    cv::Vec3d normal(cv::Vec3d pt)
    {
      return normalize(pt - center_);
    }

  private:
    cv::Vec3d center_;
    int radius_;
};

class Triangle : public Shape
{
  public:
    Triangle(cv::Vec3d pt1, cv::Vec3d pt2, cv::Vec3d pt3, Color color)
      : pt1_(pt1), pt2_(pt2), pt3_(pt3),  Shape(color)
    {
      cv::Vec3d e1 = pt2_ - pt1_;
      cv::Vec3d e2 = pt3_ - pt2_;
      normal_ = normalize(e1.cross(e2));
    }
    std::pair<cv::Vec3d, double>* intersect(Ray ray)
    {
      cv::Vec3d e1 = pt2_ - pt1_;
      cv::Vec3d e2 = pt3_ - pt2_;
      cv::Vec3d e3 = pt1_ - pt3_;

      // FIXME: name
      double k = normal_.dot(pt1_);

      double t = (k - ray.orig().dot(normal_))/(ray.dir().dot(normal_));
      cv::Vec3d intersect = ray.orig() + t * ray.dir();

      if (t > 0
          && (e1.dot(intersect - pt1_) > 0)
          && (e2.dot(intersect - pt2_) > 0)
          && (e3.dot (intersect - pt3_) > 0))
        return new std::pair<cv::Vec3d, double>(intersect, t);
      return NULL;
    }
    cv::Vec3d normal(cv::Vec3d pt)
    {
      return normal_;
    }

  private:
    cv::Vec3d pt1_;
    cv::Vec3d pt2_;
    cv::Vec3d pt3_;
    cv::Vec3d normal_;
};

#endif
