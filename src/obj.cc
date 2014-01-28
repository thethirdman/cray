#include "obj.hh"
#include <tiny_obj_loader.h>
#include <cassert>
#include <list>

Obj* Obj::parse(tinyxml2::XMLNode* node)
{
  const char* name = node->ToElement()->Attribute("path");
  double scale = 1;
  node->ToElement()->QueryDoubleAttribute("scale", &scale);
  Material* mat = nullptr;
  Vec3d trans (0,0,0);
  double rot[3];

  bool interp = node->ToElement()->Attribute("interp", "true");

  if(interp)
    std::cout << "INTERP" << std::endl;

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
  return new Obj(name, *mat, scale, trans, rot, interp);
}

Vec3d rotate(double rot[], double x, double y, double z)
{

  double xrot = cos(rot[1]) * cos(rot[2]) * x
              + (cos(rot[0])*sin(rot[2]) + sin(rot[0])*sin(rot[1])*cos(rot[2])) * y
              + (sin(rot[0])*sin(rot[2]) - cos(rot[0])*sin(rot[1])*cos(rot[2])) * z;

  double yrot = -cos(rot[1]) * sin(rot[2]) * x
              + (cos(rot[0])*cos(rot[2]) - sin(rot[0])*sin(rot[1])*sin(rot[2])) * y
              + (sin(rot[0])*cos(rot[2]) + cos(rot[0])*sin(rot[1])*sin(rot[2])) * z;

  double zrot = sin(rot[1]) * x - sin(rot[0])*cos(rot[1]) * y + cos(rot[0])*cos(rot[1])*z;
  return Vec3d(xrot, yrot, zrot);
}

Vec3d project(double scale, Vec3d translate, double rot[], double x, double y, double z)
{
  x = x * scale + translate[0];
  y = -y * scale + translate[1];
  z = z * scale + translate[2];

  return rotate(rot, x, y, z);
}

Obj::Obj(const char* fname, Material& mat, double scale, Vec3d translate, double rot[], bool interp)
  : Shape(mat), name_(fname)
{
  std::vector<tinyobj::shape_t> shapes;
  std::vector<Shape*> contents;
  std::string err = tinyobj::LoadObj(shapes, name_, "scenes/");
  std::cout << "Error: " << err << std::endl;
  std::map<Vec3d, std::list<Triangle*>> ptMap;

  bool existNormals = false;

  for (unsigned int s = 0; s < shapes.size(); s++)
  {
    std::vector<float>& positions = shapes[s].mesh.positions;
    std::vector<float>& normals = shapes[s].mesh.normals;

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

      if (interp)
      {
        NormalTriangle* t = new NormalTriangle(pt1, pt2, pt3, material_);

        contents.push_back(t);
        if (normals.size() == 0)
        {
          ptMap[pt1].push_back(t);
          ptMap[pt2].push_back(t);
          ptMap[pt3].push_back(t);
        }
        else
        {
          existNormals = true;

          //std::cout << "Normal size: " << std::endl;
          assert(positions.size() == normals.size());
          // FIXME: should be +0 +1 +2 instead of this
          Vec3d n1  = rotate(rot, normals[index1 * 3 + 0], -normals[index1 * 3 + 1], normals[index1 * 3 + 2]);
          Vec3d n2  = rotate(rot, normals[index2 * 3 + 0], -normals[index2 * 3 + 1], normals[index2 * 3 + 2]);
          Vec3d n3  = rotate(rot, normals[index3 * 3 + 0], -normals[index3 * 3 + 1], normals[index3 * 3 + 2]);

          t->setNormal(1, n1);
          t->setNormal(2, n2);
          t->setNormal(3, n3);
        }
      }
      else
        contents.push_back(new Triangle(pt1, pt2, pt3, material_));
    }
  }

  if (interp && !existNormals)
  {
    for (unsigned int i = 0; i < contents.size(); i++)
    {
      ((NormalTriangle*) contents[i])->updateNormals(ptMap);
    }
  }

  std::cout << "Size: " << contents.size() << std::endl;

  polygons_.buildTree(contents);
  bbox_ = polygons_.getBBox();
}

bool Obj::containsPoint(const Vec3d& pt) const
{
    return polygons_.findSurroundingShape(pt) != nullptr;
}

bool Obj::computeColorFromTexture(const Vec3d& where, Color& out) const
{
    Shape* s = polygons_.findSurroundingShape(where);

    if (s == nullptr) // no Shape in polygons_ does contain where.
    {
        return false;
    }

    bool obvious_answer = s->computeColorFromTexture(where, out);
    assert(obvious_answer == true);
    return obvious_answer;
}
