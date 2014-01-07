#include <cassert>
#include "material.hh"

Material::Material(MaterialFunctor      func,
                   const PhongBundle&   pb)
    : func_(func),
      ambient_coef_(pb[0]), diffuse_coef_(pb[1]),
      specular_coef_(pb[2]), brilliancy_(pb[3])
{
}

Material::~Material() {}

const MaterialFunctor& Material::get_functor() const
{
    return func_;
}

PhongBundle Material::get_phongbundle() const
{
    return {{ ambient_coef_, diffuse_coef_, specular_coef_, brilliancy_ }};
}

void Material::set_phongbundle(const PhongBundle& pb)
{
    assert(pb[0] >= 0 && pb[0] <= 1);
    assert(pb[1] >= 0 && pb[1] <= 1);
    assert(pb[2] >= 0 && pb[2] <= 1);
    assert(pb[3] >= 1);

    ambient_coef_  = pb[0];
    diffuse_coef_  = pb[1];
    specular_coef_ = pb[2];
    brilliancy_    = pb[3];
}

Material* Material::parse(tinyxml2::XMLNode* node)
{
  tinyxml2::XMLElement* elt = node->ToElement();

  if (elt->Attribute("type", "simple"))
  {
    float ambient  = nan("");
    float diffuse  = nan("");
    float specular = nan("");
    float brilliancy = nan("");

    elt->QueryFloatAttribute("ambient", &ambient);
    elt->QueryFloatAttribute("diffuse", &diffuse);
    elt->QueryFloatAttribute("specular", &specular);
    elt->QueryFloatAttribute("brilliancy", &brilliancy);

    if (isnan(ambient) || isnan(diffuse) || isnan(specular) || isnan(brilliancy))
    {
      std::cerr << "Error: missing attribute for material" << std::endl;
      exit(1);
    }
    // FIXME: color
    tinyxml2::XMLElement* child_elt = node->FirstChild()->ToElement();
    Color c = Color::parse(child_elt);
    std::function<Color(int,int)> fn = [c](int,int) {return c;};
    return new Material(fn, PhongBundle{{ambient, diffuse, specular, brilliancy}});
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
