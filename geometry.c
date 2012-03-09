#ifndef GEOMETRY_H
#define GEOMETRY_H

#define LEFT 0
#define TOP 1
#define RIGHT 2
#define BOTTOM 3
#define FAR 4

#define MEM_ALLOCATION_INCREMENT 1

typedef struct
{
    float x;
    float y;
    float z;
} point;

typedef struct
{
    point normal;
    point root;
} plane;

typedef struct
{
    point *array;
    int num_elements;
    int num_allocations;
} point_array;

typedef struct
{
    point_array *array;
    int num_elements;
    int num_allocations;
} point_2d_array;

void add_point_to_array(point_array *root, point object)
{
    // If I need to allocate some new memory
    if (root->num_elements == root->num_allocations)
    {
        root->num_allocations += MEM_ALLOCATION_INCREMENT;
        root->array = realloc(root->array, ((root->num_allocations)*sizeof(point)));
    }
    root->array[root->num_elements++] = object;
}
void free_point_array(point_array *root)
{
    int i;
    for (i=0; i<root->num_elements; i++)
    {
        free(root->array + i);
    }
    root->num_elements = 0;
    root->num_allocations = 0;
}

void allocate_new_point_array(point_2d_array *root)
{

    // If I need to allocate some new memory
    if (root->num_elements == root->num_allocations)
    {
        root->num_allocations += MEM_ALLOCATION_INCREMENT;
        root->array = realloc(root->array, ((root->num_allocations)*sizeof(point_array)));
    }
    // create a new array
    point_array object;
    object.num_elements = 0;
    object.num_allocations = 0;
    object.array = 0;
    // add it to the array list
    root->array[root->num_elements++] = object;
}
void free_point_2d_array(point_2d_array *root)
{
    int i;
    for (i=0; i<root->num_elements; i++)
    {
        // Free everything in the array
        free_point_array(root->array + i);
    }
    root->num_elements = 0;
    root->num_allocations = 0;
}

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
    c.x = a.y*b.z + a.z*b.y;
    c.y = a.x*b.z + a.z*b.x;
    c.x = a.x*b.y + a.y*b.x;
    return c;
}

int point_in_frustum(point query, point direction, float FOV, float depth, float half_width, float half_height)
{
    static point old_direction, up, right;
    static plane planes[5];
    static point tl, br, center;// tl = topleft vertex, br = bottomright vertex, nf = far side normal
    static int first_time;
    point tmp, tmp2;

    if ((old_direction.x != direction.x) || (old_direction.y != direction.y) || (old_direction.z != direction.z))
    {
        old_direction = direction;
        first_time = 1;
    }

    if (first_time==1)
    {
        // INITIALIZE EVERYTHING

        first_time = 0;

        // Get the primitives up and right
        tmp.x = 0;
        tmp.y = 0;
        tmp.z = 1;
        right = cross_product(tmp, direction);
        up = cross_product(direction, right);

        // Get the center of the frustum
        center = direction;
        center.x *= depth;
        center.y *= depth;
        center.z *= depth;

        // Get the offsets
        tmp = up;
        tmp.x *= half_height;
        tmp.y *= half_height;
        tmp.z *= half_height;
        tmp2 = right;
        tmp2.x *= half_width;
        tmp2.y *= half_width;
        tmp2.z *= half_width;

        // Get the two corners
        tl = center;// topleft
        tl.x += tmp.x - tmp2.x;
        tl.y += tmp.y - tmp2.y;
        tl.z += tmp.z - tmp2.z;
        br = center;// bottomright
        br.x += tmp2.x - tmp.x;
        br.y += tmp2.y - tmp.y;
        br.z += tmp2.z - tmp.z;

        // Set them to the corresponding things
        planes[0].root = tl;
        planes[1].root = tl;
        planes[2].root = br;
        planes[3].root = br;
        planes[4].root = center;

        // Get the normals
        // The far side
        planes[FAR].normal = direction;
        planes[FAR].normal.x *= -1;
        planes[FAR].normal.y *= -1;
        planes[FAR].normal.z *= -1;

        // The left side
        planes[LEFT].normal = cross_product(up, tl);
        normalize(&(planes[LEFT].normal));

        // The top side
        planes[TOP].normal = cross_product(right, tl);
        normalize(&(planes[TOP].normal));

        // The right side
        planes[RIGHT].normal = cross_product(up, br);
        normalize(&(planes[RIGHT].normal));

        // The bottom side
        planes[BOTTOM].normal = cross_product(right, br);
        normalize(&(planes[BOTTOM].normal));
    }

    int i;
    point newpoint;
    for (i=LEFT; i<5; i++)
    {
        newpoint = query;
        newpoint.x += planes[i].normal.x;
        newpoint.y += planes[i].normal.y;
        newpoint.z += planes[i].normal.z;
        if ((newpoint.x*newpoint.x + newpoint.y*newpoint.y + newpoint.z*newpoint.z) <= (query.x*query.x + query.y*query.y + query.z*query.z))
        {
            return 0;// the point is outside
        }
    }
    return 1;// the point is inside the frustum
}
#endif
