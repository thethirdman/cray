#include "color.hh"
#include "utils.hh"

Color Color::parse(const tinyxml2::XMLElement* elt)
{
  double r = nan("");
  double g = nan("");
  double b = nan("");

  elt->QueryDoubleAttribute("r", &r);
  elt->QueryDoubleAttribute("g", &g);
  elt->QueryDoubleAttribute("b", &b);

  PARSE_ERROR_IF(isnan(r) || isnan(g) || isnan(b),
                 "missing attribute in color, either r, g or b");

  return Color(r,g,b);
}

Color operator+ (Color a, Color other)
{
  double new_r = a.r()+ other.r();
  double new_g = a.g()+ other.g();
  double new_b = a.b()+ other.b();
  double new_max = a.max() + other.max();

  return Color(new_r, new_g, new_b, new_max);
}

Color operator+(Color rgb, double val)
{
  if (val < 0)
    return rgb - (-val);

  double new_r = rgb.r()+val;
  double new_g = rgb.g()+val;
  double new_b = rgb.b()+val;

  return Color( (new_r > 1. ? 1. : new_r)
              , (new_g > 1. ? 1. : new_g)
              , (new_b > 1. ? 1. : new_b)
              , rgb.max());
}

Color satSum(Color a, Color other)
{
  double new_r = a.r()+ other.r();
  double new_g = a.g()+ other.g();
  double new_b = a.b()+ other.b();
  return Color( (new_r > 1. ? 1. : new_r)
              , (new_g > 1. ? 1. : new_g)
              , (new_b > 1. ? 1. : new_b)
              , 1.);
}

Color operator-(Color rgb, double val)
{
  if (val < 0)
    return rgb + (-val);
  double new_r = rgb.r()-val;
  double new_g = rgb.g()-val;
  double new_b = rgb.b()-val;
  return Color((new_r < 0. ? 0. : new_r)
              ,(new_g < 0. ? 0. : new_g)
              ,(new_b < 0. ? 0. : new_b)
              , rgb.max()); // FIXME: put this max ?
}

Color operator* (Color a, Color other)
{
  double new_r = a.r()* other.r();
  double new_g = a.g()* other.g();
  double new_b = a.b()* other.b();
  double new_max = a.max() * other.max();
  return Color(new_r, new_g, new_b, new_max);
}

Color operator* (Color rgb, double val)
{
  return Color(rgb.r() * val, rgb.g() * val, rgb.b() * val, rgb.max());
}

Color ponderate(Color rgb, double val)
{
  return Color(rgb.r() * val, rgb.g() * val, rgb.b() * val, val * rgb.max());
}

Color operator*(double val, Color rgb)
{
  return rgb * val;
}

Color operator/ (Color rgb, double c)
{
  return Color(rgb.r() / c, rgb.g()/c, rgb.b()/c, rgb.max()/c);
}
