#ifndef NORMALTRIANGLE_HH_
# define NORMALTRIANGLE_HH_

#include "shape.hh"
#include "vector.hh"
#include <list>
#include <unordered_map>

// A triangle class that has normals associated to points.
// Used for normal interpolation on Obj
class NormalTriangle : public Triangle
{
  public:
    NormalTriangle (Vec3d pt1, Vec3d pt2, Vec3d pt3,
        Material& mat)
      : Triangle(pt1,pt2,pt3,mat)
    {}

    Vec3d normal(Ray& ray)
    {
      Vec3d inter;
      double inter_dist;

      if (!intersect(ray, inter, inter_dist))
      {
        std::cerr << "Fail normal" << std::endl;
        exit(1);
      }

      std::pair<double,double> uv = getBarycentric(ray);
      double u = uv.first;
      double v = uv.second;
      double w = 1 - u -v;

      Vec3d result = no1_ * w + no2_ * u + no3_ * v;
      return (result.dot(ray.dir()) < 0 ? -result : result);
    }

    void updateNormals(std::map<Vec3d, std::list<Triangle*>>& ptMap);

    void setNormal(int index, Vec3d vec)
    {
      if (index == 1) no1_ = vec;
      else if (index == 2) no2_ = vec;
      else no3_ = vec;
    }

  private:
    Vec3d updateVertex(std::list<Triangle*>& triangles)
    {
      Vec3d pt = Vec3d(0,0,0);
      int total = 0;

      for (auto it = triangles.begin(); it != triangles.end(); it++)
      {
        Vec3d curNormal = (*it)->getNormal();
        if (curNormal.dot(pt) < 0)
          pt = pt - curNormal;
        else
          pt = pt + curNormal;
        total++;
      }
      pt = (1./((double) total)) * pt;

      // Maybe normalize is overkill ...
      return normalize(pt);
    }

    Vec3d no1_;
    Vec3d no2_;
    Vec3d no3_;

};

#endif
