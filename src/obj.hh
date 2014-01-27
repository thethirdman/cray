#ifndef OBJ_HH_
# define OBJ_HH_

#include "shape.hh"
#include "normaltriangle.hh"
#include "vector.hh"
#include "kdtree.hh"

class Obj: public Shape
{
  public:
    static Obj* parse(tinyxml2::XMLNode* node);

    Obj(const char* fname,
        Material& mat,
        double scale,
        Vec3d translate,
        double rot[],
        double refl,
        bool interp);

    bool intersect(Ray ray, Vec3d& intersect, double& dist) const
    {
      return (polygons_.intersect(ray, intersect, dist) != 0);
    }

    bool containsPoint(const Vec3d& pt) const;

    bool computeColorFromTexture(const Vec3d& where, Color& out) const override;

    BBox getBBox() { return bbox_; }

    Vec3d normal(Ray& ray)
    {
      Vec3d intersect;
      double dist;
      Shape* shape = polygons_.intersect(ray, intersect, dist);

      if (shape)
        return shape->normal(ray);
      else
      { // FIXME: real handling
        return Vec3d(0,0,0);
      }
    }

  private:
    const char* name_;
    KDTree polygons_;
};

#endif
