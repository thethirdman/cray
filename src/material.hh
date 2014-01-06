#ifndef MATERIAL_HH
# define MATERIAL_HH

# include <functional>
# include <array>
# include "color.hh"

/**
 * Type for the functors that return a color according to a coordinate of the
 * Material. Abscissas (x) first, ordinates (y) second. Both begin from 0. It
 * is up to the functor to return something consistent when x or y tend to
 * +Inf.
 */
using MaterialFunctor = std::function<Color(int, int)>;

/**
 * Compact representation of the Phong model’s components:
 *  0. ambiant lightning coefficient (between 0 and 1, inclusive);
 *  1. diffuse lightning coefficient (between 0 and 1, inclusive);
 *  2. specular lightning coefficient (between 0 and 1, inclusive);
 *  3. brilliancy (more than 1, inclusive).
 */
class Material
{
public:
    Material(MaterialFunctor, Color& ambient, Color& diffuse, Color& specular, float brilliancy);

    Material() { };

    static Material* parse(tinyxml2::XMLNode* node);

    virtual ~Material();

    inline Color get_ambient_coef() const;
    inline Color get_diffuse_coef() const;
    inline Color get_specular_coef() const;
    inline float get_brilliancy() const;

    inline void set_ambient_coef(Color);
    inline void set_diffuse_coef(Color);
    inline void set_specular_coef(Color);
    inline void set_brilliancy(float);

    const MaterialFunctor& get_functor() const;

    inline Color color_at(int x, int y) const;

protected:
    const MaterialFunctor func_;

    /// Illumination coefficients
    Color ambient_coef_;
    Color diffuse_coef_;
    Color specular_coef_;
    float brilliancy_;
};

# include "material.hxx"

#endif // !MATERIAL_HH
