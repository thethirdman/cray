#include "obj.hh"
#include <tiny_obj_loader.h>
#include <cassert>

Obj* Obj::parse(tinyxml2::XMLNode* node)
{
  const char* name = node->ToElement()->Attribute("path");
  double scale = 1;
  node->ToElement()->QueryDoubleAttribute("scale", &scale);
  Material* mat = nullptr;
  cv::Vec3d trans (0,0,0);

  tinyxml2::XMLNode* child = node->FirstChild();
  do
  {
    tinyxml2::XMLElement* elt_child = child->ToElement();
    if (is_named("material", elt_child))
      mat = Material::parse(elt_child);
    else if (is_named("vec", elt_child) && elt_child->Attribute("name","translate"))
      trans = parseVec(elt_child);
    else
    {
      std::cerr << "Error: invalid node " << child->ToElement()->Name() << std::endl;
      exit(1);
    }

  } while ((child = child->NextSibling()));

  return new Obj(name, *mat, scale, trans, 0.);
}

Triangle convert(tinyobj::mesh_t mesh, Material& mat, double refl)
{
  std::cout << "Mesh size: " << mesh.positions.size() << std::endl;
  assert(mesh.positions.size() == 3);
  return Triangle(mesh.positions[0], mesh.positions[1], mesh.positions[2], mat, refl);
}

cv::Vec3d project(double scale, cv::Vec3d translate, double x, double y, double z)
{
  return cv::Vec3d(x * scale + translate[0], -y * scale + translate[1], z * scale + translate[2]);
  //return cv::Vec3d(x * scale + translate[0], - y * scale + translate[1], z * scale + translate[2]);
}

Obj::Obj(const char* fname, Material& mat, double scale, cv::Vec3d translate, double refl)
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

      cv::Vec3d pt1 = project(scale, translate, positions[index1 * 3], positions[index1 * 3 + 1], positions[index1 * 3 + 2]);
      cv::Vec3d pt2 = project(scale, translate, positions[index2 * 3], positions[index2 * 3 + 1], positions[index2 * 3 + 2]);
      cv::Vec3d pt3 = project(scale, translate, positions[index3 * 3], positions[index3 * 3 + 1], positions[index3 * 3 + 2]);

      contents.push_back(new Triangle(pt1,pt2,pt3,material_, refl_coef_));
    }
  }
  polygons_.buildTree(contents);
  bbox_ = polygons_.getBBox();
}
