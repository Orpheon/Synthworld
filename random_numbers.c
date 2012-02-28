#ifndef RANDOM_NUM_INIT_H
#define RANDOM_NUM_INIT_H
#include <time.h>

int perm_table[255];
float rand_vectors[255][3];

void initialize_random_numbers()
{
    unsigned int iseed = (unsigned int)time(NULL);
    srand (iseed);

    int i;
    float tmp[3], length;
    for (i=0; i<255; i++)
    {
        perm_table[i] = rand() % 254;

        tmp[0] = (float)(rand() % 1000);
        tmp[1] = (float)(rand() % 1000);
        tmp[2] = (float)(rand() % 1000);

        length = sqrt(tmp[0]*tmp[0] + tmp[1]*tmp[1] + tmp[2]*tmp[2]);
        tmp[0] /= length;
        tmp[1] /= length;
        tmp[2] /= length;

        memcpy(rand_vectors[i], tmp, sizeof(float)*3);
    }
}
#endif
