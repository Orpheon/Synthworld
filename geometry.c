#include "geometry.h"

void normalize(point *a)
{
    float length;
    length = sqrt((*a).x*(*a).x + (*a).y*(*a).y + (*a).z*(*a).z);
    a->x /= length;
    a->y /= length;
    a->z /= length;
}

point cross_product(point a, point b)
{
    point c;
    c.x = a.y*b.z - a.z*b.y;
    c.y = a.x*b.z - a.z*b.x;
    c.x = a.x*b.y - a.y*b.x;
    return c;
}
