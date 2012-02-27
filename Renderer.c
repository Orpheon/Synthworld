/* TODO:
1. Fix x/z rotations --DONE--
2. If necessary, fix y too --BUGGY, BUT WORKING--
3. Add shading and make terrain white --DONE--
5. Move to fBm
6. Use multi-fractals
7. Add texture generation
*/
#include <GL/glfw.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "geometry.c"
#include "noise.c"

#define length_of_vector(a) sqrt(a.x*a.x + a.y*a.y + a.z*a.z)
#define SIGN(a) (a/abs(a))

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1000
#define MAP_WIDTH 400
#define MAP_LENGTH 400
#define NOISE_SCALE 40
#define NOISE_RESOLUTION 400
#define NOISE_DIMENSION 0.3
#define NOISE_OCTAVE_NUMBER 5
#define CAMERA_SPEED 1
#define TURNING_SPEED 80
#ifndef PI
#define PI 3.141592654f
#endif

void generate_heightmap(float (**terrain), point base_point);
void render();
int updatemap(float **terrain, point camera, point direction);
void updateview(point camera, point direction);

int main(int argc, char **argv)
{
    int i, j;
    float length, mouse_x, mouse_y;

    point camera, direction, old_camera_pos, tmp;
    // The theoretical camera position
    camera.x = MAP_WIDTH/2;
    camera.y = NOISE_SCALE + 15.0f;
    camera.z = -MAP_LENGTH/2;

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

    tmp.x = (int)camera.x;
    tmp.y = (int)camera.y;
    tmp.z = 0;
    generate_heightmap(terrain, tmp);

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
        render(terrain, camera, direction);

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

void generate_heightmap(float (**terrain), point base_point)
{
    int x, y;
    point tmpPoint;
    FILE *fp;
    fp = fopen("image.map", "w");

    // We don't need the z, so just set it to 0 and ignore
    tmpPoint.z = 0;

    for(x=-(MAP_WIDTH/2); x<((MAP_WIDTH/2)); x++)
    {
        // /10 because positions are always ints here, but I don't want white noise. So scale up.
        tmpPoint.x = (x+(MAP_WIDTH/2.0f)+base_point.x)/NOISE_RESOLUTION;
        for(y=-(MAP_LENGTH/2); y<((MAP_LENGTH/2)); y++)
        {
            tmpPoint.y = (y+(MAP_LENGTH/2.0f)+base_point.y)/NOISE_RESOLUTION;
            //printf("(%f|%f|%f)", tmpPoint.x, tmpPoint.y, tmpPoint.z);
            terrain[x+(MAP_WIDTH/2)][y+(MAP_LENGTH/2)] = fBm(tmpPoint, NOISE_DIMENSION, NOISE_OCTAVE_NUMBER)*NOISE_SCALE;
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

void render(float **terrain, point camera, point direction)
{
    int x, z;
    float height[4], length;
    point normal;

    GLfloat lightPos[] = {0.0f, 1.0f, 0.0f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    //Set up the material

    //The color of the object
    GLfloat materialColor[] = {0.2f, 0.2f, 1.0f, 1.0f};
    //The specular (shiny) component of the material
    GLfloat materialSpecular[] = {0.0f, 0.0f, 0.0f, 0.0f};
    //The color emitted by the material
    GLfloat materialEmission[] = {0, 0, 0, 0.0f};

    glDisable(GL_COLOR_MATERIAL); //Required for the glMaterial calls to work
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, materialColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);

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
            length = length_of_vector(normal);
            normal.x /= length;
            normal.y /= length;
            normal.z /= length;

            glNormal3f(normal.x, normal.y, normal.z);
            glVertex3f(x, height[0], -(z));
            glVertex3f(x+1, height[1], -(z));
            glVertex3f(x, height[2], -(z+1));
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
    generate_heightmap(terrain, tmp);
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
