#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <limits>
#include <list>
#include <vector>
#include <utility>
#include "utils.hh"
#include "interval.hh"
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

struct IntervalConstrainedColor
{
    IntervalConstrainedColor(bool xc, bool yc,
                             unsigned xf, unsigned xt,
                             unsigned yf, unsigned yt,
                             Color&& c)
        : xconstrained{xc}, yconstrained{yc}
        , xfrom{xf}, xto{xt}, yfrom{yf}, yto{yt}
        , color{std::move(c)}
    {
    }

    bool            xconstrained;
    bool            yconstrained;
    unsigned int    xfrom;
    unsigned int    xto;
    unsigned int    yfrom;
    unsigned int    yto;
    Color           color;
};

static struct IntervalConstrainedColor
makeIntervalConstrainedColor(const tinyxml2::XMLElement* color_node)
{
    assert(color_node != nullptr);

    bool xconstrained = false;
    bool yconstrained = false;
    unsigned xfrom{0}, xto{0}, yfrom{0}, yto{0};

    if (const char* x_constraint = color_node->Attribute("when_x"))
    {
        xconstrained = true;
        size_t i = 0;
        size_t len = std::strlen(x_constraint);
        while (i < len && x_constraint[i] >= '0' && x_constraint[i] <= '9')
        {
            ++i;
        }
        PARSE_ERROR_IF(i==0, "all where_x clauses must start with a number");
        PARSE_ERROR_IF(i==len, "missing final bound (\"..B\") in where_x clause");
        PARSE_ERROR_IF(x_constraint[i] != '.' || i+1 >= len || x_constraint[i+1] != '.',
                       "where_x clauses must follow \"A..B\", A and B being integers");
        PARSE_ERROR_IF(i+2 >= len || x_constraint[i+2] < '0' || x_constraint[i+2] > '9',
                       "missing final bound (\"..B\") in where_x clause");
        xfrom = std::atoi(x_constraint);
        xto = std::atoi(x_constraint + i + 2);
        PARSE_ERROR_IF(xfrom >= xto,
                       "can't have A >= B in a \"A..B\" where_x clause");
    }

    if (const char* y_constraint = color_node->Attribute("when_y"))
    {
        yconstrained = true;
        size_t i = 0;
        size_t len = std::strlen(y_constraint);
        while (i < len && y_constraint[i] >= '0' && y_constraint[i] <= '9')
        {
            ++i;
        }
        PARSE_ERROR_IF(i==0, "all where_y clauses must start with a number");
        PARSE_ERROR_IF(i==len, "missing final bound (\"..B\") in where_y clause");
        PARSE_ERROR_IF(y_constraint[i] != '.' || i+1 >= len || y_constraint[i+1] != '.',
                       "where_y clauses must follow \"A..B\", A and B being integers");
        PARSE_ERROR_IF(i+2 >= len || y_constraint[i+2] < '0' || y_constraint[i+2] > '9',
                       "missing final bound (\"..B\") in where_y clause");
        yfrom = std::atoi(y_constraint);
        yto = std::atoi(y_constraint + i + 2);
        PARSE_ERROR_IF(yfrom >= yto,
                       "can't have A >= B in a \"A..B\" where_y clause");
    }

    return {
        xconstrained, yconstrained,
        xfrom, xto, yfrom, yto,
        Color::parse(color_node)
    };
}

using ICC = struct IntervalConstrainedColor;

static std::function<Color(int,int)>
_makeProceduralFunctor_repeat(const std::vector<ICC>& constraints,
                              bool base_exists, Color base)
{
    const DefinedInterval<unsigned>* x_dinterval;
    const DefinedInterval<unsigned>* y_dinterval;

    {
        Interval<unsigned>* x_interval{new SolidInterval<unsigned>{0, 0}};
        Interval<unsigned>* y_interval{new SolidInterval<unsigned>{0, 0}};
        Interval<unsigned>* tmp{nullptr};

        for (auto c: constraints)
        {
            if (c.xconstrained)
            {
                tmp = x_interval->aggregate(
                    Interval<unsigned>::create(c.xfrom, c.xto)
                );
                delete x_interval;
                x_interval = tmp;
            }
            if (c.yconstrained)
            {
                tmp = y_interval->aggregate(
                    Interval<unsigned>::create(c.yfrom, c.yto)
                );
                delete y_interval;
                y_interval = tmp;
            }
        }

        assert(x_interval->toDefinedInterval());
        assert(y_interval->toDefinedInterval());

        x_dinterval = x_interval->toDefinedInterval();
        y_dinterval = y_interval->toDefinedInterval();
    }

    PARSE_ERROR_IF(x_dinterval->getMin() != 0,
                   "no base color defined for interval [0;" << x_dinterval->getMin()
                   << "[ in where_x clauses");
    PARSE_ERROR_IF(y_dinterval->getMin() != 0,
                   "no base color defined for interval [0;" << y_dinterval->getMin()
                   << "[ in where_y clauses");

    unsigned xmax{x_dinterval->getMax()}, ymax{y_dinterval->getMax()};

    if (base_exists)
    {
        delete x_dinterval;
        delete y_dinterval;

        return [xmax,ymax,base,constraints](int xx, int yy) -> Color
        {
            if (xmax > 0) xx %= xmax;
            if (ymax > 0) yy %= ymax;
            unsigned x = (xx < 0 ? xmax - xx : xx);
            unsigned y = (yy < 0 ? ymax - yy : yy);
            for (auto c: constraints) {
                if (!c.xconstrained || (x >= c.xfrom && x <= c.xto)) {
                    if (!c.yconstrained || (y >= c.yfrom && y <= c.yto)) {
                        return c.color;
                    }
                }
            }
            return base;
        };
    }
    else
    {
        PARSE_ERROR_IF(!x_dinterval->continuous(),
                       "missing a default color with discontinuous "
                       "where_x clause");
        PARSE_ERROR_IF(!y_dinterval->continuous(),
                       "missing a default color with discontinuous "
                       "where_y clause");

        delete x_dinterval;
        delete y_dinterval;

        return [xmax,ymax,constraints](int xx, int yy) -> Color
        {
            if (xmax > 0) xx %= xmax;
            if (ymax > 0) yy %= ymax;
            unsigned x = (xx < 0 ? xmax - xx : xx);
            unsigned y = (yy < 0 ? ymax - yy : yy);
            for (auto c: constraints) {
                if (!c.xconstrained || (x >= c.xfrom && x <= c.xto)) {
                    if (!c.yconstrained || (y >= c.yfrom && y <= c.yto)) {
                        return c.color;
                    }
                }
            }
            assert(0 && "never reached");
        };
    }
}

static std::function<Color(int,int)>
_makeProceduralFunctor_noRepeat(std::vector<ICC>& constraints, Color base)
{
    if (constraints.size() == 0)
    {
        return [base](int,int) { return base; };
    }
    else
    {
        return [base,constraints](int x, int y) -> Color
        {
            for (auto c: constraints) {
                if (c.xconstrained) {
                    if (static_cast<unsigned>(x) >= c.xfrom
                        && static_cast<unsigned>(x) <= c.xto)
                    {
                        if (c.yconstrained
                            && (static_cast<unsigned>(y) < c.yfrom
                                || static_cast<unsigned>(y) > c.yto))
                            continue;
                        return c.color;
                    }
                    else continue;
                }
                else if (c.yconstrained
                         && static_cast<unsigned>(y) >= c.yfrom
                         && static_cast<unsigned>(y) <= c.yto) {
                    return c.color;
                }
            }
            return base;
        };
    }
}

// warning: removes elements from the given vector
static std::function<Color(int,int)>
makeProceduralFunctor(std::vector<ICC>& constraints, bool repeat)
{
    assert(constraints.size() > 0);

    auto ibase = std::find_if(
        constraints.cbegin(),
        constraints.cend(),
        [](const ICC& c){ return !c.xconstrained && !c.yconstrained; }
    );

    bool base_exists = ibase != constraints.cend();
    Color base = base_exists ? ibase->color : Color(0,0,0);

    std::remove_if(constraints.begin(), constraints.end(),
                   [](const ICC& c){ return !c.xconstrained && !c.yconstrained; });

    PARSE_ERROR_IF(!repeat && base_exists,
                   "can't create a non-repetitive procedural texture without "
                   "a base color (without where_x and where_y clauses)");

    return repeat
        ? _makeProceduralFunctor_repeat(constraints, base_exists, base)
        : _makeProceduralFunctor_noRepeat(constraints, base);
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

    PARSE_ERROR_IF(isnan(ambient) || isnan(diffuse) || isnan(specular) || isnan(brilliancy),
                   "missing attribute for material");
    // FIXME: color
    tinyxml2::XMLElement* child_elt = node->FirstChild()->ToElement();
    Color c = Color::parse(child_elt);
    std::function<Color(int,int)> fn = [c](int,int) {return c;};
    return new Material(fn, PhongBundle{{ambient, diffuse, specular, brilliancy}});
  }
  else if (elt->Attribute("type", "procedural"))
  {
      float ambient  = nan("");
      float diffuse  = nan("");
      float specular = nan("");
      float brilliancy = nan("");

      elt->QueryFloatAttribute("ambient", &ambient);
      elt->QueryFloatAttribute("diffuse", &diffuse);
      elt->QueryFloatAttribute("specular", &specular);
      elt->QueryFloatAttribute("brilliancy", &brilliancy);

      PARSE_ERROR_IF(isnan(ambient) || isnan(diffuse)
                     || isnan(specular) || isnan(brilliancy),
                     "missing attribute for material");

      std::vector<ICC> constraints;
      auto n = node->FirstChildElement("color");
      while (n != nullptr)
      {
          constraints.emplace_back(makeIntervalConstrainedColor(n));
          n = n->NextSiblingElement("color");
      }

      bool repeat = elt->Attribute("repeat")
                 && std::strcmp(elt->Attribute("repeat"), "0") != 0;

      return new Material{
          makeProceduralFunctor(constraints, repeat),
          PhongBundle{{ambient, diffuse, specular, brilliancy}}
      };
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
