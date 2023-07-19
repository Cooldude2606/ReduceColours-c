#include "./vector3.h"

// Divide all components of this vector by an integer value
void vector3_divide(Vector3 *v, int s) {
    v->x /= s; v->y /= s; v->z /= s; 
}

// Multiply all components of this vector by an integer value
void vector3_scale(Vector3 *v, int s) {
    v->x *= s; v->y *= s; v->z *= s; 
}

// Add each component of the other vector to the corrsponding component of this vector
void vector3_add(Vector3 *v, const Vector3 *o) {
    v->x += o->x; v->y += o->y; v->z += o->z;
}

// Add a multiple of each component of the other vector to the corrsponding component of this vector
void vector3_add_scaled(Vector3 *v, const Vector3 *o, int s) {
    v->x += o->x*s; v->y += o->y*s; v->z += o->z*s;
}

// Boolean comparison returning true if each component of this vector is equal to the corrsponding component of the other vector
int vector3_equals(const Vector3 *v, const Vector3 *o) {
    return v->x == o->x && v->y == o->y && v->z == o->z;
}

// Returns the squared distance between two vectors, avoiding square root for performance
int vector3_sqDistance(const Vector3 *v, const Vector3 *o) {
    int dx = v->x-o->x,  dy = v->y-o->y,  dz = v->z-o->z;
    return dx*dx + dy*dy + dz*dz;
}