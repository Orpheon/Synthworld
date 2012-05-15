#define PALEBLUE vec4(0.4, 0.4, 0.8, 1.0)
#define BLACK vec4(1.0, 1.0, 1.0, 1.0)
#define BLUE vec4(0.0, 0.0, 1.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

uniform vec3 camera_position;
varying vec4 position;

vec4 distance_based_color()
{
    float factor;
    vec4 color = GREEN;

    factor = length(position.xyz)/1000.0;
    color.xyz -= vec3(0.0, factor, -factor);

    return color;
}

void main()
{
    gl_FragColor = distance_based_color();
}
