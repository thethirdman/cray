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

  // FIXME: Testing code to remove once we have parsing of a scene file
  int x_size = atoi(argv[2]);
  int y_size = atoi(argv[3]);
  int realx = SIZE_FACTOR * x_size;
  int realy = SIZE_FACTOR * y_size;

  Camera* camera = new Camera(cv::Vec3i(0,0,0), cv::Vec3d(0., 0., 1.), cv::Vec3d(0., 1., 0.)
      , realx, realy);

  Shape* sphere = new Sphere(cv::Vec3d(-1,0.25,5), Color(0, 255, 255), 0.5, 0.1);

  Shape* sphere1 = new Sphere(cv::Vec3i(1,0,7), Color(255,0,0), 0.75, 0.5);
  Shape* sphere2 = new Sphere(cv::Vec3i(0,-1,7), Color(0,255,0), 0.50, 0.5);

  Shape* plane   = new Plane(cv::Vec3d(0,    0.75,  0), cv::Vec3d(1,0,0), cv::Vec3d(0,0,1), Color(255,255,255), 0.5);
  Shape* plane1  = new Plane(cv::Vec3d(-3,   0,     0), cv::Vec3d(0,1,0), cv::Vec3d(0,0,1), Color(255,200,200), 0);
  Shape* plane2  = new Plane(cv::Vec3d(3,    0,     0), cv::Vec3d(0,1,0), cv::Vec3d(0,0,1), Color(200,255,200), 0);
  Shape* plane3  = new Plane(cv::Vec3d(0,    0,     10), cv::Vec3d(0,1,0), cv::Vec3d(1,0,0), Color(200,200,255), 0);
  Shape* plane4  = new Plane(cv::Vec3d(0,   -3,     0), cv::Vec3d(1,0,0), cv::Vec3d(0,0,1), Color(255,255,255), 0);
  Shape* tri1 = new Triangle(cv::Vec3d(1.80, 0,     7),
                             cv::Vec3d(1,     0.75, 4),
                             cv::Vec3d(3,     0.75, 7), Color(255,255,0), 0.5);
  //Shape* tri2 = new Triangle(
  //    cv::Vec3d(-10000,5000, 0),
  //    cv::Vec3d(5000,5000, 0),
  //    cv::Vec3d(0,5000, 40000),
  //    Color(255,255,255));

  std::vector<Shape*>* shapes = new std::vector<Shape*>();
  shapes->push_back(sphere);
  shapes->push_back(sphere1);
  shapes->push_back(sphere2);
  shapes->push_back(plane);
  shapes->push_back(tri1);
  shapes->push_back(plane1);
  shapes->push_back(plane2);
  shapes->push_back(plane3);
  shapes->push_back(plane4);

  Light light(cv::Vec3d(-2.9, -2.9, 9.9), Color(255,255,255));
  Light light1(cv::Vec3d(-2.9, -2.9, 0.), Color(255,255,255));
  Light light2(cv::Vec3d(2.9,  -2.9, 9.9), Color(255,255,255));
  Light light3(cv::Vec3d(2.9,  -2.9, 0.), Color(255,255,255));

  std::vector<Light>* lights = new std::vector<Light>(1);
  lights->push_back(light);
  lights->push_back(light1);
  lights->push_back(light2);
  lights->push_back(light3);


  //std::fstream stream(argv[1]);
  Scene scene (*camera, *shapes, *lights); // = parse(stream);
  scene.setDims(realx, realy);

  scene.render();

  scene.save("out.png");

  return 0;

}
