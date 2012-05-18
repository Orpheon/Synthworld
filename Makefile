synthworld: main.c shaders.c shaders.h geometry.c geometry.h
	gcc *.c -Wall -O -o synthworld -lGL -lGLU -lGLEW -lglfw
