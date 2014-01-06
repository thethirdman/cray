#ifndef MATERIAL_HXX
# define MATERIAL_HXX

# include <cassert>
# include "material.hh"

/* GETTERS */

Color Material::get_ambient_coef() const
{
    return ambient_coef_;
}

Color Material::get_diffuse_coef() const
{
    return diffuse_coef_;
}

Color Material::get_specular_coef() const
{
    return specular_coef_;
}

float Material::get_brilliancy() const
{
    return brilliancy_;
}

/* SETTERS */

void Material::set_ambient_coef(Color coef)
{
    ambient_coef_ = coef;
}

void Material::set_diffuse_coef(Color coef)
{
    diffuse_coef_ = coef;
}

void Material::set_specular_coef(Color coef)
{
    specular_coef_ = coef;
}

void Material::set_brilliancy(float coef)
{
    assert(coef >= 1);
    brilliancy_ = coef;
}

/* COMPUTE THE COLOR */

Color Material::color_at(int x, int y) const
{
    return func_(x, y);
}

#endif // !MATERIAL_HXX
