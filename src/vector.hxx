#ifndef VECTOR_HXX_
# define VECTOR_HXX_

template <typename type, std::size_t dim>
type Vector<type, dim>::norm() const
{
    type ret = 0;

    for (unsigned int i = 0; i < dim; i++)
        ret += coord_[i] * coord_[i];

    return sqrt(ret);
}

template <typename type, std::size_t dim>
type Vector<type,dim>::dot(const Vector<type,dim>& a) const
{

  type ret = 0;
  for (unsigned int i = 0; i < dim; i++)
    ret += coord_[i] * a[i];

  return ret;
}

// FIXME: ugly
template <typename type, std::size_t dim>
Vector<type,dim> Vector<type,dim>::cross(Vector<type, dim> a)
{
  Vector<type,dim> ret;

  ret[0] = coord_[1]*a[2] - coord_[2]*a[1];
  ret[1] = coord_[2]*a[0] - coord_[0]*a[2];
  ret[2] = coord_[0]*a[1] - coord_[1]*a[0];

  return ret;
}
template <typename type, std::size_t dim>
Vector<type,dim> operator+( Vector<type,dim> a,  Vector<type,dim> b)
{
  Vector<type,dim> ret;

  for (unsigned int i = 0; i < dim; i++)
    ret[i] = a[i] + b[i];
  return ret;
}

template <typename type, std::size_t dim>
Vector<type,dim> operator-( Vector<type,dim> a,  Vector<type,dim> b)
{
  Vector<type,dim> ret;

  for (unsigned int i = 0; i < dim; i++)
    ret[i] = a[i] - b[i];
  return ret;
}

template <typename type, std::size_t dim>
Vector<type,dim> operator*(Vector<type,dim> a, type b)
{
  Vector<type,dim> ret;

  for (unsigned int i = 0; i < dim; i++)
  {
    ret[i] = a[i] * b;
  }

  return ret;
}

template <typename type, std::size_t dim>
Vector<type,dim> operator*(type a, Vector<type,dim> b)
{
  return b * a;
}

template <typename type, std::size_t dim>
Vector<type,dim> operator-(Vector<type,dim> a)
{
  Vector<type,dim> ret;
  for (unsigned int i = 0; i < dim; i++)
    ret[i] = 0 - a[i];
  return ret;
}

template <typename type, std::size_t dim>
bool operator==(const Vector<type,dim>& a, const Vector<type,dim>& b)
{
    for (std::size_t i = 0; i < dim; ++i)
    {
        if (!(a[i] == b[i])) return false;
    }
    return true;
}

#endif
