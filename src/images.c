#include "./images.h"
#include <stdio.h>

#include "../libs/lodepng/lodepng.h"
#include "../libs/nanojpeg/nanojpeg.h"

// Read a JPEG image from the given path
static Image readJPEG(const char* path) {    
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        printf("error: file not found\n");
        exit(1);
    }

    // Get the file size and allocate an input buffer
    fseek(fp, 0, SEEK_END);
    int fileSize = (int) ftell(fp);
    unsigned char* buffer = malloc(fileSize);
    fseek(fp, 0, SEEK_SET);

    // Read the file into the buffer
    fileSize = (int) fread(buffer, 1, fileSize, fp);
    fclose(fp);

    // Decode the jpeg
    njInit();
    unsigned error = njDecode(buffer, fileSize);
    if (error) {
        switch(error) {
            case NJ_NO_JPEG: printf("error %u: Not a jpeg image\n", error); break;
            case NJ_UNSUPPORTED: printf("error %u: Unsupported format\n", error); break;
            case NJ_OUT_OF_MEM: printf("error %u: Out of memory\n", error); break;
            case NJ_INTERNAL_ERR: printf("error %u: Internal error with nanojpeg\n", error); break;
            case NJ_SYNTAX_ERROR: printf("error %u: Syntax error with jpeg file\n", error); break;
            default: printf("error %u: Unknown Error\n", error); break;
        }
        exit(1);
    }

    // Get the details of the image
    unsigned width = njGetWidth(), height = njGetHeight();
    unsigned char* image = njGetImage();
    free(buffer);
    //njDone(); // This call is not required due to the realloc below

    // Convert from rgb to rgba so it can be written to a png at the end
    image = realloc(image, width*height*4);
    for (int i = width*height; i >= 0; i--) {
        image[i*4+3] = 255; image[i*4+2] = image[i*3+2]; image[i*4+1] = image[i*3+1]; image[i*4] = image[i*3]; 
    }

    return (Image){ width, height, width*height*4, image };
}

// Read a PNG image from the given path
static Image readPNG(const char* path) {
    unsigned width, height;
    unsigned char* image;

    unsigned error = lodepng_decode32_file(&image, &width, &height, path);
    if(error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        exit(1);
    }

    return (Image){ width, height, width*height*4, image };
}

// Write a PNG image to the given path
static void writePNG(Image image, const char* path) {
    unsigned error = lodepng_encode32_file(path, image.buffer, image.width, image.height);
    if(error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        exit(1);
    }
}

// Create a new image with an allocated buffer large enough to store the desired size
Image newImage(unsigned height, unsigned width) {
    size_t bufferSize = height*width*4;
    unsigned char* buffer = calloc(1, bufferSize);
    return (Image){width, height, bufferSize, buffer};
}

// Resize the image making sure the internal buffer is large enough to store it
void resizeImage(Image* image, unsigned height, unsigned width) {
    image->height = height; image->width = width;
    size_t newBufferSize = height*width*4;
    if (newBufferSize > image->bufferSize) {
        image->buffer = realloc(image->buffer, newBufferSize);
        image->bufferSize = newBufferSize;
    }
}

// Destroy an image, deallocating its internal buffer
void destroyImage(Image* image) {
    free(image->buffer);
    image->buffer = NULL;
    image->bufferSize = 0;
}

// Read either a JPEG or PNG, buffer size is 0 on error
Image readImage(const char* path) {
    char* fileType = strrchr(path, '.')+1;
    if (strcmp(fileType, "png") == 0) {
        return readPNG(path);
    } else if (strcmp(fileType, "jpeg") == 0 || strcmp(fileType, "jpg") == 0) {
        return readJPEG(path);
    } else {
        return (Image){0,0,0,NULL}; 
    }
}

// Write a PNG to the given path
void writeImage(Image image, const char* path) {
    writePNG(image, path);
}