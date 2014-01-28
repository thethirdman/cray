#include "scene.hh"
#include "vector.hh"
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

Shape* Scene::hit(Ray& ray, Vec3d& best_hit, double& best_dist)
{

  return shapes_.intersect(ray, best_hit, best_dist);
}

Color Scene::render_light(Ray &ray, Vec3d intersection, Light& l, Shape& shape, int depth)
{
  Color color = l.illumination(shape, ray, intersection, shapes_);

  // Reflection rendering
  // We launch the reflected ray
  // FIXME: reflection (shift)
  Ray refl_ray = shape.reflect(Ray(intersection, ray.dir()));
  Color refl_color = color;
  float refl_coef = shape.getMaterial().get_refl();
  //std::cout << "Refl: " << refl_coef << std::endl;
  if (depth < 5 && refl_coef > 0)
  {
    // FIXME: make diffuse reflection
    refl_color = ray_launch(refl_ray, depth + 1);

    if (refl_color.max() != 0)
      color = ponderate(color, 1. - refl_coef) + ponderate(refl_color, refl_coef);
  }
  return color;
}



Color Scene::ray_launch(Ray& ray, int depth)
{

  Shape* shape;
  // We first check for a hit with a shape
  Vec3d intersection;
  double inter_dist;

  Color result;

  // if there is a hit, we take into account the lights of the scene
  if ((shape = hit(ray, intersection, inter_dist)))
    for (auto l : lights_)
      result = result + render_light(ray, intersection, l, *shape, depth); // FIXME

  return result;
}

// For each ray, compute the color
void Scene::render(void)
{
  std::cout << "RENDER" << std::endl;
  std::vector<Ray>& mat = cam_.getRays();

  int max = x_ * y_;
  int prev = -1;
  int cur = 0;
  for (int j = 0; j < y_; j++)
    for (int i = 0; i < x_; i++)
    {
      cur = j * x_ + i;
      int percent = (int) (100 * ((float) cur)/((float) max));
      if (percent != prev)
        std::cout << percent << "% (" << cur << "/" << max << ")\r" << std::flush;
      prev = percent;
      canvas_[j*x_ + i] = ray_launch(mat[j * x_ + i], 0);
    }
  std::cout << std::endl;
}


void Scene::save(std::string fname)
{
  int out_y = y_ / SIZE_FACTOR;
  int out_x = x_ / SIZE_FACTOR;
  cv::Mat img (out_y, out_x, CV_8UC3);

  std::cout << "SAVE" << std::endl;
  std::cout << y_ << " " << x_ << std::endl;

  for (int j = 0; j < out_y; j++)
    for (int i = 0; i < out_x; i++)
    {
      Color total (0,0,0,0);
      for (int b = j * SIZE_FACTOR; b < (j+1) * SIZE_FACTOR; b++)
        for (int a = i * SIZE_FACTOR; a < (i+1) * SIZE_FACTOR; a++)
        {
          Color cur = canvas_[b * x_ + a];
          if (cur.max() == 0)
            total = total + Color(0,0,0);
          else
            total = total + canvas_[b * x_ + a];
        }
      img.at<cv::Vec3b>(j,i) = total.toBgr();

    }


  cv::imwrite(fname, img);
}
