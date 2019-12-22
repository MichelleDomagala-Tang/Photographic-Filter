
/* 
Filename: filter.c
Author: Michelle Domagala-Tang
The wikipedia for Kernal Image Processing was referenced: https://en.wikipedia.org/wiki/Kernel_(image_processing)
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_VAL 0
#define N 2

typedef struct pixel {
    unsigned char r, g, b;
} Pixel;

typedef struct image {
    unsigned int width, height, max;
    Pixel **data;
} Image;

typedef struct kernel{
    unsigned int size;
    int scale;
    int **matrix;
} Kernel;

/* Reads kernel and stores corresponding data in struct Kernel */
Kernel *readKernel(char *file_name){

    FILE *file = fopen(file_name, "r");
    if (!file){
        fprintf(stderr, "Unable to open file \"%s\"\n", file_name);
        return NULL;
    }
    Kernel *kernel = malloc(sizeof(Kernel));
    if (fscanf(file, "%d", &(kernel->size)) == 0) return NULL;
    if (kernel->size % 2 == 0){
        fprintf(stderr, "Kernal has an incompatible size n = %d\n", kernel->size);
        return NULL;
    } 
    if (fscanf(file, "%d", &(kernel->scale)) == 0) return NULL;

    int i, j;

    kernel->matrix = malloc(sizeof(int) * kernel->size);
    for (i = 0; i < kernel->size; i++){
        kernel->matrix[i] = malloc(sizeof(int) * kernel->size);
    }

    for (i = 0; i < kernel->size; i ++){
        for (j = 0; j < kernel->size; j ++){
            if (fscanf(file, "%d ", &(kernel->matrix[i][j])) == 0) return NULL;
        }
    }
    fclose(file);
    return kernel;
}

/* Reads ppm file and returns a pointer to image struct storing RGB values */
Image *readPPM(char *file_name){

    FILE *ppm_file = fopen(file_name, "r");
    if (ppm_file == NULL){
        fprintf(stderr, "Unable to open file \"%s\"\n", file_name);
        return NULL;
    }

    /* Checks for correct P3 ppm file format */
    char format[3];
    if (fscanf(ppm_file, "%2s\n", format) == 0) return NULL;
    if (strcmp(format, "P3")) return NULL;

    Image *image = malloc(sizeof(Image));

    if (fscanf(ppm_file, "%u %u %u", &image->width, &image->height, &image->max) != 3) return NULL;
    
    image->data = malloc(sizeof(Pixel *) * image->height);
    int i, j;
    for (i = 0; i < image->height; i++){
        image->data[i] = malloc(sizeof(Pixel) * image->width);
    }

    for (i = 0; i < image->height; i++){
        for (j = 0; j < image->width ; j++){
            int result = fscanf(ppm_file, "%hhu %hhu %hhu", &(image->data[i][j].r), &(image->data[i][j].g), &(image->data[i][j].b));
            if (result != 3) return NULL;
        }
    }
    fclose(ppm_file);
    return image;
}

/* Takes values from struct Image and prints into a ppm file */
int writePPM(char *file_name, Image *image){

    FILE *ppm_file = fopen(file_name, "w");
    if (!ppm_file){
        fprintf(stderr, "Unable to open file \"%s\"\n", file_name);
        return -1;
    }

    fprintf(ppm_file, "P3\n");
    fprintf(ppm_file, "%u %u\n", image->width, image->height);
    fprintf(ppm_file, "%u\n", image->max);

    int i, j;
    for (i = 0; i < image->height; i++){
        for (j = 0; j < image->width; j++){
            fprintf(ppm_file, "%u %u %u ", image->data[i][j].r, image->data[i][j].g, image->data[i][j].b);
        }
        fprintf(ppm_file,"\n");
    }
    fclose(ppm_file);
    return 0;
}

/* Applies filter through convolution of ppm file and returns struct Image storing new RGB values */
void filter (char *output, Kernel *kernel, Image *image){

    int max = (int) image->max;    
    Image *output_image = malloc(sizeof(Image));
    output_image->data = malloc(sizeof(Pixel *) * output_image->height);
    int i, j;
    for (i = 0; i < output_image->height; i++){
        output_image->data[i] = malloc(sizeof(Pixel) * output_image->width);
    }
    output_image->width = image->width;
    output_image->height = image->height;
    output_image->max = image->max;

    int row, col, accum_red, accum_green, accum_blue, height_index, width_index, red, green, blue;
    for (i = 0; i < image->height; i++){
        for (j = 0; j < image->width; j++){
            accum_blue = 0;
            accum_red = 0;
            accum_green = 0;
            
            for (row = 0; row < kernel->size; row++){
                for (col = 0; col < kernel->size; col++){
                    height_index = i - ((kernel->size)/N) + row;
                    width_index = j - ((kernel->size)/N) + col;

                    if (height_index >= 0 && width_index >= 0 && height_index < output_image->height && width_index < output_image->width){
                        red = (int) image->data[height_index][width_index].r;
                        green = (int) image->data[height_index][width_index].g;
                        blue = (int) image->data[height_index][width_index].b;

                        accum_red += (red * (kernel->matrix[row][col]) / (kernel->scale));
                        accum_green += (green * (kernel->matrix[row][col]) / (kernel->scale));
                        accum_blue += (blue * (kernel->matrix[row][col]) / (kernel->scale));
                    }
                }
            }
            if (accum_red < 0) accum_red = MIN_VAL;
            if (accum_green < 0) accum_green = MIN_VAL;
            if (accum_blue < 0) accum_blue = MIN_VAL;

            if (accum_red > max) accum_red = max;
            if (accum_green > max) accum_green = max;
            if (accum_blue > max) accum_blue = max;

            /* Store new values in output_file */
            output_image->data[i][j].r = (unsigned char) accum_red;
            output_image->data[i][j].g = (unsigned char) accum_green;
            output_image->data[i][j].b = (unsigned char) accum_blue;
        }
    }

    writePPM(output, output_image);

    free(output_image);
    free(kernel->matrix);
    free(kernel);
    free(image);
}


int main(int argc, char** argv){
    if (argc != 4){
        printf("Usage: ./filter input.ppm kernel output.ppm\n");
        return -1;
    }

    char *input = argv[1];
    char *kernel_in = argv[2];
    char *output = argv[3];

    Image *image = readPPM(input);
    Kernel *kernel = readKernel(kernel_in);
    filter(output, kernel, image);
    free(kernel);
    free(image);
    return 0;
}