#ifndef KDTREE_HH_
# define KDTREE_HH_

#include <cv.h>
#include "shape.hh"
#include "bbox.hh"
#include "vector.hh"
#include <assert.h>

class KDTree
{
  public:
    inline void buildTree(const std::vector<Shape*>& shapes);

    BBox sBuildTree(const std::vector<Shape*>& shapes, int depth);

    double findBestSplit(const std::vector<Shape*> shapes, unsigned int& shape_i, int dim);

    inline bool interLeft(const Shape* s, const Vec3d& split, int dim) const;

    inline bool interRight(const Shape* s, const Vec3d& split, int dim) const;

    Shape* intersect(const Ray& r, Vec3d& intersect, double& dist) const;

    inline Shape* findSurroundingShape(const Vec3d& pt) const;

    Shape* recFindSurroundingShape(const Vec3d& pt, char dim) const;

    inline BBox getBBox() const;

  private:
    Vec3d splitpos_;
    BBox bbox_;
    Shape* shape_;
    KDTree* left_;
    KDTree* right_;
};

# include "kdtree.hxx"

#endif // KDTREE
