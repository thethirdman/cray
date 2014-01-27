#ifndef VECTOR_HH_
# define VECTOR_HH_

#include <functional>
#include <cmath>

template <typename type, std::size_t dim>
class Vector
{
  public:
    Vector(type coord[]) : coord_(coord) {}
    Vector() {}

    Vector(type a, type b, type c)
    {
      coord_[0] = a;
      coord_[1] = b;
      coord_[2] = c;
    }

    type& operator[] (unsigned int index)
    {
      return coord_[index];
    }

    const type& operator[] (unsigned int index) const
    {
      return coord_[index];
    }

    inline type norm() const;

    inline type dot(const Vector<type,dim>& a) const;

    // FIXME: ugly
    Vector<type,dim>   cross( Vector<type, dim> a);

  private:
    type coord_[dim];
};


template <typename type, std::size_t dim>
inline Vector<type,dim> operator+(Vector<type,dim> a, Vector<type,dim> b);

template <typename type, std::size_t dim>
inline Vector<type,dim> operator-(Vector<type,dim> a, Vector<type,dim> b);

template <typename type, std::size_t dim>
inline Vector<type,dim> operator*(Vector<type,dim> a, type b);

template <typename type, std::size_t dim>
inline Vector<type,dim> operator*(type a, Vector<type,dim> b);

template <typename type, std::size_t dim>
inline Vector<type,dim> operator-(Vector<type,dim> a);

template <typename type, std::size_t dim>
inline bool operator==(const Vector<type,dim>& a, const Vector<type,dim>& b);

typedef Vector<double,3> Vec3d;

namespace std
{

    template<>
    struct hash<Vec3d>
    {
        size_t operator()(const ::Vec3d& v) const noexcept
        {
            hash<double> h;
            return h(v[0]) ^ h(v[1]) ^ h(v[2]);
        }
    };

} // namespace std

#include "vector.hxx"

#endif
