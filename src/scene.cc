#include "scene.hh"
#include <limits>

void Scene::setDims(int x, int y)
{
  canvas_ = std::vector<Color>(x * y);
  x_ = x;
  y_ = y;
}

std::pair<cv::Vec3d, double>* Scene::hit(Ray& ray, int* s_id)
{
  std::pair<cv::Vec3d, double>* best_hit = 0;
  int shape_id = -1;

  for (int s = 0; s < shapes_.size(); s++)
  {
    std::pair<cv::Vec3d, double>* hit = shapes_[s]->intersect(ray);
    if (hit)
    {
      if (shape_id == -1 || (best_hit && hit->second < best_hit->second))
      {
        shape_id = s;
        if (best_hit)
          delete best_hit;
        best_hit = hit;
      }
      else
        delete hit;
    }
  }
  *s_id = shape_id;
  return best_hit;
}

// FIXME: find this function a home
static Ray reflect(Ray ray, Shape& shape)
{
  cv::Vec3d normal_dir = shape.normal(ray.orig());
  cv::Vec3d refl_dir = ray.dir() - 2.0 * normal_dir * ray.dir().dot(normal_dir);
  const double shift = std::numeric_limits<double>::epsilon() * 65536.0;
  return Ray (ray.orig() + normal_dir * shift, normalize(refl_dir));
}

double Scene::soft_shadows_comp(Ray& lray, Shape& shape)
{
  double sdir = shape.normal(lray.orig()).dot(normalize(lray.dir()));
  int ray_num = 0 * (1. - sdir);
  int hit_count = 0;
  for (int  rn = 0; rn < ray_num; rn ++)
  {
    cv::Vec3d shift = 50 * ((2 * cv::Vec3d(normal_rand(), normal_rand(), normal_rand())) - cv::Vec3d(1.0, 1.0, 1.0));
    cv::Vec3d shift_dir = normalize(lray.dir() + shift);
    std::pair<cv::Vec3d, double>* rnhit = shape.intersect(Ray(lray.orig(), shift_dir));
    if (rnhit)
    {
      if (rnhit->second > 0)
        hit_count++;
      delete rnhit;
    }
  }
  if (ray_num == 0)
    return 1.;
  double shadowed = ((double) hit_count)/((double) ray_num);
  return (shadowed > 1 ? 1. : shadowed);
}

Color Scene::ray_launch(Ray& ray, int depth)
{

  int shape_id;
  std::pair<cv::Vec3d, double>* contact = hit(ray, &shape_id);
  Color result (0.,0.,0.);

  if (contact!= NULL)
  {
    for (auto l : lights_)
    {
      cv::Vec3d& intersection = contact->first;
      cv::Vec3d dir_light = normalize(l.orig() - intersection);
      double shadowed = 0.0;

      for (int s = 0; s < shapes_.size(); s++)
      {
        std::pair<cv::Vec3d,double>* shit = shapes_[s]->intersect(Ray(intersection, dir_light));
        if (shit && shape_id != s)
        {
          Ray foo (intersection, l.orig() - intersection);
          shadowed = soft_shadows_comp(foo, *shapes_[s]);
          delete shit;
        }
      }
      int rid;
      Ray normal_ray (intersection, shapes_[shape_id]->normal(intersection));

      Ray refl_ray = reflect(Ray(intersection, ray.dir()), *shapes_[shape_id]);
      Color refl_color = shapes_[shape_id]->getColor();

      double diffval = shapes_[shape_id]->normal(intersection).dot(dir_light);
      Color diffuse = l.getColor() * (diffval < 0 ? 0 : diffval);
      Color ambient = 0.1 * l.getColor();
      diffuse = diffuse - shadowed;
      Color light_v = diffuse + ambient;
      Color color = shapes_[shape_id]->getColor();

      if (depth < 3)
      {
        refl_color = ray_launch(refl_ray, depth + 1);
        double refl_coef = 0.5;
        color = ((1. - refl_coef)*color).add(refl_coef*refl_color);
      }

      result = result + (color * light_v);
    }
  }

  return result/lights_.size();
}

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
