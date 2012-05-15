#include <GL/glfw.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "geometry.h"
#include "shaders.h"

#define length_of_vector(a) sqrt(a.x*a.x + a.y*a.y + a.z*a.z)

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1000
#define MAP_HALFWIDTH 1000
#define MAP_HALFLENGTH 1000
#define CAMERA_SPEED 1
#define TURNING_SPEED 80
#define CAMERA_MIN_HEIGHT 5
#ifndef PI
#define PI 3.141592654f
#endif

void render();
void updateview(point camera, point direction);

int main(int argc, char **argv)
{
    int i, j;
    float length, mouse_x, mouse_y;

    point camera, direction, old_camera_pos, tmp;
    // The theoretical camera position
    camera.x = 0.0;
    camera.y = CAMERA_MIN_HEIGHT;
    camera.z = 0.0;

    // The camera position a frame ago (used for detecting and calculating movement)
    old_camera_pos = camera;

    // A unit vector, which is the direction we're pointing at
    direction.x = 0.25f;
    direction.y = 0.0f;
    direction.z = -0.25;

    // Normalize direction
    normalize(&direction);

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

    int width, height;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Get window size
    glfwGetWindowSize( &width, &height );

    // Make sure that height is non-zero to avoid division by zero
    height = height < 1 ? 1 : height;

    // Reset the mouse in the middle
    glfwSetMousePos(width/2, height/2);

    //Set up the lighting
    glDisable(GL_LIGHTING);

    glViewport (0, 0, (GLsizei)width, (GLsizei)height); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)width / (GLfloat)height, 1.0, 1000.0); //set the perspective (angle of sight, width, height, depth)
    glMatrixMode (GL_MODELVIEW); //set the matrix back to model

    glFlush();

    // Load and compile the shaders
    GLuint shader = createShaders();

    // create one display list
    GLuint grid = glGenLists(1);

    // compile the display list, store the grid in it
    glNewList(grid, GL_COMPILE);
        int x, z;
        for(x=-MAP_HALFWIDTH; x<MAP_HALFWIDTH-1; x++)
        {
            glBegin(GL_TRIANGLE_STRIP);
                for(z=-MAP_HALFLENGTH; z<(MAP_HALFLENGTH-1); z++)
                {
                    glVertex3f(x, 0.0, -(z));
                    glVertex3f(x+1, 0.0, -(z));
                }
            glEnd();
        }
    glEndList();

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
        normalize(&direction);

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

        updateview(camera, direction);

        // Render
        render(grid, camera, direction, shader);

        glPopMatrix();// Restore the original coordinate system

        // Check if ESC key was pressed or window was closed
        running = !glfwGetKey( GLFW_KEY_ESC ) &&
        glfwGetWindowParam( GLFW_OPENED );

        old_camera_pos = camera;
    }
    // delete it if it is not used any more
    glDeleteLists(grid, 1);
    // Close window and terminate GLFW
    glfwTerminate();
    // Exit program
    exit( EXIT_SUCCESS );
}

void render(GLuint grid, point camera, point direction, GLuint shader_program)
{
    //printf("\n( %f | %f | %f )", camera.x, camera.y, camera.z);

    // Sync the camera position variable
    GLint camera_pos_ptr;
    camera_pos_ptr = glGetUniformLocation(shader_program, "camera_position");
    glUniform3f(camera_pos_ptr, camera.x, camera.y, camera.z);

    // draw the display list
    glCallList(grid);
    // Swap front and back rendering buffers
    glfwSwapBuffers();
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
    //glTranslated(-camera.x, -camera.y, -camera.z);
}
