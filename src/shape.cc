#include <tiny_obj_loader.h>
#include <cassert>

#include "shape.hh"
#include "obj.hh"

Shape* Shape::parse(tinyxml2::XMLNode* node)
{
  if (is_named("sphere", node))
    return Sphere::parse(node);
  else if (is_named("plane", node))
    return Plane::parse(node);
  else if (is_named("triangle", node))
    return Triangle::parse(node);
  else if (is_named("obj", node))
    return Obj::parse(node);
  else
  {
    std::cerr << "Error: unexpected shape of type "
        << node->ToElement()->Name() << std::endl;
    exit(1);
  }
}

Color Shape::getColorAt(const Vec3d& surface_point) const
{
    const auto it = computed_color_points_.find(surface_point);
    Color c;
    if (it == computed_color_points_.cend()) {

        // avoid "unused variable 'valid' when compiled with NDEBUG
# ifdef NDEBUG
        computeColorFromTexture(surface_point, c);
# else
        bool valid = computeColorFromTexture(surface_point, c);
        assert(valid);
# endif //NDEBUG

        computed_color_points_[surface_point] = c; // cache
    }
    else {
        c = it->second;
    }
    return c;
}

Sphere* Sphere::parse(tinyxml2::XMLNode* node)
{
  double radius = nan("");
  Vec3d pos;
  Material* mat = nullptr;

  node->ToElement()->QueryDoubleAttribute("r", &radius);
  if (isnan(radius))
  {
    std::cerr << "Error: missing radius for sphere" << std::endl;
    exit(1);
  }

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt = child->ToElement();
    if (is_named("vec", child) && elt->Attribute("name","pos"))
      pos = parseVec(elt);
    else if (is_named("material", child))
      mat = Material::parse(elt);
    else
    {
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }
  }
  while ((child = child->NextSibling()));

  // FIXME: refl value
  return new Sphere(pos, *mat, radius, 0.1);
}

bool Sphere::containsPoint(const Vec3d& point) const
{
    return fequals(radius_, (point - center_).norm());
}

bool Sphere::computeColorFromTexture(const Vec3d& where, Color& out) const
{
    assert(this->containsPoint(where));

    if (lazy_texturing_first_point_ == nullptr)
    {
        lazy_texturing_first_point_ = new Vec3d{where};
        out = material_.color_at(0, 0);
    }
    else
    {
        Vec3d center2origin = *lazy_texturing_first_point_ - center_;
        Vec3d center2where  = where - center_;
        Vec3d origin2where  = where - *lazy_texturing_first_point_;
        double angle = center2origin.dot(center2where)
            / (center2origin.norm() * center2where.norm());
        origin2where = 1.0 / origin2where.norm() * origin2where; // normalized
        out = material_.color_at(angle * radius_ * origin2where[0],
                                 angle * radius_ * origin2where[1]);
    }
    return true;
}

Plane* Plane::parse(tinyxml2::XMLNode* node)
{
  Vec3d pos;
  Vec3d dir1;
  Vec3d dir2;
  Material* mat = nullptr;

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt = child->ToElement();
    if (is_named("vec", child) && elt->Attribute("name","pos"))
      pos = parseVec(elt);
    else if (is_named("material", child))
      mat = Material::parse(elt);
    else if (is_named("vec", child) && elt->Attribute("name", "dir1"))
      dir1 = parseVec(elt);
    else if (is_named("vec", child) && elt->Attribute("name", "dir2"))
      dir2 = parseVec(elt);
    else
    {
      std::cerr << "Error: invalid node "
          << child->ToElement()->Name() << std::endl;
      exit(1);
    }
  }
  while ((child = child->NextSibling()));

  // FIXME: refl value
  return new Plane(pos,dir1,dir2, *mat, 0);
}

bool Plane::computeColorFromTexture(const Vec3d& where, Color& out) const
{
    assert(this->containsPoint(where));

    if (lazy_texturing_first_point_ == nullptr) // first time called
    {
        lazy_texturing_first_point_ = new Vec3d{where};
        out = material_.color_at(0, 0);
    }
    else
    {
        Vec3d diff = where - *lazy_texturing_first_point_;
        out = material_.color_at(diff[0], diff[1]);
    }
    return true;
}

bool Plane::containsPoint(const Vec3d& point) const
{
    return fequals(0, normal_.dot(point - pt1_));
}

Triangle* Triangle::parse(tinyxml2::XMLNode* node)
{
  Vec3d pt1;
  Vec3d pt2;
  Vec3d pt3;
  Material* mat = nullptr;

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt = child->ToElement();
    if (is_named("vec", child))
    {
      Vec3d vec = parseVec(elt);
      if (elt->Attribute("name","pt1"))
        pt1 = vec;
      else if (elt->Attribute("name","pt2"))
        pt2 = vec;
      else if (elt->Attribute("name", "pt3"))
        pt3 = vec;
      else
      {
        std::cerr << "Error: invalid name for vec "
            << elt->Attribute("name") << std::endl;
      }
    }
    else if (is_named("material", child))
      mat = Material::parse(elt);
    else
    {
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }
  }
  while ((child = child->NextSibling()));

  return new Triangle(pt1,pt2,pt3, *mat, 0.5);
}

bool Triangle::containsPoint(const Vec3d& point) const
{
    Vec3d v2{point  - pt1_};

    if (!fequals(normal_.dot(v2), 0)) return false; // not in the same plane

    Vec3d v0{pt3_   - pt1_};
    Vec3d v1{pt2_   - pt1_};

    double dot00{v0.dot(v0)};
    double dot01{v0.dot(v1)};
    double dot02{v0.dot(v2)};
    double dot11{v1.dot(v1)};
    double dot12{v1.dot(v2)};

    double invDenom{1 / (dot00 * dot11 - dot01 * dot01)};
    double u{(dot11 * dot02 - dot01 * dot12) * invDenom};
    double v{(dot00 * dot12 - dot01 * dot02) * invDenom};

    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

bool Triangle::computeColorFromTexture(const Vec3d& where, Color& out) const
{
    assert(this->containsPoint(where));

    if (lazy_texturing_first_point_ == nullptr) // first time called
    {
        lazy_texturing_first_point_ = new Vec3d{where};
        out = material_.color_at(0, 0);
    }
    else
    {
        Vec3d diff = where - *lazy_texturing_first_point_;
        out = material_.color_at(diff[0], diff[1]);
    }
    return true;
}
