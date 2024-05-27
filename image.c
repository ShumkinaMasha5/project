#include "lodepng.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

char* load_png_file(const char *filename, int *width, int *height) {
    unsigned char *image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if (error) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
        return NULL;
    }

    return (image);
}


void floodFill(unsigned char* image, int x, int y, int newColor1, int newColor2, int newColor3, int oldColor, int width, int height) {
    typedef struct { int x, y;} Point;

    int dx[] = {-1, 0, 1, 0};
    int dy[] = {0, 1, 0, -1};

    Point* stack = malloc(width * height * 4 * sizeof(Point));
    long top = 0;

    stack[top++] = (Point){x, y};

    while(top > 0) {
        Point p = stack[--top];

        if(p.x < 0 || p.x >= width || p.y < 0 || p.y >= height)
            continue;

        int resultIndex = (p.y * width + p.x) * 4;
        if(image[resultIndex] > oldColor)
            continue;

        image[resultIndex] = newColor1;
        image[resultIndex + 1] = newColor2;
        image[resultIndex + 2] = newColor3;


        for(int i = 0; i < 4; i++) {
            int nx = p.x + dx[i];
            int ny = p.y + dy[i];
            int nIndex = (ny * width + nx) * 4;
            if(nx > 0 && nx < width && ny > 0 && ny < height && image[nIndex] <= oldColor) {
                stack[top++] = (Point){nx, ny};
            }
        }
    }
    free(stack);
}


void applySobelFilter(unsigned char *image, int width, int height) {
    int gx[3][3] = {{-1, 0, 1},
                    {-2, 0, 2},
                    {-1, 0, 1}};

    int gy[3][3] = {{1, 2, 1},
                    {0, 0, 0},
                    {-1, -2, -1}};


    unsigned char *result = malloc(width * height * 4 * sizeof(unsigned char));

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int sumX = 0, sumY = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int index = ((y+dy) * width + (x+dx)) * 4;
                    int gray = (image[index] + image[index + 1] + image[index + 2]) / 3;
                    sumX += gx[dy + 1][dx + 1] * gray;
                    sumY += gy[dy + 1][dx + 1] * gray;
                }
            }
            int magnitude = sqrt(sumX * sumX + sumY * sumY);
            if (magnitude > 255) magnitude = 255;
            if (magnitude < 0) magnitude = 0;

            int resultIndex = (y * width + x) * 4;
            result[resultIndex] = (unsigned char)magnitude;
            result[resultIndex + 1] = (unsigned char)magnitude;
            result[resultIndex + 2] = (unsigned char)magnitude;
            result[resultIndex + 3] = image[resultIndex + 3];
        }
    }

    for (int i = 0; i < width * height * 4; i++) {
        image[i] = result[i];
    }


    free(result);
}





int main() {
    int w = 0, h = 0;

    char *filename = "skull2.png";

    char *picture = load_png_file(filename, &w, &h);

    applySobelFilter(picture, w, h);

    int epsilon = 16;
    int color1 = epsilon * 3;
    int color2 = epsilon * 3;
    int color3 = epsilon * 3;
    for(int y = 1; y < h - 1; y++) {
        for(int x = 1; x < w - 1; x++) {
            if(picture[4 * (y * w + x)] < epsilon) {
                floodFill(picture, x, y, color1, color2, color3, epsilon, w, h);

            }
            color1 = rand() % (255 - epsilon * 2) + epsilon * 2;
            color2 = rand() % (255 - epsilon * 2) + epsilon * 2;
            color3 = rand() % (255 - epsilon * 2) + epsilon * 2;
        }
    }
    char *output_filename = "output3.png";
    lodepng_encode32_file(output_filename, picture, w, h);

    free(picture);
    return 0;
}

