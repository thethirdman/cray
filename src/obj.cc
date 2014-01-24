#include "obj.hh"
#include <tiny_obj_loader.h>
#include <cassert>

Obj* Obj::parse(tinyxml2::XMLNode* node)
{
  const char* name = node->ToElement()->Attribute("path");
  double scale = 1;
  node->ToElement()->QueryDoubleAttribute("scale", &scale);
  Material* mat = nullptr;
  Vec3d trans (0,0,0);
  double rot[3];

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt_child = child->ToElement();
    if (is_named("material", elt_child))
      mat = Material::parse(elt_child);
    else if (is_named("vec", elt_child) && elt_child->Attribute("name","translate"))
      trans = parseVec(elt_child);
    else if (is_named("rotate", elt_child))
    {
      elt_child->QueryDoubleAttribute("xrot", &rot[0]);
      elt_child->QueryDoubleAttribute("yrot", &rot[1]);
      elt_child->QueryDoubleAttribute("zrot", &rot[2]);
    }
    else
    {
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }

  } while ((child = child->NextSibling()));

  // Deg to radians
  for (int i = 0; i < 3; i++)
    rot[i] = M_PI * rot[i]/180;
  return new Obj(name, *mat, scale, trans, rot, 0.);
}

Vec3d project(double scale, Vec3d translate, double rot[], double x, double y, double z)
{
  x = x * scale + translate[0];
  y = -y * scale + translate[1];
  z = z * scale + translate[2];

  double xrot = cos(rot[1]) * cos(rot[2]) * x
              + (cos(rot[0])*sin(rot[2]) + sin(rot[0])*sin(rot[1])*cos(rot[2])) * y
              + (sin(rot[0])*sin(rot[2]) - cos(rot[0])*sin(rot[1])*cos(rot[2])) * z;

  double yrot = -cos(rot[1]) * sin(rot[2]) * x
              + (cos(rot[0])*cos(rot[2]) - sin(rot[0])*sin(rot[1])*sin(rot[2])) * y
              + (sin(rot[0])*cos(rot[2]) + cos(rot[0])*sin(rot[1])*sin(rot[2])) * z;

  double zrot = sin(rot[1]) * x - sin(rot[0])*cos(rot[1]) * y + cos(rot[0])*cos(rot[1])*z;

  return Vec3d(xrot, yrot, zrot);
}

Obj::Obj(const char* fname, Material& mat, double scale, Vec3d translate, double rot[], double refl)
  : Shape(mat, refl), name_(fname)
{
  std::vector<tinyobj::shape_t> shapes;
  std::vector<Shape*> contents;
  std::string err = tinyobj::LoadObj(shapes, name_, "scenes/");
  std::cout << "Error: " << err << std::endl;

  for (unsigned int s = 0; s < shapes.size(); s++)
  {
    std::vector<float>& positions = shapes[s].mesh.positions;
    for (unsigned int idx = 0; idx < shapes[s].mesh.indices.size() / 3; idx++)
    {
      unsigned int index1 =  shapes[s].mesh.indices[3 * idx];
      unsigned int index2 =  shapes[s].mesh.indices[3 * idx + 1];
      unsigned int index3 =  shapes[s].mesh.indices[3 * idx + 2];

      Vec3d pt1 = project(scale, translate, rot,
                          positions[index1 * 3],
                          positions[index1 * 3 + 1],
                          positions[index1 * 3 + 2]);
      Vec3d pt2 = project(scale, translate, rot,
                          positions[index2 * 3],
                          positions[index2 * 3 + 1],
                          positions[index2 * 3 + 2]);
      Vec3d pt3 = project(scale, translate, rot,
                          positions[index3 * 3],
                          positions[index3 * 3 + 1],
                          positions[index3 * 3 + 2]);

      contents.push_back(new Triangle(pt1,pt2,pt3,material_, refl_coef_));
    }
  }
  polygons_.buildTree(contents);
  bbox_ = polygons_.getBBox();
}

bool Obj::computeColorFromTexture(const Vec3d& where, Color& out) const
{
    // FIXME
    return true;
}
