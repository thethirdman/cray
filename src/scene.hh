#ifndef SCENE_HH_
# define SCENE_HH_

#include <string>
#include <cv.h>
#include <highgui.h>
#include "camera.hh"
#include "shape.hh"
#include "light.hh"

#define SIZE_FACTOR 1

class Scene
{
  public:
    Scene(Camera& cam, std::vector<Shape*>& shapes, std::vector<Light>& lights)
      : cam_(cam), shapes_(shapes), lights_(lights) {} // FIXME: remove
    void setDims(int x, int y);
    void render(void);
    void save(std::string fname);

  private:
    std::pair<cv::Vec3d, double>* hit(Ray& ray, int* s_id);
    Color ray_launch(Ray& ray, int depth);
    double soft_shadows_comp(Ray& lray, Shape& shape);
    int x_;
    int y_;
    Camera& cam_;
    std::vector<Shape*>& shapes_; // FIXME: have real structure after
    std::vector<Light>& lights_;
    std::vector<Color> canvas_;
};

#endif
