#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <highgui.h>
#include <limits>
#include <list>
#include <vector>
#include <utility>
#include "utils.hh"
#include "interval.hh"
#include "material.hh"
#include "bitmap_texture.hh"

Material::Material(MaterialFunctor      func,
                   const PhongBundle&   pb,
                   float refl)
    : func_(func),
      ambient_coef_(pb[0]), diffuse_coef_(pb[1]),
      specular_coef_(pb[2]), brilliancy_(pb[3]),
      refl_coef_(refl)
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
_makeProceduralFunctor_repeat_noBase(const std::vector<ICC>& constraints)
{
    Interval<unsigned>* x_interval{new EmptyInterval<unsigned>};
    Interval<unsigned>* y_interval{new EmptyInterval<unsigned>};
    Interval<unsigned>* tmp{nullptr};

    for (const ICC& c: constraints)
    {
        if (c.xconstrained)
        {
            tmp = x_interval->aggregate(Interval<unsigned>::create(c.xfrom, c.xto));
            delete x_interval;
            x_interval = tmp;
        }
        if (c.yconstrained)
        {
            tmp = y_interval->aggregate(Interval<unsigned>::create(c.yfrom, c.yto));
            delete y_interval;
            y_interval = tmp;
        }
    }

    unsigned xmax{
        x_interval->toDefinedInterval()
            ? x_interval->toDefinedInterval()->getMax()
            : 0
    };
    unsigned ymax{
        y_interval->toDefinedInterval()
            ? y_interval->toDefinedInterval()->getMax()
            : 0
    };

    PARSE_ERROR_IF(x_interval->toDefinedInterval()
                   && x_interval->toDefinedInterval()->getMin() > 0,
                   "no base color for unspecified interval [0;"
                   << x_interval->toDefinedInterval()->getMin()
                   << "[ in where_x clauses");
    PARSE_ERROR_IF(y_interval->toDefinedInterval()
                   && y_interval->toDefinedInterval()->getMin() > 0,
                   "no base color for unspecified interval [0;"
                   << y_interval->toDefinedInterval()->getMin()
                   << "[ in where_y clauses");

    PARSE_ERROR_IF(x_interval->toDefinedInterval()
                   && !x_interval->toDefinedInterval()->continuous(),
                   "missing a default color with discontinuous "
                   "where_x clauses");
    PARSE_ERROR_IF(y_interval->toDefinedInterval()
                   && !y_interval->toDefinedInterval()->continuous(),
                   "missing a default color with discontinuous "
                   "where_y clauses");

    delete x_interval;
    delete y_interval;

    return [xmax,ymax,constraints](int xx, int yy) -> Color
    {
        unsigned x = xmax > 0
            ? (xx >= 0 ? xx % xmax : ((xx + 1) % xmax) + xmax - 1)
            : (xx >= 0 ? xx : std::numeric_limits<unsigned>::max() + xx);
        unsigned y = ymax > 0
            ? (yy >= 0 ? yy % ymax : ((yy + 1) % ymax) + ymax - 1)
            : (yy >= 0 ? yy : std::numeric_limits<unsigned>::max() + yy);
        for (const ICC& c: constraints) {
            if (!c.xconstrained || (x >= c.xfrom && x <= c.xto)) {
                if (!c.yconstrained || (y >= c.yfrom && y <= c.yto)) {
                    return c.color;
                }
            }
        }
        std::exit(33);
    };
}

static std::function<Color(int,int)>
_makeProceduralFunctor_repeat_withBase(const std::vector<ICC>& constraints,
                                       Color base)
{
    Interval<unsigned>* x_interval{new EmptyInterval<unsigned>};
    Interval<unsigned>* y_interval{new EmptyInterval<unsigned>};
    Interval<unsigned>* tmp{nullptr};

    for (const ICC& c: constraints)
    {
        if (c.xconstrained)
        {
            tmp = x_interval->aggregate( Interval<unsigned>::create(c.xfrom, c.xto));
            delete x_interval;
            x_interval = tmp;
        }
        if (c.yconstrained)
        {
            tmp = y_interval->aggregate(Interval<unsigned>::create(c.yfrom, c.yto));
            delete y_interval;
            y_interval = tmp;
        }
    }

    unsigned xmax{
        x_interval->toDefinedInterval()
            ? x_interval->toDefinedInterval()->getMax()
            : 0
    };
    unsigned ymax{
        y_interval->toDefinedInterval()
            ? y_interval->toDefinedInterval()->getMax()
            : 0
    };

    delete x_interval;
    delete y_interval;

    return [xmax,ymax,constraints,base](int xx, int yy) -> Color
    {
        unsigned x = xmax > 0
            ? (xx >= 0 ? xx % xmax : ((xx + 1) % xmax) + xmax - 1)
            : (xx >= 0 ? xx : std::numeric_limits<unsigned>::max() + xx);
        unsigned y = ymax > 0
            ? (yy >= 0 ? yy % ymax : ((yy + 1) % ymax) + ymax - 1)
            : (yy >= 0 ? yy : std::numeric_limits<unsigned>::max() + yy);
        for (const ICC& c: constraints) {
            if (!c.xconstrained || (x >= c.xfrom && x <= c.xto)) {
                if (!c.yconstrained || (y >= c.yfrom && y <= c.yto)) {
                    return c.color;
                }
            }
        }
        return base;
    };
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
            unsigned xx = (x > 0 ? x : 0);
            unsigned yy = (y > 0 ? y : 0);
            for (auto c: constraints) {
                if (!c.xconstrained || (xx >= c.xfrom && xx <= c.xto)) {
                    if (!c.yconstrained || (yy >= c.yfrom && yy <= c.yto)) {
                        return c.color;
                    }
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

    if (ibase != constraints.cend())
    {
        Color color{ibase->color};
        std::remove_if(constraints.begin(), constraints.end(),
                       [](const ICC& c){ return !c.xconstrained && !c.yconstrained; });
        if (repeat)
            return _makeProceduralFunctor_noRepeat(constraints, color);
        else
            return _makeProceduralFunctor_repeat_withBase(constraints, color);
    }
    else
    {
        PARSE_ERROR_IF(!repeat,
                   "can't create a non-repetitive procedural texture "
                   "without a base color "
                   "(having no where_x nor where_y clauses)");
        return _makeProceduralFunctor_repeat_noBase(constraints);
    }
}

Material* Material::parse(tinyxml2::XMLNode* node)
{
  tinyxml2::XMLElement* elt = node->ToElement();

  float ambient  = nan("");
  float diffuse  = nan("");
  float specular = nan("");
  float brilliancy = nan("");
  float refl = nan("");

  elt->QueryFloatAttribute("ambient", &ambient);
  elt->QueryFloatAttribute("diffuse", &diffuse);
  elt->QueryFloatAttribute("specular", &specular);
  elt->QueryFloatAttribute("brilliancy", &brilliancy);
  elt->QueryFloatAttribute("refl", &refl);

  PARSE_ERROR_IF(isnan(ambient) || isnan(diffuse) || isnan(specular) || isnan(brilliancy),
                 "missing attribute for material");

  if (elt->Attribute("type", "simple"))
  {
    // FIXME: color
    tinyxml2::XMLElement* child_elt = node->FirstChild()->ToElement();
    Color c = Color::parse(child_elt);
    std::function<Color(int,int)> fn = [c](int,int) {return c;};
    return new Material(fn, PhongBundle{{ambient, diffuse, specular, brilliancy}}, refl);
  }
  else if (elt->Attribute("type", "procedural"))
  {
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
          PhongBundle{{ambient, diffuse, specular, brilliancy}},
          refl
      };
  }
  else if (elt->Attribute("type", "bitmap"))
  {
      BitmapTextureTranslation trans{0, 0};
      if (const char* xoff = elt->Attribute("x-offset"))
      {
          trans.first = std::atoi(xoff);
      }
      if (const char* yoff = elt->Attribute("y-offset"))
      {
          trans.second = std::atoi(yoff);
      }

      tinyxml2::XMLElement* child_elt = node->FirstChildElement("image");
      PARSE_ERROR_IF(child_elt == nullptr,
                     "missing image node for bitmap material");
      return new BitmapTexture{
          cv::imread(child_elt->GetText()),
          PhongBundle{{ambient, diffuse, specular, brilliancy}},
          refl,
          trans
      };
  }
  else
  {
    std::cerr << "Unrecognized Material of type: " << elt->Attribute("type") << std::endl;
    exit(1);
  }

}
