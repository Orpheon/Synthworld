/* TODO:
1. Fix x/z rotations --DONE--
2. If necessary, fix y too --BUGGY, BUT WORKING--
3. Add shading and make terrain white --DONE--
5. Move to fBm --DONE--
6. Add bump-mapping --DONE--
7. Use multi-fractals
8. Add texture generation
*/
#include <GL/glfw.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "geometry.c"
#include "random_numbers.c"
#include "noise.c"

#define length_of_vector(a) sqrt(a.x*a.x + a.y*a.y + a.z*a.z)
#define SIGN(a) (a/abs(a))

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1000
#define MAP_WIDTH 400
#define MAP_LENGTH 400
#define BUMPMAP_SIZE 0.1
#define NOISE_SCALE 400
#define NOISE_RESOLUTION 500
#define NOISE_DIMENSION 0.5
#define NOISE_OCTAVE_NUMBER 5
#define MULTIFRACTAL_OFFSET 0.3
#define SEA_LEVEL -10
#define CAMERA_SPEED 1
#define TURNING_SPEED 80
#ifndef PI
#define PI 3.141592654f
#endif

void generate_heightmap(float (**terrain), point base_point, point (**bumpmap));
void render();
int updatemap(float **terrain, point camera, point direction);
void updateview(point camera, point direction);

int main(int argc, char **argv)
{
    initialize_random_numbers();

    int i, j;
    float length, mouse_x, mouse_y;

    point camera, direction, old_camera_pos, tmp;
    // The theoretical camera position
    camera.x = 0.0;
    camera.y = 15.0f;
    camera.z = 0.0;

    // The camera position a frame ago (used for detecting and calculating movement)
    old_camera_pos = camera;

    // A unit vector, which is the direction we're pointing at
    direction.x = 0.25f;
    direction.y = 0.0f;
    direction.z = -0.25;

    // Normalize direction
    length = length_of_vector(direction);
    direction.x /= length;
    direction.y /= length;
    direction.z /= length;

    float **terrain;
    terrain = (float**) malloc(MAP_WIDTH*sizeof(float*));
    for (i = 0; i < MAP_WIDTH; i++)
    {
        terrain[i] = (float*) malloc(MAP_LENGTH*sizeof(float));
    }
    point **bumpmap;
    bumpmap = (point**) malloc(MAP_WIDTH*sizeof(point*));
    for (i = 0; i < MAP_WIDTH; i++)
    {
        bumpmap[i] = (point*) malloc(MAP_LENGTH*sizeof(point));
    }

    tmp.x = (int)camera.x;
    tmp.y = (int)camera.y;
    tmp.z = 0;

    generate_heightmap(terrain, tmp, bumpmap);

    int running = GL_TRUE;
    // Initialize GLFW
    if( !glfwInit() )
    {
        exit( EXIT_FAILURE );
    }

    // Open an OpenGL window
    if( !glfwOpenWindow(SCREEN_WIDTH,SCREEN_HEIGHT, 8,8,8,8,8,8, GLFW_WINDOW ) )
    {
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    int x, z, width, height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get window size
    glfwGetWindowSize( &width, &height );

    // Make sure that height is non-zero to avoid division by zero
    height = height < 1 ? 1 : height;

    // Reset the mouse in the middle
    glfwSetMousePos(width/2, height/2);

    //Set up the lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    //Ambient lighting
    GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    GLfloat lightColor[] = {0.6f, 0.6f, 0.6f, 1.0f};
    //Diffuse (non-shiny) light component
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    //Specular (shiny) light component
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);

    glViewport (0, 0, (GLsizei)width, (GLsizei)height); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)width / (GLfloat)height, 1.0, 1000.0); //set the perspective (angle of sight, width, height, depth)
    glMatrixMode (GL_MODELVIEW); //set the matrix back to model

    glFlush();

    // Main loop
    while( running )
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);// Fill the screen with black
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Reset both buffers

        glPushMatrix();// Backup the current coordinate system

        // Handle mouse input
        glfwGetMousePos(&i, &j);

        // Turn them into floats for later
        mouse_x = i;
        mouse_y = j;

        // Get the positions relative to the center of the screen, and normalize them
        mouse_x = (mouse_x - width/2)/width;
        mouse_y = (mouse_y - height/2)/height;

        // Change the direction vector along the x/z plane
        tmp.x = -direction.z;
        tmp.z = direction.x;
        tmp.y = 0.0;
        length = length_of_vector(tmp);
        tmp.x *= mouse_x/length;
        tmp.z *= mouse_x/length;
        direction.x += tmp.x;
        direction.z += tmp.z;
        // Change the direction vector along the y plane
        direction.y += sin(PI*TURNING_SPEED*mouse_y/360);
        //printf("\n %f %f %f", mouse_y, direction.y, direction.z);

        // Normalize the direction vector again
        length = length_of_vector(direction);
        direction.x /= length;
        direction.y /= length;
        direction.z /= length;

        // Reset the mouse in the middle if K isn't pressed (to allow the moving around of the window)
        if (glfwGetKey('K') != GLFW_PRESS)
        {
            glfwSetMousePos(width/2, height/2);
        }


        // Handle keyboard input
        if (glfwGetKey('W') == GLFW_PRESS)
        {
            camera.x += CAMERA_SPEED*direction.x;
            camera.y -= CAMERA_SPEED*direction.y;
            camera.z += CAMERA_SPEED*direction.z;
        }
        else if (glfwGetKey('S') == GLFW_PRESS)
        {
            camera.x -= CAMERA_SPEED*direction.x;
            camera.y += CAMERA_SPEED*direction.y;
            camera.z -= CAMERA_SPEED*direction.z;
        }

        if (glfwGetKey('A') == GLFW_PRESS)
        {
            camera.x += CAMERA_SPEED*direction.z;
            camera.z -= CAMERA_SPEED*direction.x;
        }
        else if (glfwGetKey('D') == GLFW_PRESS)
        {
            camera.x -= CAMERA_SPEED*direction.z;
            camera.z += CAMERA_SPEED*direction.x;
        }

        //updatemap(terrain, camera, old_camera_pos);

        updateview(camera, direction);

        // Render
        render(terrain, bumpmap, camera, direction);

        glPopMatrix();// Restore the original coordinate system

        // Check if ESC key was pressed or window was closed
        running = !glfwGetKey( GLFW_KEY_ESC ) &&
        glfwGetWindowParam( GLFW_OPENED );

        old_camera_pos = camera;
    }
    // free the memory used by the terrain
    for(i=0; i<MAP_WIDTH; i++)
    {
        free(terrain[i]);
    }
    free(terrain);
    // Close window and terminate GLFW
    glfwTerminate();
    // Exit program
    exit( EXIT_SUCCESS );
}

void generate_heightmap(float (**terrain), point base_point, point (**bumpmap))
{
    int x, y;
    float tmp[3], maxvalue, a, b;
    point tmpPoint, bump_vector;
    FILE *fp;
    fp = fopen("image.map", "w");

    // We don't need the z, so just set it to 0 and ignore
    tmpPoint.z = 0;

    // Multifractal_1 range testing
    maxvalue = -1;
    for (tmpPoint.x=0; tmpPoint.x<=3; tmpPoint.x+=0.008)
    {
        for (tmpPoint.y=0; tmpPoint.y<=3; tmpPoint.y+=0.008)
        {
            tmp[0] = multifractal_1(tmpPoint, NOISE_DIMENSION, NOISE_OCTAVE_NUMBER, MULTIFRACTAL_OFFSET);
            if (tmp[0] > maxvalue)
            {
                maxvalue = tmp[0];
            }
        }
    }

    for(x=-(MAP_WIDTH/2); x<((MAP_WIDTH/2)); x++)
    {
        // /10 because positions are always ints here, but I don't want white noise. So scale up.
        tmpPoint.x = (x+(MAP_WIDTH/2.0f)+base_point.x)/NOISE_RESOLUTION;
        for(y=-(MAP_LENGTH/2); y<((MAP_LENGTH/2)); y++)
        {
            tmpPoint.y = (y+(MAP_LENGTH/2.0f)+base_point.y)/NOISE_RESOLUTION;
            //terrain[x+(MAP_WIDTH/2)][y+(MAP_LENGTH/2)] = fBm(tmpPoint, NOISE_DIMENSION, NOISE_OCTAVE_NUMBER)*NOISE_SCALE;
            terrain[x+(MAP_WIDTH/2)][y+(MAP_LENGTH/2)] = multifractal_1(tmpPoint, NOISE_DIMENSION, NOISE_OCTAVE_NUMBER, MULTIFRACTAL_OFFSET)*NOISE_SCALE/maxvalue;
            //terrain[x+(MAP_WIDTH/2)][y+(MAP_LENGTH/2)] = multifractal_altitude(tmpPoint, NOISE_DIMENSION, NOISE_OCTAVE_NUMBER)*NOISE_SCALE;

            bump_vector.x = tmpPoint.x*NOISE_RESOLUTION;
            bump_vector.y = tmpPoint.y*NOISE_RESOLUTION;
            bump_vector.z = tmpPoint.z*NOISE_RESOLUTION;
            memcpy(tmp, rand_vectors[fold(bump_vector)], sizeof(float)*3);
            bump_vector.x = tmp[0]*BUMPMAP_SIZE;
            bump_vector.y = tmp[1]*BUMPMAP_SIZE;
            bump_vector.z = tmp[2]*BUMPMAP_SIZE;
            bumpmap[x+(MAP_WIDTH/2)][y+(MAP_LENGTH/2)] = bump_vector;
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

void render(float **terrain, point **bumpmap, point camera, point direction)
{
    int x, z;
    float height[4], length;
    point normal, bump_normal;

    GLfloat lightPos[] = {0.0f, 1.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_COLOR_MATERIAL); //Required for the glMaterial calls to work

    for(x=0; x<MAP_WIDTH-1; x++)
    {
        glBegin(GL_QUAD_STRIP);

        for(z=0; z<(MAP_LENGTH-1); z++)
        {
            // Get the heights from the terrain map
            height[0] = terrain[x][z];
            height[1] = terrain[x+1][z];
            height[2] = terrain[x][z+1];
            height[3] = terrain[x+1][z+1];

            // Calculate the normal of this plane
            normal.x = height[1]-height[0];
            normal.y = 1;
            normal.z = height[2]-height[0];
            normalize(&normal);

            if (height[0] > SEA_LEVEL)
            {
                glColor3f(0.394117647, 0.3, 0.0);// brown
            }
            else
            {
                glColor3f(0.0, 0.0, 0.31372549); // dark blue
                height[0] = SEA_LEVEL;
                height[1] = SEA_LEVEL;
                height[2] = SEA_LEVEL;
                height[3] = SEA_LEVEL;
                normal.x = 0.0;
                normal.y = 1.0;
                normal.z = 0.0;
            }
            bump_normal.x = normal.x + (bumpmap[x][z]).x;
            bump_normal.y = normal.y + (bumpmap[x][z]).y;
            bump_normal.z = normal.z + (bumpmap[x][z]).z;
            normalize(&bump_normal);
            glNormal3f(bump_normal.x, bump_normal.y, bump_normal.z);
            glVertex3f(x, height[0], -(z));

            bump_normal.x = normal.x + (bumpmap[x+1][z]).x;
            bump_normal.y = normal.y + (bumpmap[x+1][z]).y;
            bump_normal.z = normal.z + (bumpmap[x+1][z]).z;
            normalize(&bump_normal);
            glNormal3f(bump_normal.x, bump_normal.y, bump_normal.z);
            glVertex3f(x+1, height[1], -(z));

            bump_normal.x = normal.x + (bumpmap[x][z+1]).x;
            bump_normal.y = normal.y + (bumpmap[x][z+1]).y;
            bump_normal.z = normal.z + (bumpmap[x][z+1]).z;
            normalize(&bump_normal);
            glNormal3f(bump_normal.x, bump_normal.y, bump_normal.z);
            glVertex3f(x, height[2], -(z+1));

            bump_normal.x = normal.x + (bumpmap[x+1][z+1]).x;
            bump_normal.y = normal.y + (bumpmap[x+1][z+1]).y;
            bump_normal.z = normal.z + (bumpmap[x+1][z+1]).z;
            normalize(&bump_normal);
            glNormal3f(bump_normal.x, bump_normal.y, bump_normal.z);
            glVertex3f(x+1, height[3], -(z+1));
        }
        glEnd();
    }

    // Swap front and back rendering buffers
    glfwSwapBuffers();
}

int updatemap(float **terrain, point camera, point old_cam)
{
    if( (int)old_cam.x == (int)camera.x )
    {
        if( (int)old_cam.y == (int)camera.y )
        {
            if( (int)old_cam.z == (int)camera.z )
            {
                // Nothing has changed. Return
                return;
            }
        }
    }

    // We've moved; recalculate the terrain map.
    point tmp;
    tmp.x = (int)camera.x;
    tmp.y = (int)camera.y;
    tmp.z = 0;
    //generate_heightmap(terrain, tmp);
}

void updateview(point camera, point direction)
{
    float length, xrot, yrot;
    length = sqrt(1-(direction.y*direction.y));
    xrot = asin(direction.x/length);
    yrot = asin(direction.y);

    xrot *= 180/PI;
    yrot *= 180/PI;

    //printf("\n---\n %f %f %f\n%f %f", direction.x, direction.y, direction.z, xrot, yrot);

    if (direction.z > 0)
    {
        xrot = 180-xrot;
    }

    glRotatef(xrot, 0.0, 1.0, 0.0);
    glRotatef(yrot, -direction.z, 0.0, direction.x);
    glTranslated(-camera.x, -camera.y, -camera.z);
}
