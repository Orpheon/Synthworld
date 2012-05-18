#ifndef GEOMETRY_H
#define GEOMETRY_H

typedef struct
{
    float x;
    float y;
    float z;
} point;

void normalize(point *a);

point cross_product(point a, point b);

#endif

