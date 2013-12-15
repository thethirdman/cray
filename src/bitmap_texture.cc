#include "bitmap_texture.hh"

/**
 * BitmapTexture constructor.
 * Because the MaterialFunctor (in Material) is built before the texture_, we
 * first define a dummy MaterialFunctor in the initialization list, then we
 * redefine it with its real value in the constructor body. That way, we can
 * safely use a lambda that captures the (const) pointer to texture_.
 * Sorry for "const" :D
 */
BitmapTexture::BitmapTexture(const cv::Mat& texture,
                             const PhongBundle& pb,
                             const BitmapTextureTranslation& translation)
    : Material(nullptr, pb),
      texture_(texture),
      translation_(translation),
      x_max_(texture.cols - 1),
      y_max_(texture.rows - 1)
{
    MaterialFunctor* mfp = const_cast<MaterialFunctor*>(&func_);
    delete mfp;
    mfp = new MaterialFunctor(
        [this](int x, int y) -> Color
        {
            // First apply the translation; then, if bigger than the image,
            // loop back.
            x = (x + this->translation_.first)  % (this->x_max_ + 1);
            y = (y + this->translation_.second) % (this->y_max_ + 1);

            // Effective pixel color retrieval, from
            // http://stackoverflow.com/a/7903042
            uint8_t* pixelPtr = (uint8_t*)this->texture_.data;
            int cn = this->texture_.channels();
            int blue  = pixelPtr[x*cn*this->texture_.cols + y*cn + 0]; // B
            int green = pixelPtr[x*cn*this->texture_.cols + y*cn + 1]; // G
            int red   = pixelPtr[x*cn*this->texture_.cols + y*cn + 2]; // R
            return Color(red, green, blue);
        }
    );
}
