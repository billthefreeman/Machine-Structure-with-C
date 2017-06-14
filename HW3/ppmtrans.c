/*
 * Date created: 2/11/2015
 * By: Feiyu Lu and Jeanne-Marie Musca
 * a client that performs some simple image transformations
 * it implements both 90-degree and 180-degree rotations
 *      For extra credit we  also implements: 
 *      270 rotation, flip horizontal, flip vertical,
 *      transpose
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mem.h"
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

typedef A2Methods_UArray2 A2;

/* a struct containing information to pass into closure during mapping */
typedef struct map_help {
        A2Methods_T methods_copy;
        A2 output_img;
        
        /* if rotation: 0,90,180,270, depending on angle of rotation
         * if flip: 10 for horizontal, 20 for vertical
         * if transpose: 30
         * if no transformation specified: -1 */
        int transform_type;  
} *Map_help;

/* Used while reading in user input, helps save specifications */
#define SET_METHODS(METHODS, MAP, WHAT) do {                            \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n", argv[0]);     \
                                exit(1);                        \
                }                                               \
} while (0)

/* Transform the image according to user instructions */
static Pnm_ppm transform_image(Pnm_ppm image_reader, int transform_type,
                A2Methods_mapfun map, A2Methods_T methods_copy);

/* Puts each pixel in a new location during mapping function */
static void relocate_pixel(int i, int j, A2 image, A2Methods_Object *ptr,
                void *cl);

/* Helper function: to specify proper usage when input is incorrectly formed */
static void usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>, "
                        "-flip {horizontal, vertical}, -transpose] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

int main(int argc, char *argv[])
{
        /* Default transform to -1 (no change to image)
         * Rotation values: 0, 90, 180, 270 (angle of rotation)
         * Flip values: 10 for horizontal, 20 for vertical */
        int transform = -1;


         /* Check if any files have been passed in by user */
        int found_file = 0;
        
        Pnm_ppm input_img = NULL;
        
        /* Default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);

        /* Default to best map */
        A2Methods_mapfun *map = methods->map_default;
        assert(map);

        /* Read in file and transformations specified in stdin */
        for (int i = 1; i < argc; i++) {
                FILE *fp = fopen(argv[i], "r");
                if (fp !=NULL){
                        found_file = 1;
                        input_img = Pnm_ppmread(fp, methods);
                        fclose(fp);
                }
                
                /* Check if type of access is specified */
                if (!strcmp(argv[i], "-row-major")) {
                        SET_METHODS(uarray2_methods_plain,
                                    map_row_major, "row-major");
                } else if (!strcmp(argv[i], "-col-major")) {
                        SET_METHODS(uarray2_methods_plain,
                                    map_col_major, "column-major");
                } else if (!strcmp(argv[i], "-block-major")) {
                        SET_METHODS(uarray2_methods_blocked,
                                map_block_major, "block-major");
                                
                /* Check if rotation angle is specified */
                } else if (!strcmp(argv[i], "-rotate")) {
                        if (transform != -1) {
                                fprintf(stderr, "Give at most one " 
                                                "transformation\n");
                                usage(argv[0]);
                        }
                        if (!(i + 1 < argc)) {    /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        transform = strtol(argv[++i], &endptr, 10);
                        if (!(transform == 0 || transform == 90
                                        || transform == 180
                                        || transform == 270)) {
                                fprintf(stderr, "Rotation must be "
                                                "0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                
                /* Check if flip type is specified */        
                } else if (!strcmp(argv[i], "-flip")) {
                        if (transform != -1) {
                                fprintf(stderr, "Give at most one " 
                                                "transformation\n");
                                usage(argv[0]);
                        }
                        if (!(i + 1 < argc)) {    /* no flip value */
                                usage(argv[0]);
                        }
                        if (!(strcmp(argv[i++], "horizontal"))) {
                                transform = 10;
                        } else if (!(strcmp(argv[i++], "vertical"))) {
                                transform = 20;           
                        } else {
                               fprintf(stderr, "Flip must be "
                                                "horizontal or "
                                                "vertical\n");
                                usage(argv[0]); 
                        }
                
                /* Check if transpose is specified */
                }else if(!strcmp(argv[i],"-transpose")){
                        if (transform != -1){
                                fprintf(stderr, "Give at most one " 
                                                "transformation\n");
                                usage(argv[0]);
                        }
                        transform = 30;
                        
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n",
                                argv[0], argv[i]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
                
        }

        /* no file found */
        if (found_file == 0) {
                input_img = Pnm_ppmread(stdin, methods);
        }

        /* Reader gets updated image */
        input_img = transform_image(input_img, transform, map, methods);

        /* update the height and width of ppm-reader*/
        input_img->height = input_img->methods->height(input_img->pixels);
        input_img->width = input_img->methods->width(input_img->pixels);

        /* output the transformed image*/
        Pnm_ppmwrite(stdout, input_img);

        Pnm_ppmfree(&input_img);

        exit(EXIT_SUCCESS);
}

/* Return a Pnm_ppm containing transformed image.
 * Parameters:
 *      image_reader: holds original image and methods
 *      transform_type: given transformation
 *      map: given method of accessing pixels in image
 *      methods_copy: A2methods struct, based on specified access method
 */
static Pnm_ppm transform_image(Pnm_ppm image_reader, int transform_type,
                A2Methods_mapfun map, A2Methods_T methods_copy)
{
        int new_width = image_reader->width;
        int new_height = image_reader->height;

        /* Define and malloc the struct to pass to mapping function */
        Map_help newArray;
        NEW(newArray);

        newArray->methods_copy = methods_copy;

        newArray->transform_type = transform_type;

        /* Exchange height and width for 90 & 270 degree rotation & transpose */
        if(transform_type == 90 || transform_type == 270 
                                || transform_type == 30){
                new_width = image_reader->height;
                new_height = image_reader->width;
        }
        /* create an array to the new image */
        newArray->output_img = image_reader->methods->new_with_blocksize(
                        new_width, new_height,
                        image_reader->methods->size(image_reader->pixels),
                        image_reader->methods->blocksize(image_reader->pixels));

        /* Iterates over the pixels in the original image.
         *     Invariant: - the pixels that have already been visited are in
         *                  the correct location in transformed image
         *                - the pixels not yet visited are in their original
         *                  location
         */
        map(image_reader->pixels, relocate_pixel, newArray);

        methods_copy->free(&(image_reader->pixels));

        /* Give the new image to the reader */
        image_reader->pixels = newArray->output_img;

        FREE(newArray);
        return image_reader;
}

/* Calculate new position for each pixel.
 * Parameters:
 *       i, j: (col) (row) index of current object
 *       image: UArray2 that stores original image
 *       ptr: pointer to current object (a Pnm_rgb struct)
 *       cl: closure (will be a map_help struct)
 */
static void relocate_pixel(int i, int j, A2 image, A2Methods_Object *ptr,
                void *cl)
{
        int new_col, new_row; /* new position for pixel */

        struct map_help *new = (struct map_help *)cl;

        A2Methods_T methods = new->methods_copy;
        A2 new_image = new->output_img;

        int img_height = methods->height(new_image);
        int img_width = methods->width(new_image);

        int transform = new->transform_type;

        /* Calculate new position based on specified transformation */
        
        /* rotate */
        if (transform == 180){
                new_col = img_width-i-1;
                new_row = img_height-j-1;
        }else if (transform == 90) {
                new_col = img_width-j-1;
                new_row = i;
        }else if(transform == 270){
                new_col= j;
                new_row = img_height-i-1;
                
        /* flip horizontal */        
        }else if (transform == 10) {
                new_col = img_width-i-1;
                new_row = j;
                
        /* flip vertical */ 
        }else if (transform == 20) {
                new_col = i;
                new_row = img_height-j-1;
                
        /* transpose */        
        }else if(transform == 30){
                new_col = j;
                new_row = i;                
        /* no transformation */        
        }else {
                new_col = i;
                new_row = j;
        }
        /* copy elements to the new struct*/
        memcpy(methods->at(new_image, new_col, new_row), ptr,
                        methods->size(new->output_img));

        (void) image;
}