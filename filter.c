
// *******************************************************
// filter.c
//
// Generates serial data to print for debugging
// P.J. Bones UCECE, modified by Ryan hall
// Last modified:  18.04.2017
// Helicopter project
// Group: A03 Group 10
// *******************************************************

// filter.cpp - Filtering measured frequency.
// ENEL200 Design Project, building a theremin.
// 25-09-17, Group 13.
//
// Convolute a Gausian filter over distance data.

#define KERNEL_SIZE 13

double distanceBuffer[KERNEL_SIZE] = {0.0};
double filterKernal[KERNEL_SIZE] = {0.000244140625, 0.0029296875, 0.01611328125,
    0.0537109375, 0.120849609375, 0.193359375, 0.2255859375, 0.193359375,
    0.120849609375, 0.0537109375, 0.01611328125, 0.0029296875, 0.000244140625};

// True modulus, result is allways a positive int.
int modp(int a, int b) {
    return (a + b) % b;
}

// Apply filter kernal to distance data (i.e. smooth the data).
double filter_apply(float nextDistance) {
    static int nextPos = 3;

    // Update newest value.
    distanceBuffer[nextPos] = nextDistance;

    double sum = 0.0;
    for (int i = 0; i < KERNEL_SIZE; i++) {
        int disti = modp(nextPos + i + 1, KERNEL_SIZE);
        sum += distanceBuffer[disti] * filterKernal[i];
    }

    // Increment for next time.
    nextPos = modp(nextPos + 1, KERNEL_SIZE);
    return sum;
}


// So that there is no transient sound on startup, initalise to max value.
void filter_init_with(double initValue)
{
    for (int i = 0; i < KERNEL_SIZE; distanceBuffer[i++] = initValue);
}
