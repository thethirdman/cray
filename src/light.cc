#include "light.hh"
Light Light::parse(tinyxml2::XMLNode* node)
{
  cv::Vec3d pos;
  Color col;

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


  return Light(pos, col);
}
