#ifndef MATERIAL_HH
# define MATERIAL_HH

# include <functional>
# include <array>
# include "color.hh"
# include "vector.hh"

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
using PhongBundle = std::array<float, 4>;

class Material
{
public:
    Material(MaterialFunctor, const PhongBundle&);

    static Material* parse(tinyxml2::XMLNode* node);

    virtual ~Material();

    inline float get_ambient_coef() const;
    inline float get_diffuse_coef() const;
    inline float get_specular_coef() const;
    inline float get_brilliancy() const;
    PhongBundle  get_phongbundle() const;

    inline void set_ambient_coef(float);
    inline void set_diffuse_coef(float);
    inline void set_specular_coef(float);
    inline void set_brilliancy(float);
    void        set_phongbundle(const PhongBundle&);

    const MaterialFunctor& get_functor() const;

    inline Color color_at(int x, int y) const;

protected:
    const MaterialFunctor func_;

    /// Illumination coefficients
    float ambient_coef_;
    float diffuse_coef_;
    float specular_coef_;
    float brilliancy_;
};

# include "material.hxx"

#endif // !MATERIAL_HH
