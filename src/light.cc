#include "light.hh"
Light Light::parse(tinyxml2::XMLNode* node)
{
  Vec3d pos;
  Color col;
  float radius = nan("");
  int samples = 0;

  tinyxml2::XMLElement* node_elt = node->ToElement();
  node_elt->QueryFloatAttribute("r", &radius);
  node_elt->QueryIntAttribute("samples", &samples);
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


  if (isnan(radius))
    return Light(pos, col);
  else
    return Light(pos, radius, samples, col);
}
