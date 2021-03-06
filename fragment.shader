#define NUMBER_OF_PATCHES vec2(1.0)
#define NOISE_SCALING 200.0
#define ZONE_SCALING 400.0

#define FOG_OPACITY_DISTANCE 2000.0
#define WATER_HEIGHT 0.0
#define GREEN_LIMIT_HEIGHT 50.0
#define SNOW_LINE_HEIGHT 150.0

#define FRACTAL_LACUNARITY 1.97866
#define MIN_RESOLUTION 0.01
#define MAX_RESOLUTION 10.0
#define MAX_DISTANCE 5656.854249
#define FRACTAL_DIMENSION 0.3

#define BLACK vec4(0.0, 0.0, 0.0, 1.0)
#define WHITE vec4(1.0, 1.0, 1.0, 1.0)
#define DARK_BROWN vec4(0.274509804, 0.176470588, 0.047058824, 1.0)
#define LIGHT_BROWN vec4(0.737254902, 0.415686275, 0.168627451, 1.0)
#define LIGHT_GRAY vec4(0.8, 0.8, 0.8, 1.0)
#define GRAY vec4(0.5, 0.5, 0.5, 1.0)
#define DARK_GRAY vec4(0.3, 0.3, 0.3, 1.0)
#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)
#define BLUE vec4(0.0, 0.0, 1.0, 1.0)
#define SKYCOLOR vec4(0.8, 0.8, 0.9, 1.0)

#define PI 3.141592653589793

#define ease_curve(a) (a*a*( 3.0 - 2.0*a ))
#define bias(a, b) (pow(a, log(b)/log(0.5)))

uniform vec3 camera_position;
uniform float time;
uniform mat3 xz_rotation_matrix;
uniform int isSkybox;
varying vec4 position;
varying vec3 point_normal;

float snoise(vec3);
float fractal_noise(vec3, float);

void main( void )
{
    if (isSkybox == 1)
    {
        gl_FragColor = SKYCOLOR;
        return;
    }

    float distance = length(position.xyz - camera_position);
    float noise = fractal_noise(position.xyz, distance);
    noise = noise/2.0 + 0.5;// Get the noise in the range 0..1

    vec4 color_top, color_bottom;
    float ypos = position.y + (2.0*noise - 1.0) * ZONE_SCALING;

    if (position.y <= WATER_HEIGHT)
    {
        vec4 color = mix(BLUE, SKYCOLOR, bias(0.3, fractal_noise(vec3(position.x, time, position.z), 1.0)/2.0 + 0.5));
        color_top = color;// Calculating it only once for speed
        color_bottom = color;
    }
    else if (ypos < GREEN_LIMIT_HEIGHT)
    {
        if (noise > 0.5)
        {
            color_top = mix(DARK_BROWN, GREEN, noise);
            color_bottom = mix(GREEN, LIGHT_BROWN, noise);
        }
        else
        {
            color_top = mix(DARK_BROWN, GRAY, noise);
            color_bottom = mix(DARK_GRAY, LIGHT_BROWN, noise);
        }
    }
    else if (ypos > SNOW_LINE_HEIGHT)
    {
        color_top = mix(WHITE, GRAY, noise);
        color_bottom = mix(DARK_GRAY, DARK_BROWN, noise);
    }
    else
    {
        color_top = mix(DARK_BROWN, GRAY, noise);
        color_bottom = mix(DARK_GRAY, LIGHT_BROWN, noise);
    }

    gl_FragColor = mix(color_top, color_bottom, noise);

    // Shading approx.
    float height = point_normal.y + (NOISE_SCALING)/2.0;
    gl_FragColor = max(gl_FragColor + mix(vec4(-0.2, -0.2, -0.2, 1.0), vec4(0.0, 0.0, 0.0, 1.0), height/(NOISE_SCALING)), 0.0 );

    // Fog
    float a = (distance + (NOISE_SCALING - ypos)*2.0) / FOG_OPACITY_DISTANCE;

    if (a > 1.0)
    {
        gl_FragColor = SKYCOLOR;
    }
    else
    {
        gl_FragColor = mix(gl_FragColor, SKYCOLOR, min(1.0, bias(0.3, a + noise)));
    }

}


float fractal_noise(vec3 point, float distance)
{
    float detail, count=0.0;

    if (distance <= MAX_DISTANCE)
    {
        detail = mix(MAX_RESOLUTION, MIN_RESOLUTION, (distance/MAX_DISTANCE));
    }
    else
    {
        detail = MIN_RESOLUTION;
    }
    float frequency, value = 0.0;

    for (frequency=MIN_RESOLUTION; frequency<=detail && frequency<MAX_RESOLUTION; frequency*=FRACTAL_LACUNARITY)
    {
        value += (snoise(point*frequency));
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

// BEGIN OF NOISE FUNCTION, NOT MADE BY ME

//
// Description : Array and textureless GLSL 2D/3D/4D simplex
// noise functions.
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

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2 C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i = floor(v + dot(v, C.yyy) );
  vec3 x0 = v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  // x0 = x0 - 0.0 + 0.0 * C.xxx;
  // x1 = x0 - i1 + 1.0 * C.xxx;
  // x2 = x0 - i2 + 2.0 * C.xxx;
  // x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ ); // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }

// END OF NOISE FUNCTION
