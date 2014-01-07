#ifndef MATERIAL_HXX
# define MATERIAL_HXX

# include <cassert>
# include "material.hh"

/* GETTERS */

float Material::get_ambient_coef() const
{
    return ambient_coef_;
}

float Material::get_diffuse_coef() const
{
    return diffuse_coef_;
}

float Material::get_specular_coef() const
{
    return specular_coef_;
}

float Material::get_brilliancy() const
{
    return brilliancy_;
}

/* SETTERS */

void Material::set_ambient_coef(float coef)
{
    assert(coef >= 0 && coef <= 1);
    ambient_coef_ = coef;
}

void Material::set_diffuse_coef(float coef)
{
    assert(coef >= 0 && coef <= 1);
    diffuse_coef_ = coef;
}

void Material::set_specular_coef(float coef)
{
    assert(coef >= 0 && coef <= 1);
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
