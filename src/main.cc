#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "scene.hh"
#include "camera.hh"

void usage(char* pname)
{
  std::cout << "Usage: " << pname << " source.sce x y" << std::endl;
  std::cout << "  source.sce: file describing the scene" << std::endl;
  std::cout << "  x and y   : dimensions of the generated image" << std::endl;
}

Scene& parse(std::fstream& stream);

int main(int argc, char** argv)
{
  if (argc != 4)
  {
    usage(argv[0]);
    return 1;
  }

  int x_size = atoi(argv[2]);
  int y_size = atoi(argv[3]);
  int realx = SIZE_FACTOR * x_size;
  int realy = SIZE_FACTOR * y_size;
  Camera* camera = new Camera(cv::Vec3i(0,0,-700), cv::Vec3d(0., 0., 1.), cv::Vec3d(0., 1., 0.)
      , realx, realy);

  Shape* sphere = new Sphere(cv::Vec3i(0,0,280), Color(0, 255, 255), 25);
  //Ray* ray = new Ray(cv::Vec3i(0,100,0), cv::Vec3b(0,0,1));

  //Ray *res = sphere->intersect(*ray);
  //std::cout << res << std::endl;
  Shape* sphere1 = new Sphere(cv::Vec3i(0,125,300), Color(255,0,0), 75);
  Shape* sphere2 = new Sphere(cv::Vec3i(150,50,290), Color(0,255,0), 50);
  Shape* tri1 = new Triangle(cv::Vec3d(-100,-100,300),
      cv::Vec3d(-50, -100, 300),
      cv::Vec3d(-75, -25, 325), Color(255,255,0));
  Shape* tri2 = new Triangle(
      cv::Vec3d(-500,500, 0),
      cv::Vec3d(500,500, 0),
      cv::Vec3d(0,500, 4000),
      Color(255,255,255));

  std::vector<Shape*>* shapes = new std::vector<Shape*>(5);
  shapes->at(0) = sphere;
  shapes->at(1) = sphere1;
  shapes->at(2) = sphere2;
  shapes->at(3) = tri1;
  shapes->at(4) = tri2;

  Light light(cv::Vec3d(0., -200., 100.), Color(255,255,255));
  //Light light1(cv::Vec3d(200., 0., 200.), Color(255,0,0));
  std::vector<Light>* lights = new std::vector<Light>(1);
  lights->at(0) = light;
  //lights->at(1) = light1;


  //std::fstream stream(argv[1]);
  Scene scene (*camera, *shapes, *lights); // = parse(stream);
  scene.setDims(realx, realy);

  scene.render();

  scene.save("out.png");

  return 0;

}
