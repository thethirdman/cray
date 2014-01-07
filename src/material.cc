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
