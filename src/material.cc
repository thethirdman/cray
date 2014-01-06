#include <cassert>
#include <tinyxml2.h>
#include "material.hh"
#include "utils.hh"

Material::Material(MaterialFunctor func, Color& ambient, Color& diffuse, Color& specular, float brilliancy)
  : func_(func), ambient_coef_(ambient), diffuse_coef_(diffuse),
    specular_coef_(specular), brilliancy_(brilliancy)
{
}

Material::~Material() {}

const MaterialFunctor& Material::get_functor() const
{
    return func_;
}

Material* Material::parse(tinyxml2::XMLNode* node)
{
  tinyxml2::XMLElement* elt = node->ToElement();

  if (elt->Attribute("type", "simple"))
  {
    Color ambient;
    Color diffuse;
    Color specular;
    float brilliancy = nan("");

    tinyxml2::XMLNode* child = node->FirstChild();
    do
    {
      tinyxml2::XMLElement* celt = child->ToElement();
      if (is_named("color", child))
      {
        Color tmp = Color::parse(celt);
        if (celt->Attribute("name", "ambient"))
          ambient = tmp;
        else if (celt->Attribute("name", "diffuse"))
          diffuse = tmp;
        else if (celt->Attribute("name", "specular"))
          specular = tmp;
        else
        {
          std::cerr << "Error: invalid name " << celt->Attribute("name") << std::endl;
          exit(1);
        }

      }
      else if (is_named("brilliancy", child))
      {
        celt->QueryFloatAttribute("value", &brilliancy);
      }
      else
      {
        std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
        exit(1);
      }
    }
    while ((child = child->NextSibling()));

    return new Material(nullptr, ambient, diffuse, specular, brilliancy);
  }
  else if (elt->Attribute("type", "procedural"))
  {
    std::cerr << "Procedural texture not yet implemented" << std::endl;
    exit(1);
  }
  else if (elt->Attribute("type", "bitmap"))
  {
    std::cerr << "Bitmap texture not yet implemented" << std::endl;
    exit(1);
  }
  else
  {
    std::cerr << "Unrecognized Material of type: " << elt->Attribute("type") << std::endl;
    exit(1);
  }
}
