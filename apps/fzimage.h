
#ifndef FZIMAGE_H
#define FZIMAGE_H

#include "fzrefcount.h"
#include "fzinputstream.h"

/**
 * Static, application accesible images.
 * FZImage contains a contiguous area of memory that is readable/writable by the application,
 * and represents an image.
 */
class FZImage : public FZRefCounted {
	public:
	enum Format {
		rgba32, rgb32, rgb24, dual16, mono16, mono8
	};

	private:
	char* data;
	unsigned int* clut;
	unsigned int clutSize;

	unsigned int width, height;
	Format imageFormat;

	protected:
	FZImage(unsigned int w, unsigned int h, Format f);
	~FZImage();

	public:
	/**
	 * Get the image logical format.
	 */
	Format getFormat() const;
	/**
	 * Get the image bytes per pixel.
	 */
	unsigned int getBytesPerPixel() const;
	/**
	 * Get the image logical dimensions.
	 */
	void getDimensions(unsigned int& w, unsigned int &h) const;
	/**
	 * Get a pointer to the image data.
	 */
	char* getData();
	/**
	 * Get a pointer to the color table data; CLUT are always 32bpp RGBA
	 */
	unsigned int* getCLUT();
	/**
	 * Get color table size
	 */
	unsigned int getCLUTSize();

	void swizzle(int sx, int sy);

	/**
	 * Create an image from a PNG file.
	 */
	static FZImage* createFromPNG(FZInputStream* in);
	/**
	 * Create an image from a JPEG file.
	 */
	//static FZImage* createFromJPEG(FZInputStream* in);
	/**
	 * Create an empty image.
	 * An image is created with the specified parameters, and its data is
	 * initialized to 0
	 */
	static FZImage* createEmpty(unsigned int w, unsigned int h, unsigned int cl, Format f);
	/**
	 * Create a rgb24 format image from a rgb32 format image.
	 */
	static FZImage* createRGB24FromRGB32(FZImage* from);
	/**
	 * Create an N 2-exponent lower resolution image.
	 * Requires a power of two image size on both axis. Returns a new image with N 2-exponent
	 * less width and height.
	 */
	//static FZImage* createMipMap(FZImage* from, int level);
};

#endif

