#ifndef __H_color3
#define __H_color3

#include "vector3.h"
#include <limits.h>

// Small struct with no external reference which can contain 3 char values
typedef struct Colour3 {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Colour3;

// I tested macros vs functions and macros had a conclusive victory even under O3

// Create a new colour3 instance, there is no destroy method because there is no memory allocation
#define colour3_new(r, g, b) (Colour3){r, g, b}

// Generate a unique hash of this colour which fits within an int
#define colour3_hash(c) (c.r << 16) + (c.g << 8) + c.b

// Convert to and from a vector3 instance, useful when calculating average colours
#define colour3_fromVector3(v) (Colour3){v.x%(UCHAR_MAX+1), v.y%(UCHAR_MAX+1), v.z%(UCHAR_MAX+1)}
#define colour3_toVector3(c) (Vector3){c.r, c.g, c.b}

// Convert to and from a char buffer contained within the Image class
#define colour3_fromBuffer(bm, i) (Colour3){bm[i], bm[i+1], bm[i+2]}
#define colour3_toBuffer(c, bm, i) bm[i]=c.r; bm[i+1]=c.g; bm[i+2]=c.b;
#define colour3_toBufferWithAlpha(c, bm, i, a) bm[i]=c.r; bm[i+1]=c.g; bm[i+2]=c.b; bm[i+3]=a;

#endif // __H_color3