#include "./laplacian.h"

// Function to apply the Laplacian filter
void applyLaplacian(unsigned char src[IMAGE_HEIGHT][IMAGE_WIDTH], char dst[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    static int kernel[3][3] = {
        { 0,  1, 0},
        { 1, -4, 1},
        { 0,  1, 0}
    };

    int i, j, k, l, sum;
    for (i = 1; i < IMAGE_HEIGHT - 1; ++i)
    {
        for (j = 1; j < IMAGE_WIDTH - 1; ++j)
        {
            sum = 0;
            for (k = -1; k <= 1; ++k)
            {
                for (l = -1; l <= 1; ++l)
                {
                    sum += src[i + k][j + l] * kernel[k + 1][l + 1];
                }
            }
            dst[i][j] = sum;
        }
    }
}
