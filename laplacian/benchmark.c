#include "laplacian.h"

int main() {
    unsigned char src[IMAGE_HEIGHT][IMAGE_WIDTH];
    int dst[IMAGE_HEIGHT][IMAGE_WIDTH] = {0};

    // Initialize the source image
    int i, j;
    for (i = 0; i < IMAGE_HEIGHT; ++i)
    {
        for (j = 0; j < IMAGE_WIDTH; ++j)
        {
            src[i][j] = image_source[i][j];
        }
    }

    applyLaplacian(src, dst);

    return 0;
}

