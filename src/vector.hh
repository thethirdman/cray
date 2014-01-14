#ifndef VECTOR_HH_
# define VECTOR_HH_

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

    type norm()
    {
      type ret = 0;

      for (unsigned int i = 0; i < dim; i++)
        ret += coord_[i] * coord_[i];

      return sqrt(ret);
    }
    type dot(Vector<type,dim> a);
    // FIXME: ugly
    Vector<type,dim>   cross( Vector<type, dim> a);

  private:
    type coord_[dim];
};


template <typename type, std::size_t dim>
Vector<type,dim> operator+( Vector<type,dim> a,  Vector<type,dim> b);

template <typename type, std::size_t dim>
Vector<type,dim> operator-( Vector<type,dim> a,  Vector<type,dim> b);

template <typename type, std::size_t dim>
Vector<type,dim> operator*( Vector<type,dim> a, type b);

template <typename type, std::size_t dim>
Vector<type,dim> operator*(type a,  Vector<type,dim> b);

template <typename type, std::size_t dim>
Vector<type,dim> operator-( Vector<type,dim> a);

typedef Vector<double,3> Vec3d;

#include "vector.hxx"

#endif
