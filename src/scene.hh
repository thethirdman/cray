#ifndef SCENE_HH_
# define SCENE_HH_

#include <string>
#include <cv.h>
#include <highgui.h>
#include <tinyxml2.h>
#include "utils.hh"
#include "camera.hh"
#include "shape.hh"
#include "light.hh"

// The size factor is used for supersampling. Supersampling is a technique used
// in order to have simple anti-aliasing on a scene. It creates a bigger image
// (and therefore, more precise), and then reduces it with interpolation before
// saving it;
#define SIZE_FACTOR 1

/// The scene contains all the necessary element to render an image
class Scene
{
  public:
    Scene(Camera& cam, std::vector<Shape*>& shapes, std::vector<Light>& lights)
      : cam_(cam), shapes_(shapes), lights_(lights) {} // FIXME: remove

    // Returns a fresh scene parsed from a file
    static Scene* parse(char* path, int x, int y);

    // Sets the dimensions of the image to render
    void setDims(int x, int y);

    // Renders and scene and fill canvas_
    void render(void);

    // Saves canvas_ into fname
    void save(std::string fname);

  private:
    // Launches a ray into a scene, and tries to hit a shape. Returns the
    // closest shape hit, with the location of the intersection point and its
    // distance to the origin of the ray. s_id represents the shape that is
    // hit. If no hit, returns NULL and s_id is set to -1
    // FIXME: better return type ?
    bool hit(Ray& ray, int& s_id, cv::Vec3d& intersect, double& dist);

    Color render_light(Ray &ray, cv::Vec3d intersection, Light& l, Shape& shape, int depth);
    /// Does the complete rendering of the scene for a given ray.
    // The depth parameter is used to determine the maximum reflection depth
    // (reflection computation is just a recursion with a new ray).
    // Returns the color associated to the pixel that we try to render
    Color ray_launch(Ray& ray, int depth);

    // Soft shadows rendering method. For now, sucks a lot, since it takes time
    // for a bad result.
    double soft_shadows_comp(Ray& lray, Shape& shape);

    // The dimension of the canvas to render.
    // FIXME: redundant with camera x_ and y_. Better design possible ?
    int x_;
    int y_;

    // The view point of view
    Camera& cam_;

    // The shapes of the scene
    // TODO: later, instead of using a vector, we should use a better
    // data-structure such as a BSP
    std::vector<Shape*>& shapes_; // FIXME: have real structure after

    // The lights illuminating the scene
    std::vector<Light>& lights_;

    // The pixel of the image we render
    std::vector<Color> canvas_;
};

#endif
