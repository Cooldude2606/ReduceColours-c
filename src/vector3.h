#ifndef __H_vector3
#define __H_vector3

// Small struct with no external reference which can contain 3 int values
typedef struct Vector3 {
    int x;
    int y;
    int z;
} Vector3;

// Create a new vector3 instance, there is no destroy method because there is no memory allocation
#define vector3_new(x, y, z) (Vector3){x, y, z}

// I tested macros vs functions and with O0 macros were better
// but under O3 it was inconclusive and so functions were used for readability purposes

/* #define vector3_divide(v,s) v.x/=s; v.y/=s; v.z/=s
#define vector3_scale(v,s) v.x*=s; v.y*=s; v.z*=s

#define vector3_add(v,o) v.x+=o.x; v.y+=o.y; v.z+=o.z
#define vector3_add_scaled(v,o,s) v.x+=o.x*s; v.y+=o.y*s; v.z+=o.z*s

#define vector3_equals(v,o) v.x==o->x && v.y==o->y && v.z==o->z */

// Divide / Multiply all components of a vector by an integer value
void vector3_divide(Vector3 *v, int s);
void vector3_scale(Vector3 *v, int s);

// Add / Subtract multiples of one vector from another
void vector3_add(Vector3 *v, const Vector3 *o);
void vector3_add_scaled(Vector3 *v, const Vector3 *o, int s);

// Boolean comparison for if one vector is equal to another vector
int vector3_equals(const Vector3 *v, const Vector3 *o);

// Get the squared distance between two vectors, no use of sqrt for performance
int vector3_sqDistance(const Vector3 *v, const Vector3 *o);

#endif // __H_vector3