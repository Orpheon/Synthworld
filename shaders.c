#include <GL/glfw.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
	printf("%s\n",infoLog);
        free(infoLog);
    }
}

GLuint createShaders()
{
    glFlush();

	GLuint v, f, p;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	FILE *vert_file = fopen("vertex.shader", "rb");
    fseek(vert_file, 0, SEEK_END);
    long v_pos = ftell(vert_file);
    fseek(vert_file, 0, SEEK_SET);
    char *vs = malloc(v_pos);
    fread(vs, v_pos, 1, vert_file);
    fclose(vert_file);
    glShaderSource(v, 1, &vs,NULL);

    FILE *frag_file = fopen("fragment.shader", "rb");
    fseek(frag_file, 0, SEEK_END);
    long f_pos = ftell(frag_file);
    fseek(frag_file, 0, SEEK_SET);
    char *fs = malloc(f_pos);
    fread(fs, f_pos, 1, frag_file);
    fclose(frag_file);
    glShaderSource(f, 1, &fs,NULL);

	free(vs);
	free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);

	p = glCreateProgram();

	glAttachShader(p,v);
	glAttachShader(p,f);

	glLinkProgram(p);
	glUseProgram(p);

	printShaderInfoLog(p);

	return p;
}
