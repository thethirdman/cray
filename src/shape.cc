#include "shape.hh"


Shape* Shape::parse(tinyxml2::XMLNode* node)
{
  if (is_named("sphere", node))
    return Sphere::parse(node);
  else if (is_named("plane", node))
    return Plane::parse(node);
  else if (is_named("triangle", node))
    return Triangle::parse(node);
  else
  {
    std::cerr << "Error: unexpected shape of type " << node->ToElement()->Name() << std::endl;
    exit(1);
  }
}

Sphere* Sphere::parse(tinyxml2::XMLNode* node)
{
  double radius = nan("");
  cv::Vec3d pos;
  Color col;

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
    else if (is_named("color", child))
      col = Color::parse(elt);
    else
    {
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }
  }
  while ((child = child->NextSibling()));

  // FIXME: refl value
  return new Sphere(pos, col, radius, 0.3);
}

Plane* Plane::parse(tinyxml2::XMLNode* node)
{
  cv::Vec3d pos;
  cv::Vec3d dir1;
  cv::Vec3d dir2;
  Color col;

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt = child->ToElement();
    if (is_named("vec", child) && elt->Attribute("name","pos"))
      pos = parseVec(elt);
    else if (is_named("color", child))
      col = Color::parse(elt);
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
  return new Plane(pos,dir1,dir2, col, 0);
}

Triangle* Triangle::parse(tinyxml2::XMLNode* node)
{
  cv::Vec3d pt1;
  cv::Vec3d pt2;
  cv::Vec3d pt3;
  Color col;

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt = child->ToElement();
    if (is_named("vec", child))
    {
      cv::Vec3d vec = parseVec(elt);
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
    else if (is_named("color", child))
      col = Color::parse(elt);
    else
    {
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }
  }
  while ((child = child->NextSibling()));

  return new Triangle(pt1,pt2,pt3,col, 0.5);
}
