#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "scene.hh"
#include "camera.hh"

void usage(char* pname)
{
  std::cout << "Usage: " << pname << " source.xml result.img x y" << std::endl;
  std::cout << "  source.xml: file describing the scene" << std::endl;
  std::cout << "  result.img: file containing the result" << std::endl;
  std::cout << "  x and y   : dimensions of the generated image" << std::endl;
}

Scene& parse(std::fstream& stream);

int main(int argc, char** argv)
{
  if (argc != 5)
  {
    usage(argv[0]);
    return 1;
  }

  int x_size = atoi(argv[3]);
  int y_size = atoi(argv[4]);
  int realx = SIZE_FACTOR * x_size;
  int realy = SIZE_FACTOR * y_size;

  Scene* scene = Scene::parse(argv[1], realx, realy);
  scene->render();

  scene->save(argv[2]);

  return 0;

}
