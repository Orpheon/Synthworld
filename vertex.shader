#define NOISE_SCALING 1000.0
#define NOISE_FREQUENCY 0.001
#define VIEW_DISTANCE 100000.0

#define WATER_HEIGHT -camera_position.y

#ifndef PI
#define PI 3.141592654
#endif

#define FRACTAL_LACUNARITY 1.97869
#define MIN_RESOLUTION 0.01
#define MAX_RESOLUTION 10.0
#define MAX_DISTANCE 5656.854249
#define FRACTAL_DIMENSION 5.0


uniform vec3 camera_position;
uniform vec3 camera_direction;
uniform float time;
uniform int isSkybox;
varying vec4 position;
varying vec3 point_normal;

float snoise(vec2);
float fractal_noise(vec2);

void main()
{
    if (isSkybox == 1)
    {
        gl_Position = ftransform();
        position = gl_Position;
        return;
    }

    vec4 newpos;
    newpos = gl_Vertex;

    // Calculating the normal of this vertex
    vec2 point = (newpos.xz + camera_position.xz) * NOISE_FREQUENCY;
//    vec3 point1 = vec3(point.x, 0.0, point.y - 1.0),
//         point2 = vec3(point.x - 0.5, 0.0, point.y - 0.866025404), // <-- Equilateral triangle; 0.866025404 = cos(PI/6.0)
//         point3 = vec3(point.x + 0.5, 0.0, point.y - 0.866025404);

    newpos.y = fractal_noise(point) * NOISE_SCALING - camera_position.y;

//    point1.y = fractal_noise(point1.xz);
//    point2.y = fractal_noise(point2.xz);
//    point3.y = fractal_noise(point3.xz);
//
//    point_normal = normalize( cross( point2-point1, point3-point1 ) );// Used for lighting in the frag shader

    if (newpos.y <= WATER_HEIGHT)
    {
        newpos.y = WATER_HEIGHT;
    }

    gl_Position = gl_ModelViewProjectionMatrix * newpos;

    position = newpos;
    position.xyz += camera_position;
}

float fractal_noise(vec2 point)
{
    float distance = length(point - camera_position.xz);
    float detail, count=0.0, offset;

    if (distance <= MAX_DISTANCE)
    {
        detail = mix(MAX_RESOLUTION, MIN_RESOLUTION, (distance/MAX_DISTANCE));
    }
    else
    {
        detail = MIN_RESOLUTION;
    }
    float frequency, value = 1.0;

    for (frequency=MIN_RESOLUTION; frequency<=detail && frequency<MAX_RESOLUTION; frequency*=FRACTAL_LACUNARITY)
    {
        value += snoise(point*frequency);
        count++;
    }

    if (detail > frequency)
    {
        float remainder = detail - frequency;
        value += snoise(point) * remainder;
//        count += remainder;
    }

    value /= count;

    return value;
}

// START OF NOISE FUNCTION

//
// Description : Array and textureless GLSL 2D simplex noise function.
// Author : Ian McEwan, Ashima Arts.
// Maintainer : ijm
// Lastmod : 20110822 (ijm)
// License : Copyright (C) 2011 Ashima Arts. All rights reserved.
// Distributed under the MIT License. See LICENSE file.
// https://github.com/ashima/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187, // (3.0-sqrt(3.0))/6.0
                      0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626, // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i = floor(v + dot(v, C.yy) );
  vec2 x0 = v - i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
+ i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x = a0.x * x0.x + h.x * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

// END OF NOISE FUNCTION
