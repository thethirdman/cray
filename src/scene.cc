#include "scene.hh"
#include <limits>

Scene* Scene::parse(char* path, int x, int y)
{
  Camera* camera = NULL;
  std::vector<Shape*>* shapes = new std::vector<Shape*>();
  std::vector<Light>* lights = new std::vector<Light>();

  tinyxml2::XMLDocument doc;
  doc.LoadFile(path);

  tinyxml2::XMLNode* rootnode = doc.FirstChild();
  assert_node(rootnode, "scene");

  tinyxml2::XMLNode* child = rootnode->FirstChild();
  do
  {
    if (is_named("camera", child))
      camera = Camera::parse(child->FirstChild(), x, y);
    else if (is_named("shapes", child))
    {
      tinyxml2::XMLNode* xmlshapes = child->FirstChild();
      do
      {
        shapes->push_back(Shape::parse(xmlshapes));
      }
      while ((xmlshapes = xmlshapes->NextSibling()));
    }
    else if (is_named("lights", child))
    {
      tinyxml2::XMLNode* xmllights = child->FirstChild();
      do
      {
        lights->push_back(Light::parse(xmllights));
      }
      while ((xmllights = xmllights->NextSibling()));
    }

    else
      assert_node(child, "camera or shapes or lights.");
  }
  while ((child = child->NextSibling()));

  // FIXME: type
  Scene* res = new Scene(*camera, *shapes, *lights);
  res->setDims(x,y);
  return res;
}

void Scene::setDims(int x, int y)
{
  canvas_ = std::vector<Color>(x * y);
  x_ = x;
  y_ = y;
}

bool Scene::hit(Ray& ray, int& s_id, cv::Vec3d& best_hit, double& best_dist)
{
  int shape_id = -1;

  // For each shape, if we hit something, we check if it is the smallest
  // distance we found yet.
  for (unsigned int s = 0; s < shapes_.size(); s++)
  {
    cv::Vec3d intersect;
    double dist;

    if (shapes_[s]->intersect(ray, intersect, dist))
    {
      if (shape_id == -1 || (dist < best_dist))
      {
        shape_id = s;
        best_hit = intersect;
        best_dist = dist;
      }
    }
  }
  s_id = shape_id;
  return (shape_id != -1 ? true : false);
}

Color Scene::render_light(Ray &ray, cv::Vec3d intersection, Light& l, Shape& shape, int depth)
{
  Color color = l.illumination(shape, ray, intersection, shapes_);

  // Reflection rendering
  // We launch the reflected ray
  Ray refl_ray = shape.reflect(Ray(intersection, ray.dir()));
  Color refl_color = color;
  if (depth < 0 && shape.refl() > 0)
  {
    // FIXME: make diffuse reflection
    refl_color = ray_launch(refl_ray, depth + 1);
    double refl_coef = shape.refl();

    if (refl_color.max() != 0)
      color = ((1. - refl_coef)*color) + (refl_coef*refl_color);
  }
  return color;
}



Color Scene::ray_launch(Ray& ray, int depth)
{

  int shape_id;
  // We first check for a hit with a shape
  cv::Vec3d intersection;
  double inter_dist;

  Color result;

  // if there is a hit, we take into account the lights of the scene
  if (hit(ray, shape_id, intersection, inter_dist))
    for (auto l : lights_)
      result = result + render_light(ray, intersection, l, *shapes_[shape_id], depth);

  return result;
}

// For each ray, compute the color
void Scene::render(void)
{
  std::cout << "RENDER" << std::endl;
  std::vector<Ray>& mat = cam_.getRays();

  for (int j = 0; j < y_; j++)
    for (int i = 0; i < x_; i++)
    {
      int max = x_ * y_;
      int cur = j * x_ + i;
      //std::cout << (int) (100 * ((float) cur)/((float) max)) << "% (" << cur << "/" << max << ")\r" << std::flush;
      canvas_[j*x_ + i] = ray_launch(mat[j * x_ + i], 0);
    }
  std::cout << std::endl;
}


void Scene::save(std::string fname)
{
  cv::Mat img (y_, x_, CV_8UC3);

  std::cout << "SAVE" << std::endl;
  std::cout << y_ << " " << x_ << std::endl;

  for (int j = 0; j < y_; j++)
    for (int i = 0; i < x_; i++)
      img.at<cv::Vec3b>(j,i) = canvas_[j * x_ + i].toBgr();

  cv::Mat r_img (y_ /SIZE_FACTOR, x_ / SIZE_FACTOR, CV_8UC3);

  cv::resize(img, r_img, r_img.size(), CV_INTER_CUBIC);
  cv::imwrite(fname, r_img);
}
