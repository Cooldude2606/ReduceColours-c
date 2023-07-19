#ifndef __H_images
#define __H_images

#include <stdlib.h>

// Contains the bitmap data for an image, all image types are converted into this
typedef struct Image {
    unsigned width;
    unsigned height;
    size_t bufferSize;
    unsigned char* buffer;
} Image;

// Create a new image instance with its internal buffer initialised to 0 and of the correct size
Image newImage(unsigned height, unsigned width);

// Resize the internal buffer of an image instance to be able to fit the new height and width, only ever increases the allocation
void resizeImage(Image* image, unsigned height, unsigned width);

// Destroy an image instance, deallocating its internal buffer
void destroyImage(Image* image);

// Read in any image from a file
Image readImage(const char* path);

// Write the image to file as a png
void writeImage(Image image, const char* path);

#endif // __H_images