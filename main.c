#include <GL/glfw.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "geometry.c"
#include "random_numbers.c"
#include "noise.c"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 1000
#define SEA_LEVEL -10
#define GRAVITY 0.7
#define CAMERA_SPEED 1
#define TURNING_SPEED 80
#define CAMERA_MIN_HEIGHT 5
#define MAX_SIGHT_WIDTH MAX_SIGHT_DISTANCE*(tan(FOV*(90/PI)))
#define MAX_SIGHT_DISTANCE 1000
#define FOV 60
#define DISTANCE_FUNCTION(d) (d) // Experiment with this
#ifndef PI
#define PI 3.141592654f
#endif

void updateview();
void render();

int main(int argc, char **argv)
{
    //initialize_random_numbers();

    float mouse_x, mouse_y;

    point camera, direction;
    // The theoretical camera position
    camera.x = 0.0;
    camera.y = CAMERA_MIN_HEIGHT;
    camera.z = 0.0;

    // A unit vector, which is the direction we're pointing at
    direction.x = 0.25f;
    direction.y = 0.0f;
    direction.z = -0.25;
    normalize(&direction);

    // TODO: GENERATE TERRAIN

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

//    //Set up the lighting
//    glEnable(GL_LIGHTING);
//    glEnable(GL_LIGHT0);
//
//    //Ambient lighting
//    GLfloat ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
//    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
//    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
//
//    GLfloat lightColor[] = {0.6f, 0.6f, 0.6f, 1.0f};
//    //Diffuse (non-shiny) light component
//    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
//    //Specular (shiny) light component
//    glLightfv(GL_LIGHT0, GL_SPECULAR, lightColor);

    glViewport (0, 0, (GLsizei)width, (GLsizei)height); //set the viewport to the current window specifications
    glMatrixMode (GL_PROJECTION); //set the matrix to projection

    glLoadIdentity ();
    gluPerspective (60, (GLfloat)width / (GLfloat)height, 1.0, 1000.0); //set the perspective (angle of sight, width, height, depth)
    glMatrixMode (GL_MODELVIEW); //set the matrix back to model

    glFlush();

    // Main loop
    int i, j, hasMoved;
    float x, y, value, dz, distance, increment_x, increment_y;
    point tmp;
    point_2d_array terrain;
    point_array point_row;
    while( running )
    {
        glPushMatrix();// Backup the current coordinate system

        // Handle mouse input
        glfwGetMousePos(&i, &j);

        // Turn them into floats for later
        mouse_x = i;
        mouse_y = j;

        // Get the positions relative to the center of the screen, and normalize them
        mouse_x = (mouse_x - width/2)/width;
        mouse_y = (mouse_y - height/2)/height;

        hasMoved = 0;

        if (mouse_x != 0 || mouse_y != 0)
        {
            // Translate the mouse movement into 3d coordinates
            tmp.x = -direction.z;
            tmp.z = direction.x;
            tmp.y = 0.0;
            normalize(&tmp);
            tmp.x *= mouse_x;
            tmp.z *= mouse_x;

            direction.x += tmp.x;
            direction.z += tmp.z;

            direction.y += sin(PI*TURNING_SPEED*mouse_y/360);

            normalize(&direction);

//            // Reset the mouse in the middle if K isn't pressed (to allow the moving around of the window)
//            if (glfwGetKey('K') != GLFW_PRESS)
//            {
//                glfwSetMousePos(width/2, height/2);
//            }
            hasMoved = 1;
        }

        // Handle keyboard input
        if (glfwGetKey('W') == GLFW_PRESS)
        {
            camera.x += CAMERA_SPEED*direction.x;
            camera.y -= CAMERA_SPEED*direction.y;
            camera.z += CAMERA_SPEED*direction.z;
            hasMoved = 1;
        }
        else if (glfwGetKey('S') == GLFW_PRESS)
        {
            camera.x -= CAMERA_SPEED*direction.x;
            camera.y += CAMERA_SPEED*direction.y;
            camera.z -= CAMERA_SPEED*direction.z;
            hasMoved = 1;
        }

        if (glfwGetKey('A') == GLFW_PRESS)
        {
            camera.x += CAMERA_SPEED*direction.z;
            camera.z -= CAMERA_SPEED*direction.x;
            hasMoved = 1;
        }
        else if (glfwGetKey('D') == GLFW_PRESS)
        {
            camera.x -= CAMERA_SPEED*direction.z;
            camera.z += CAMERA_SPEED*direction.x;
            hasMoved = 1;
        }

//        // Gravity
//        camera.y -= GRAVITY;

        i = 0;
        if (hasMoved==1)
        {
            updateview(camera, direction);

            x = -MAX_SIGHT_DISTANCE;
            while (x < MAX_SIGHT_DISTANCE)
            {
                if (i>=terrain.num_elements)
                {
                    // We need to start creating new rows
                    point_row.array = 0;
                    point_row.num_allocations = 0;
                    point_row.num_elements = 0;
                    add_to_point_2d_array(&terrain, point_row);
                }
                else
                {
                    point_row = terrain.array[i];
                }

                increment_x = DISTANCE_FUNCTION(abs(x));

                j = 0;
                y = -MAX_SIGHT_WIDTH;
                while (y < MAX_SIGHT_WIDTH)
                {
                    increment_y = DISTANCE_FUNCTION(abs(y));

                    value = noise2d(x+camera.x, y+camera.y);
                    tmp.x = x-camera.x;
                    tmp.y = y-camera.y;
                    tmp.z = value-camera.z;
                    if (point_in_frustum(tmp, direction, FOV, MAX_SIGHT_DISTANCE, MAX_SIGHT_WIDTH/2, (MAX_SIGHT_WIDTH*height/width)/2) == 1)
                    {
                        // The point is visible, so remember it


                        if (j >= point_row.num_elements)
                        {
                            // Have to create a new element in the array
                            add_to_point_array(&point_row, tmp);
                        }
                        else
                        {
                            // The room is already there, just overwrite the old value
                            point_row.array[j] = tmp;
                        }

                        // Add points around that specific point

                        tmp.x += increment_x;
                        if (j >= point_row.num_elements)
                        {
                            // Have to create a new element in the array
                            add_to_point_array(&point_row, tmp);
                        }
                        else
                        {
                            // The room is already there, just overwrite the old value
                            point_row.array[j] = tmp;
                        }

                        tmp.x -= increment_x;
                        tmp.y += increment_y;
                        if (j >= point_row.num_elements)
                        {
                            // Have to create a new element in the array
                            add_to_point_array(&point_row, tmp);
                        }
                        else
                        {
                            // The room is already there, just overwrite the old value
                            point_row.array[j] = tmp;
                        }

                        tmp.x += increment_x;
                        if (j >= point_row.num_elements)
                        {
                            // Have to create a new element in the array
                            add_to_point_array(&point_row, tmp);
                        }
                        else
                        {
                            // The room is already there, just overwrite the old value
                            point_row.array[j] = tmp;
                        }

                        j++;
                    }
                    y += increment_y;
                }
                x += increment_x;

                while (j<point_row.num_elements)
                {
                    // Some elements have to get deleted, those that weren't used this frame
                    free(point_row.array + point_row.num_elements-1);
                    point_row.num_elements--;
                }

                // Only create a new row if this one was used
                if (j != 0)
                {
                    i++;
                }
            }

            while (i<terrain.num_elements)
            {
                // Some elements have to get deleted
                point_row = terrain.array[terrain.num_elements-1];
                free_point_array(&point_row);
                free(terrain.array + terrain.num_elements-1);
                terrain.num_elements--;
            }
        }

        // Render
        render(camera, direction);

        glPopMatrix();// Restore the original coordinate system

        // Check if ESC key was pressed or window was closed
        running = !glfwGetKey( GLFW_KEY_ESC ) &&
        glfwGetWindowParam( GLFW_OPENED );
    }
    // Free all the memory allocated for the terrain
    free_point_2d_array(&terrain);
    // Close window and terminate GLFW
    glfwTerminate();
    // Exit program
    exit( EXIT_SUCCESS );
}

void render(point_2d_array terrain, point camera, point direction)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);// Fill the screen with black
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Reset both buffers

    int i, j;
    point_array point_row;
    point tmp_point;
    for (i=0; i<terrain.num_elements; i++)
    {
        point_row = terrain.array[i];
        glBegin(GL_TRIANGLE_STRIP);
        for (j=0; j<point_row.num_elements; j++)
        {
            tmp_point = point_row.array[j];
            glVertex3f(tmp_point.y, tmp_point.z, -tmp_point.x);
        }
        glEnd();
    }

    glFlush ();
    glfwSwapBuffers ();
}

void updateview(point camera, point direction)
{
    float length, xrot, yrot;
    length = sqrt(1-(direction.y*direction.y));
    xrot = asin(direction.x/length);
    yrot = asin(direction.y);

    xrot *= 180/PI;
    yrot *= 180/PI;

    if (direction.z > 0)
    {
        xrot = 180-xrot;
    }

    glRotatef(xrot, 0.0, 1.0, 0.0);
    glRotatef(yrot, -direction.z, 0.0, direction.x);
    //glTranslated(-camera.x, -camera.y, -camera.z);
}
