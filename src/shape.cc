#include <tiny_obj_loader.h>
#include <cassert>

// #include "shape.hh"
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
    std::cerr << "Error: unexpected shape of type " << node->ToElement()->Name() << std::endl;
    exit(1);
  }
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
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }
  }
  while ((child = child->NextSibling()));

  // FIXME: refl value
  return new Plane(pos,dir1,dir2, *mat, 0);
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
        std::cerr << "Error: invalid name for vec " << elt->Attribute("name") << std::endl;
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
