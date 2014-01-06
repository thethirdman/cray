#ifndef BITMAP_TEXTURE_HH
# define BITMAP_TEXTURE_HH

# include <utility>
# include <cv.h>
# include "material.hh"

/** A translation according to x (first) and y (second) coordinates. */
using BitmapTextureTranslation = std::pair<int, int>;

class BitmapTexture: public Material
{
public:
    /**
     * A BitmapTexture’s MaterialFunctor cannot be anything else than the
     * function returning the Color of the image at given coordinates.
     * Therefore we define it implicitely.
     */
    BitmapTexture (MaterialFunctor, Color& ambient, Color& diffuse, Color& specular, float brilliancy) = delete;
    BitmapTexture(const cv::Mat& texture,
                  Color& ambient, Color& diffuse, Color& specular, float brilliancy,
                  const BitmapTextureTranslation& = std::make_pair(0, 0));

private:
    const cv::Mat&                 texture_;
    const BitmapTextureTranslation translation_;
    const int x_max_;
    const int y_max_;
};

#endif // !BITMAP_TEXTURE_HH
