#include "./laplacian.h"

unsigned char source_image[6][6] = {
    {236,150,124,208,255,255},
    {173,65,65,113,242,255},
    {156,62,59,54,123,210},
    {202,78,50,51,55,123},
    {133,72,55,68,86,157},
    {145,163,160,171,199,232},
};
int main() {
    char dst[IMAGE_HEIGHT][IMAGE_WIDTH] = {0};

    // Initialize the source image
    applyLaplacian(source_image, dst);

    return 0;
}

