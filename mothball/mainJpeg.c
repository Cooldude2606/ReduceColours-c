#include "../libs/lodepng/lodepng.h"
#include "../libs/jpeg/src/djpg.h"
#include "../src/priorityQueue.h"
#include "../src/hashMap.h"
#include "../src/colour3.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Image {
    int width;
    int height;
    int bufferSize;
    unsigned char* buffer;
} Image;

typedef struct Node {
    Colour3 color;
    Colour3 replacement;
    int frequency;
} Node;

int getFileSize(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long pos = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return pos;
}

Image readJPEG(char* path) {
    int XDiv8, YDiv8;
    unsigned width, height;
    unsigned char* image;

    // https://github.com/akumrao/jpeg/blob/master/src/djpg.c#L1441
    FILE* fp = fopen(path, "rb");
    int fileSize = getFileSize(fp);
    unsigned char* buffer = malloc(fileSize);
    fileSize = (int) fread(buffer, fileSize, 1, fp);
    fclose(fp);

    DecodeJpgFileData(buffer, fileSize, &image, &width, &height, &XDiv8, &YDiv8);
    if(image == NULL) printf("error: failed to decode jpeg\n");

    // Convert from rgb to rgba so it can be written to a png at the end
    image = realloc(image, width*height*4);
    for (int i = width*height; i >= 0; i--) {
        image[i*4+3] = 255; image[i*4+2] = image[i*3+2]; image[i*4+1] = image[i*3+1]; image[i*4] = image[i*3]; 
    }

    Image rtn = { width, height, width*height*4, image };
    return rtn;
}

Image readPNG(char* path) {
    unsigned width, height;
    unsigned char* image;

    unsigned error = lodepng_decode32_file(&image, &width, &height, path);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    Image rtn = { width, height, width*height*4, image };
    return rtn;
}

void writePNG(Image image, char* path) {
    unsigned error = lodepng_encode32_file(path, image.buffer, image.width, image.height);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

void scanImage(Image image, HashMap* colours) {
    for (int i = 0; i < image.height*image.width; i++) {
        Colour3 color = colour3_fromBuffer(image.buffer+(i*4));
        int key = colour3_hash(color);

        Node* node = hashMap_getValue(colours, key);
        if (node) {
            node->frequency++;
        } else {
            node = malloc(sizeof(Node));
            node->color = color; node->replacement = color; node->frequency = 0;
            hashMap_setValue(colours, key, node);
        }
    }   
}

int main(int argc, char** argv) {
    // Check that exactly two arguments were given
    if (argc != 3) {
        printf("error: wrong number of arguments, 2 expect got %i\n", argc);
        return 1;
    }

    char* inputPath = argv[1];
    int desiredColours = atoi(argv[2]);
    // Check the second argument is an int greater than 0
    if (desiredColours <= 0) {
        printf("error invalid argument 2: must be integer greater than 0\n");
        return 1;
    }

    // Get the file type so we can read in the image correctly
    Image image;
    char* fileType = strrchr(inputPath, '.')+1;
    printf("File type: %s\n", fileType);
    if (strcmp(fileType, "png") == 0) {
        image = readPNG(inputPath);
    } else if (strcmp(fileType, "jpeg") == 0 || strcmp(fileType, "jpg") == 0) {
        image = readJPEG(inputPath);
    } else {
        printf("error invalid argument 1: file type '%s' can not be decoded\n", fileType);
        return 1; 
    }

    // Read in the image and set up the data structures
    HashMap* colours = hashMap_preAlloc(image.width*image.height/9);

    // Read all the colours in the image
    scanImage(image, colours);

    // Find the max frequency in the image
    int maxFrequency = 0;
    for (int i = 0; i < colours->max; i++) {
        Node* node = colours->map[i].value;
        if (node && node->frequency > maxFrequency) {
            maxFrequency = node->frequency;
        }
    }

    // Replace the colours in the image with a grey scale frequency
    double logMaxFrequency = log(maxFrequency);
    for (int i = 0; i < image.height*image.width; i++) {
        Colour3 color = colour3_fromBuffer(image.buffer+(i*4));
        int key = colour3_hash(color);
        Node* node = hashMap_getValue(colours, key);
        
        int value = (int)(255 * log(node->frequency) - logMaxFrequency);
        Colour3 replacement = colour3_new(value, value, value);
        colour3_toBuffer(replacement, image.buffer+(i*4));
    }

    // Copy the input path so it can be modified
    char outputPath[256];
    strcpy(outputPath, inputPath);

    // Edit the file name to end in "_out"
    char* lastIndexOf = strrchr(outputPath, '.');
    strcpy(lastIndexOf, "_out.png");
    writePNG(image, outputPath);

    return 0;
}