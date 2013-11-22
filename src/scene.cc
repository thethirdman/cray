#include "scene.hh"
#include <limits>

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
  for (int s = 0; s < shapes_.size(); s++)
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

Color Scene::ray_launch(Ray& ray, int depth)
{

  int shape_id;
  // We first check for a hit with a shape
  cv::Vec3d intersection;
  double inter_dist;
  
  Color result;

  // if there is a hit, we take into account the lights of the scene
  if (hit(ray, shape_id, intersection, inter_dist))
  {
    for (auto l : lights_)
    {

      // We define the ray from the shape to the light
      cv::Vec3d dir_light = normalize(l.orig() - intersection);
      double shadowed = 0.0;

      // If this ray its a shape, it shadowed.
      for (int s = 0; s < shapes_.size(); s++)
      {
        cv::Vec3d hit; 
        double dist;
        
        if (shapes_[s]->intersect(Ray(intersection, dir_light), hit, dist) && shape_id != s)
        {
          // If it is shadowed, we try a soft shadow rendering
          Ray foo (intersection, l.orig() - intersection);
          shadowed = soft_shadows_comp(foo, *shapes_[s]);
        }
      }

      // FIXME: the lightning section is imho, bugged. White objects appear
      // gray because of the reflection stuff, and the whole thing should be
      // redesigned: the diffusion value should be associated to an object to
      // simulate different materials, as well as the reflection. Is also think
      // that ambien lightning should not be a constant value, but computed
      // somehow.
      // I'm not sure if we should to the mean of the different light values.


      // Lambert light diffusion method.
      // The dot product defines a "lightning" coefficient which ponderates the
      // intensity of the color of the shape.
      // Shadows apply only to the diffuse light vaue
      double diffval = shapes_[shape_id]->normal(intersection).dot(dir_light) - shadowed;
      double ambient = 0.1;
      Color color;
      if (diffval > ambient)
      {
        Color diffuse = l.getColor() * (diffval < 0 ? 0 : diffval);
        color = (shapes_[shape_id]->getColor() * diffuse);
      }
      // Ambient light value: the color that an objet will have if it is not
      // illuminated.
      else
        color = shapes_[shape_id]->getColor() * ambient;


      // Reflection rendering
      // We launch the reflected ray
      Ray refl_ray = reflect(Ray(intersection, ray.dir()), *shapes_[shape_id]);
      Color refl_color = shapes_[shape_id]->getColor();
      if (depth < 5)
      {
        // FIXME: make diffuse reflection
        refl_color = ray_launch(refl_ray, depth + 1);
        double refl_coef = shapes_[shape_id]->refl();

        if (refl_color.max() != 0)
          color = ((1. - refl_coef)*color) + (refl_coef*refl_color);
      }

      //Color light_v = diffuse + ambient;
      result = result + color;
    }
  }

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
