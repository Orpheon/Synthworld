#ifndef GEOMETRY_H
#define GEOMETRY_H
typedef struct
{
    float x;
    float y;
    float z;
} point;

void normalize(point *a)
{
    float length;
    length = sqrt((*a).x*(*a).x + (*a).y*(*a).y + (*a).z*(*a).z);
    (*a).x /= length;
    (*a).y /= length;
    (*a).z /= length;
}
#endif
