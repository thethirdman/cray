#include "camera.hh"


Camera* Camera::parse(tinyxml2::XMLNode* node, int x, int y)
{
  cv::Vec3d pos;
  cv::Vec3d dir;
  cv::Vec3d up;

  bool setPos = false;
  bool setDir = false;
  bool setUp = false;

  do
  {
    assert_node(node, "vec");
    tinyxml2::XMLElement* elt = node->ToElement();
    if (!setPos && elt->Attribute("name", "pos"))
    {
      pos = parseVec(elt);
      setPos = true;
    }
    else if (!setDir && elt->Attribute("name", "dir"))
    {
      dir = parseVec(elt);
      setDir = true;
    }
    else if (!setUp && elt->Attribute("name", "up"))
    {
      up = parseVec(elt);
      setUp = true;
    }
    else
    {
      std::cerr << "Error: unexpected vec named: " << elt->Attribute("name") << std::endl;
      exit(1);
    }
  } while ((node = node->NextSibling()));

  if (!setPos || !setDir || !setUp)
  {
    std::cerr << "Error: missing vec in camera, either pos, dir or up" << std::endl;
    exit(1);
  }

  return new Camera(pos,dir,up,x,y);
}
