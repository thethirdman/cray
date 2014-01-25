#include "kdtree.hh"

BBox KDTree::sBuildTree(const std::vector<Shape*>& shapes, int depth)
{
    left_ = nullptr;
    right_ = nullptr;
    if (shapes.size() == 1)
    {
        shape_ = shapes[0];
        bbox_ = shape_->getBBox();

        return bbox_;
    }

    int dim = depth % 3;

    unsigned int shape_i = -1;
    splitpos_ = Vec3d (0.,0.,0.);
    splitpos_[dim] = findBestSplit(shapes, shape_i, dim);

    std::vector<Shape*> lshapes;

    std::vector<Shape*> rshapes;

    shape_ = shapes[shape_i];
    bbox_ = shape_->getBBox();

    for (unsigned int i = 0; i < shapes.size(); i++)
    {
        if (i == shape_i) continue;

        if (interLeft(shapes[i], splitpos_, dim))
            lshapes.push_back(shapes[i]);
        if (interRight(shapes[i], splitpos_, dim))
            rshapes.push_back(shapes[i]);
    }

    if (lshapes.size() != 0)
    {
        left_ = new KDTree ();
        bbox_.merge(left_->sBuildTree(lshapes, depth + 1));
    }

    if (rshapes.size() != 0)
    {
        right_ = new KDTree();
        bbox_.merge(right_->sBuildTree(rshapes, depth + 1));
    }

    return bbox_;
}

double KDTree::findBestSplit(const std::vector<Shape*> shapes,
                             unsigned int& shape_i,
                             int dim)
{
    std::vector<double> centers(shapes.size());

    for (unsigned int i = 0; i < shapes.size(); i++)
        centers[i] = shapes[i]->center()[dim];
    std::sort(centers.begin(), centers.end());

    shape_i = centers.size()/2;
    return centers[shape_i];
}

Shape* KDTree::intersect(const Ray& r, Vec3d& intersect, double& dist) const
{
    if (bbox_.mustShoot(r))
    {
        double best_dist;
        Vec3d best_inter;
        bool hit = false;
        Shape* ret = nullptr;

        if (shape_)
        {
            hit = shape_->intersect(r, best_inter, best_dist);
            if (hit)
                ret = shape_;
        }

        Vec3d cur_inter;
        double cur_dist = -1;

        if (left_)
        {
            Shape* sit = left_->intersect(r, cur_inter, cur_dist);
            if (sit && (!hit || cur_dist < best_dist))
            {
                best_dist = cur_dist;
                best_inter = cur_inter;
                ret = sit;
                hit = true;
            }
        }

        if (right_)
        {
            Shape* sit = right_->intersect(r, cur_inter, cur_dist);
            if (sit && (!hit || cur_dist < best_dist))
            {
                best_dist = cur_dist;
                best_inter = cur_inter;
                ret = sit;
                hit = true;
            }
        }

        intersect = best_inter;
        dist = best_dist;
        return ret;
    }
    else
    {
        return nullptr;
    }
}

Shape* KDTree::recFindSurroundingShape(const Vec3d& pt, char dim) const
{
    if (!bbox_.containsPoint(pt))
    {
        return nullptr;
    }

    assert(shape_ != nullptr);

    if (shape_->containsPoint(pt))
    {
        return shape_;
    }

    dim %= 3;

    if (left_ != nullptr && pt[dim] < this->splitpos_[dim])
    {
        return left_->recFindSurroundingShape(pt, dim + 1);
    }

    if (right_ != nullptr && pt[dim] > this->splitpos_[dim])
    {
        return right_->recFindSurroundingShape(pt, dim + 1);
    }

    return nullptr;
}
