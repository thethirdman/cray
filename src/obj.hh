#ifndef OBJ_HH_
# define OBJ_HH_

#include "shape.hh"
#include "kdtree.hh"

class Obj: public Shape
{
  public:
    static Obj* parse(tinyxml2::XMLNode* node);
    Obj (const char* fname, Material& mat, double scale, cv::Vec3d translate, double rot[], double refl);
    bool intersect(Ray ray, cv::Vec3d& intersect, double& dist)
    {
      return (polygons_.intersect(ray, intersect, dist) != 0);

    }

    BBox getBBox()
    {
      return bbox_;
    }

    cv::Vec3d normal(Ray& ray)
    {
      cv::Vec3d intersect;
      double dist;
      Shape* shape = polygons_.intersect(ray, intersect, dist);

      if (shape)
        return shape->normal(ray);
      else
      { // FIXME: real handling
        //std::cerr << "Tried to get a normal on a non-intersecting ray" << std::endl;
        return cv::Vec3d(0,0,0);
      }
    }
  private:
    const char* name_;
    KDTree polygons_;
};

#endif
