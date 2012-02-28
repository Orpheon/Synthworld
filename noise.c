#include "geometry.c"
#include "random_numbers.c"
#include <stdio.h>
#include <string.h>

#define simplefloor(a) ((double)((long)(a)-((a)<0.0)))
#define floor(a) ((tempdoub=(a))<0.0?((long)tempdoub)-1L:(long)tempdoub)
#define dot(a,b) (a.x*b.x + a.y*b.y + a.z*b.z)
#define drop(a) (3 * (a*a) - 2 * (fabs(a) * a * a))
#define lerp(a, b, t) (a + t*(b - a))
#define print_point(a) (printf("(  %f  |  %f  |  %f  )", a.x, a.y, a.z))

#define LACUNARITY 2.0f

float noise(point query)
{
    int i;
    point intpoint, grad, vec_to_point;
    float influences[8], weight_x, weight_y, weight_z, a, b, c, d, value, tmp[3];
    double tempdoub;

    i = 0;
    for (intpoint.z=simplefloor(query.z); intpoint.z<=floor(query.z+1); intpoint.z++)
    {
        for (intpoint.y=simplefloor(query.y); intpoint.y<=floor(query.y+1); intpoint.y++)
        {
            for (intpoint.x=simplefloor(query.x); intpoint.x<=floor(query.x+1); intpoint.x++)
            {
                // Get a random vector
                memcpy(tmp, rand_vectors[fold(intpoint)], sizeof(float)*3);
                grad.x = tmp[0];
                grad.y = tmp[1];
                grad.z = tmp[2];

                // Get the vector from the intpoint to the real point
                vec_to_point.x = query.x-intpoint.x;
                vec_to_point.y = query.y-intpoint.y;
                vec_to_point.z = query.z-intpoint.z;

                // Get the dot product, and normalize it
                value = dot(grad, vec_to_point);
                value /= 1.7320508075688772; // sqrt(3)

                // Add it to the list
                influences[i++] = value;
            }
        }
    }

    // Get the interpolation weights
    value = query.x-simplefloor(query.x);
    weight_x = drop(value);
    value = query.y-simplefloor(query.y);
    weight_y = drop(value);
    value = query.z-simplefloor(query.z);
    weight_z = drop(value);

    // Interpolate using those weights
    a = lerp(influences[0], influences[1], weight_x);
    b = lerp(influences[2], influences[3], weight_x);
    c = lerp(influences[4], influences[5], weight_x);
    d = lerp(influences[6], influences[7], weight_x);

	a = lerp(a, b, weight_y);
	b = lerp(c, d, weight_y);

	value = lerp(a, b, weight_z);
	return value;
}

int fold(point A)
{
    return perm_table[ (perm_table[ (perm_table[(int)A.x % 254] + (int)A.y) % 254] + (int)A.z) % 254];
}

float fBm(point query, float H, float octaves)
{
    static int first_time = 1;
    static float *exponent_array;

    int i;
    float frequency, value;

    if (first_time)
    {
        frequency = 1.0f;
        exponent_array = (float *)malloc((octaves+1)*sizeof(float));
        for (i=0; i<=octaves; i++)
        {
            exponent_array[i] = pow(frequency, -H);
            frequency *= LACUNARITY;
        }
    }

    frequency = 1.0f;
    value = 1.0f;

    for (i=0; i<octaves; i++)
    {
        value += noise(query) * exponent_array[i];
        query.x *= LACUNARITY;
        query.y *= LACUNARITY;
        query.z *= LACUNARITY;
    }

    float remainder;
    remainder = octaves - (int)octaves;
    if (remainder)
    {
        value += remainder * noise(query) * exponent_array[i];
    }

    return value;
}

float multifractal_1(point query, float H, float octaves, float offset)
{
    static int first_time = 1;
    static float *exponent_array;

    int i;
    float frequency, value;

    if (first_time)
    {
        frequency = 1.0f;
        exponent_array = (float *)malloc((octaves+1)*sizeof(float));
        for (i=0; i<=octaves; i++)
        {
            exponent_array[i] = pow(frequency, H);
            frequency *= LACUNARITY;
        }
    }

    frequency = 1.0f;
    value = 1.0f;

    for (i=0; i<octaves; i++)
    {
        value *= (noise(query) * offset * exponent_array[i]);
        query.x *= LACUNARITY;
        query.y *= LACUNARITY;
        query.z *= LACUNARITY;
    }

    float remainder;
    remainder = octaves - (int)octaves;
    if (remainder)
    {
        value += remainder * noise(query) * exponent_array[i];
    }

    return value;
}
