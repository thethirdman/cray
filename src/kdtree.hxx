#ifndef KDTREE_HXX_
# define KDTREE_HXX_

void KDTree::buildTree(const std::vector<Shape*>& shapes)
{
    bbox_ = sBuildTree(shapes, 0);
}

bool KDTree::interLeft(const Shape* s, const Vec3d& split, int dim) const
{
    return s->center()[dim] < split[dim];
}

bool KDTree::interRight(const Shape* s, const Vec3d& split, int dim) const
{
    return s->center()[dim] >= split[dim];
}

Shape* KDTree::findSurroundingShape(const Vec3d& pt) const
{
    return recFindSurroundingShape(pt, 0);
}

BBox KDTree::getBBox() const
{
    return bbox_;
}

#endif //!KDTREE_HXX_
