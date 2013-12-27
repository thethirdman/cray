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
      while (xmlshapes = xmlshapes->NextSibling());
    }
    else if (is_named("lights", child))
    {
      tinyxml2::XMLNode* xmllights = child->FirstChild();
      do
      {
        lights->push_back(Light::parse(xmllights));
      }
      while (xmllights = xmllights->NextSibling());
    }

    else
      assert_node(child, "camera or shapes or lights.");
  }
  while (child = child->NextSibling());

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

double Scene::soft_shadows_comp(Ray& lray, Shape& shape)
{
  // We first compute the number of rays we wanted to launch based on the
  // normal to the intersection point: if it is close to 0, we may be on the
  // border of an object, and more rays should be launched.
  // FIXME: something sdirs is smaller that 0, which make ray_num false
  double sdir = shape.normal(lray).dot(normalize(lray.dir()));

  // The multiplication by 0 is here to disable the soft_shadows
  int ray_num = 0 * (1. - sdir);
  int hit_count = 0;

  for (int  rn = 0; rn < ray_num; rn ++)
  {
    // We launch a ray with a random direction
    cv::Vec3d shift = 50 * ((2 * cv::Vec3d(normal_rand(), normal_rand(), normal_rand())) - cv::Vec3d(1.0, 1.0, 1.0));
    cv::Vec3d shift_dir = normalize(lray.dir() + shift);

    // classic intersection check
    cv::Vec3d intersect;
    double dist;

    if (shape.intersect(Ray(lray.orig(), shift_dir), intersect, dist))
      if (dist > 0)
        hit_count++;
  }

  // Hack to disable the soft shadows
  if (ray_num == 0)
    return 1.;

  // We then do the mean over rays launched: if a lot of rays are launched, but
  // few hit the shape, it means that we are on the border of the shape.
  double shadowed = ((double) hit_count)/((double) ray_num);

  // This return check is rubbish
  return clamp_one(shadowed);
}

Color Scene::render_light(Ray &ray, cv::Vec3d intersection, Light& l, Shape& shape, int depth)
{
  // We define the ray from the shape to the light
  cv::Vec3d dir_light = normalize(l.orig() - intersection);
  double light_dist = cv::norm(l.orig() - intersection);
  double shadowed = 0.0;

  Ray light_ray (intersection, dir_light);
  // If this ray its a shape, it shadowed.
  for (unsigned int s = 0; s < shapes_.size(); s++)
  {
    cv::Vec3d hit;
    double dist;

    if (shapes_[s]->intersect(light_ray, hit, dist) && &shape != shapes_[s])
    {
      // If it is shadowed, we try a soft shadow rendering
      //Ray foo (intersection, l.orig() - intersection);
      if (dist < light_dist)
        shadowed = 1;// soft_shadows_comp(foo, *shapes_[s]);
    }
  }

  // FIXME: light ray could be recomputed for tidyness
  Color color = l.illumination(shape, ray, light_ray, shadowed);

  // Reflection rendering
  // We launch the reflected ray
  Ray refl_ray = shape.reflect(Ray(intersection, ray.dir()));
  Color refl_color = shape.getColor();
  if (depth < 1 && shape.refl() > 0)
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

  return result/lights_.size();
}

// For each ray, compute the color
void Scene::render(void)
{
  std::cout << "RENDER" << std::endl;
  std::vector<Ray>& mat = cam_.getRays();

  for (int j = 0; j < y_; j++)
    for (int i = 0; i < x_; i++)
      canvas_[j*x_ + i] = ray_launch(mat[j * x_ + i], 0);
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
