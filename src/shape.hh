#ifndef SHAPE_HH_
# define SHAPE_HH_

#include <unordered_map>
#include "ray.hh"
#include "utils.hh"
#include "color.hh"
#include "material.hh"
#include "bbox.hh"
#include "vector.hh"

// Abstract class shape
class Shape
{
  public:
    static Shape* parse(tinyxml2::XMLNode* node);

    ~Shape() { delete lazy_texturing_first_point_; }

    // Returns the normal to a shape at the point of intersection, or a null
    // pointer otherwise
    virtual bool intersect(Ray ray, Vec3d& intersect, double& dist) const = 0;

    // The normal vector to a shape at the intersection point pt
    virtual Vec3d normal(Ray& ray) = 0;

    BBox getBBox() const {return bbox_;}

    /* Returns the Color at this Shape's surface_point.
     * Be sure that surface_point really is contained by this Shape!
     */
    Color getColorAt(const Vec3d& surface_point) const;

    /* Given a point (where), compute the shape's color at that point
     * according to the texture described in material_. If the point is
     * located on the Shape's surface, this method returns true and set its
     * out argument to the appropriate color; otherwise, false is returned and
     * out is not changed.
     * This method does not update computed_color_points_: this is
     * getColorAt's job.
     */
    virtual bool computeColorFromTexture(const Vec3d& where, Color& out) const = 0;

    /* Returns true iff pt is part of this shape's surface. */
    virtual bool containsPoint(const Vec3d& pt) const = 0;

    const Material& getMaterial() const
    {
      return material_;
    }

    Vec3d center(void) const {return center_;}

    Ray reflect(Ray ray)
    {
      Ray op_ray = ray.op_dir();
      Vec3d normal_dir = this->normal(op_ray);
      // Direction of the reflected vector
      Vec3d refl_dir = ray.dir() - 2.0 * normal_dir * ray.dir().dot(normal_dir);
      //
      // Because of the double imprecision, we have to shift the reflected ray out
      // of the shape. Still, something smells fishy, I think something wrong is
      // done somewhere.
      const double shift = std::numeric_limits<double>::epsilon() * 65536.0;

      return Ray (ray.orig() + normal_dir * shift, normalize(refl_dir));
    }

  protected:
    Shape(Material& mat)
        : material_(mat)
        , lazy_texturing_first_point_(nullptr)
    {}

    Material material_;
    Vec3d center_;
    BBox bbox_;

    /* First point of the shape for which a color has been computed (from a
     * texture). The first point is attributed the origin (0,0) of the
     * attached texture (material_). According to the relevant shape
     * specialization (Sphere, Plane, etc.), color of further points is
     * computed relatively to this first point.
     * If the first point has not been computed, this attribute is nullptr.
     */
    mutable Vec3d* lazy_texturing_first_point_;

    /* This map holds the color values for points that already have been
     * required. Since the texture does not change between successive rays,
     * there is no use of re-compute a point's color value according to the
     * texture of the Material attached to the Shape, so we cache that color.
     */
    mutable std::unordered_map<Vec3d,Color> computed_color_points_;
};

class Sphere : public Shape
{
  public:
    static Sphere* parse(tinyxml2::XMLNode* node);

    Sphere(Vec3d c, Material& mat, double r)
        : Shape(mat), radius_(r)
    {
      center_ = c;
      Vec3d radVec (radius_, radius_, radius_);
      bbox_ = BBox(center_ - radVec, center_ + radVec);
    }

    bool intersect(Ray ray, Vec3d& intersect, double& dist) const
    {
      double a = ray.dir().dot(ray.dir());
      Vec3d o_c = ray.orig() - center_; // O-C
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
        if (t1 >= 0 && t2 >= 0)
          mint = std::min(t1,t2);
        else if (t1 >= 0)
          mint = t1;
        else if (t2 >= 0)
          mint = t2;
        else
          return false;

        // FIXME: for now, only one of the two solutions is returned
        // it is the smallest distance. Not sure if this is a good idea
        intersect = ray.orig() + mint * ray.dir();
        dist = mint;
        return true;
      }
      else
        return false;
    }

    Vec3d normal(Ray& ray)
    {
      return normalize(ray.orig() - center_);
    }

    bool containsPoint(const Vec3d& point) const override;

  private:
    bool computeColorFromTexture(const Vec3d& where, Color& out) const override;

    double radius_;
};

class Plane : public Shape
{
  public:
    static Plane* parse(tinyxml2::XMLNode* node);

    Plane(Vec3d pt1, Vec3d dir1, Vec3d dir2, Material& mat)
    : Shape(mat), pt1_(pt1), dir1_(dir1), dir2_(dir2)
    {
      normal_ = normalize(dir1.cross(dir2));

      Vec3d tdir (1,1,1);
      Vec3d minPt = std::numeric_limits<double>::lowest() * tdir;
      Vec3d maxPt = std::numeric_limits<double>::max() * tdir;

      bbox_ = BBox(minPt, maxPt);
      center_ = pt1;
    }

    ~Plane() { delete lazy_texturing_first_axis_; }

    bool intersect(Ray ray, Vec3d& intersect, double& dist) const
    {
      // We first compute the intersection between the ray and the plane in
      // which the triangle is included
      // FIXME: name
      double k = normal_.dot(pt1_);

      double t = (k - ray.orig().dot(normal_))/(ray.dir().dot(normal_));
      intersect = ray.orig() + t * ray.dir();
      dist = t;
      if (t >= 0)
        return true;
      else
        return false;
    }

    Vec3d normal(Ray& ray)
    {
      return (normal_.dot(ray.dir()) < 0 ? -normal_ : normal_);
    }

    bool containsPoint(const Vec3d& point) const override;

  protected:
    bool computeColorFromTexture(const Vec3d& where, Color& out) const override;

    Vec3d pt1_;
    Vec3d dir1_;
    Vec3d dir2_;
    Vec3d normal_;

    mutable Vec3d* lazy_texturing_first_axis_ = nullptr;
};

class Triangle : public Shape
{
  public:
    static Triangle* parse(tinyxml2::XMLNode* node);

    Triangle(Vec3d pt1, Vec3d pt2, Vec3d pt3, Material& mat)
      : Shape(mat), pt1_(pt1), pt2_(pt2), pt3_(pt3)
      {
        normal_ = normalize((pt3_ - pt2_).cross(pt1_ - pt3_));
        bbox_ = BBox(minVec(minVec(pt1_, pt2_), pt3_),
                  maxVec(maxVec(pt1_, pt2_), pt3_));
        center_ = (1./3.) * (pt1 + pt2 + pt3);

        // The edges of the triangles
        e1_ = (pt2_ - pt1_);
        e2_ = (pt3_ - pt1_);
      }

    ~Triangle() { delete lazy_texturing_first_axis_; }

    bool intersect(Ray ray, Vec3d& intersect, double& dist) const
    {
      Vec3d p = ray.dir().cross(e2_);
      double det = e1_.dot(p);
      double inv_det = 1. / det;

      Vec3d t = ray.orig() - pt1_;

      double u = t.dot(p) * inv_det;
      if (u < 0 || u > 1)
        return false;

      Vec3d q = t.cross(e1_);
      double v = ray.dir().dot(q) * inv_det;
      if (v < 0 || u + v > 1)
        return false;

      dist = e2_.dot(q) * inv_det;
      intersect = ray.orig() + dist * ray.dir();
      if (dist >= 0.)
        return true;
      else
        return false;
    }

    Vec3d normal(Ray& ray)
    {
      return (normal_.dot(ray.dir()) < 0 ? -normal_ : normal_);
    }

    bool containsPoint(const Vec3d& point) const override;
    Vec3d getNormal() {return normal_;}

  protected:
    // A triangle is defined by 3 points in the space
    Vec3d pt1_;
    Vec3d pt2_;
    Vec3d pt3_;

    Vec3d e1_;
    Vec3d e2_;

    // The direction of the normal is the same for the whole triangle, so, we
    // precompute it
    Vec3d normal_;

    mutable Vec3d* lazy_texturing_first_axis_ = nullptr;

    std::pair<double,double> getBarycentric(Ray ray)
    {
      Vec3d p = ray.dir().cross(e2_);
      double det = e1_.dot(p);
      double inv_det = 1. / det;

      Vec3d t = ray.orig() - pt1_;
      double u = t.dot(p) * inv_det;

      Vec3d q = t.cross(e1_);
      double v = ray.dir().dot(q) * inv_det;

      return std::pair<double,double>(u,v);
    }

  private:
    bool computeColorFromTexture(const Vec3d& where, Color& out) const override;
};

#endif
