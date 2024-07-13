#include "./laplacian.h"

// Function to apply the Laplacian filter
void applyLaplacian(unsigned char src[IMAGE_HEIGHT][IMAGE_WIDTH], int dst[IMAGE_HEIGHT][IMAGE_WIDTH]) {
    int kernel[3][3] = {
        { 0,  1, 0},
        { 1, -4, 1},
        { 0,  1, 0}
    };

    for (int i = 1; i < IMAGE_HEIGHT - 1; ++i) {
        for (int j = 1; j < IMAGE_WIDTH - 1; ++j) {
            int sum = 0;
            for (int k = -1; k <= 1; ++k) {
                for (int l = -1; l <= 1; ++l) {
                    sum += src[i + k][j + l] * kernel[k + 1][l + 1];
                }
            }
            dst[i][j] = sum;
        }
    }
}
