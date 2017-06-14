/*
 * brightness.c
 * written by: ben tanen & feiyu lu
 * assignment: COMP-40 hw #1 (part a)
 *
 * this code will read in a file and attempt to convert it to a Pnmrdr_T
 * if successful, and of the correct type, it will then calculate the
 * average brightness of the image file.
 * The code relies heavily on the Pnmrdr interface which comes from pnmrdr.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <pnmrdr.h>

Pnmrdr_T checkFileType(FILE *fp);
float calcBrightnessAvg(Pnmrdr_T img);

int main(int argc, char *argv[])
{
        /* declare FILE and Pnmrdr_T ptrs */
        FILE *img;
        Pnmrdr_T pnm_img;

        /* pass filename from command line */
        if (argc == 2) {
                img = fopen(argv[1],"rb");

        /* pass file binary from stdin */
        } else if (argc == 1) {
                img = stdin;

        /* user passed more than two arguments, resulting in error */
        } else {
                fprintf(stderr, "invalid command-line arguments\n");
                exit(EXIT_FAILURE);
        }

        /* checks if file exists */
        if (pnm_img == NULL) {
                fprintf(stderr, "file doesn't exist\n");
                exit(EXIT_FAILURE);
        }

        /*
         * checks if given file type is of valid type
         * if so, returns pointer to converted Pnmrdr_T type
         * if not, returns NULL
         */
        pnm_img = checkFileType(img);
        if (pnm_img != NULL) {
                fprintf(stdout,"%5.3f\n",calcBrightnessAvg(pnm_img));
        }

        /* close file */
        fclose(img);

        return EXIT_SUCCESS;
}

/*
 * attempts to convert file to Pnmrdr_T format
 * if successful, return pointer to instance
 * if not, return NULL pointer
 */
Pnmrdr_T checkFileType(FILE *fp)
{
        /* intialized ptr to return */

        /* attempt to convert file to Pnmrdr_T type */
        TRY
                Pnmrdr_T pnm_img = Pnmrdr_new(fp);
                if (Pnmrdr_data(pnm_img).type == Pnmrdr_gray)
                        return pnm_img;
                else
                        /* report error and free memory */
                        fprintf(stderr,"file not graymap\n");
                        Pnmrdr_free(&pnm_img);

        /* if badformat exception thrown */
        EXCEPT(Pnmrdr_Badformat)
                fprintf(stderr,"wrong file type\n");
        END_TRY;

        /* return nothing, signifying failure */
        return NULL;
}

/*
 * given a particular Pnmrdr_T img
 * calculate the brightness of all of the pxls by looping over each
 * result is returned as a value 0-1 (0 is black, 1 is white)
 */
float calcBrightnessAvg(Pnmrdr_T pnm_img)
{
        /* declare image variables */
        int imgWidth  = Pnmrdr_data(pnm_img).width;
        int imgHeight = Pnmrdr_data(pnm_img).height;
        int imgPxlCount = imgWidth * imgHeight;
        int imgDenom  = Pnmrdr_data(pnm_img).denominator;

        /* initalize average calculation */
        float avgSum = 0;

        /* for loop over every pixel */
        for (int i=0;i<imgPxlCount;i++) {
                avgSum += Pnmrdr_get(pnm_img);
        }     ;

        /* free memory of Pnmrdr file */
        Pnmrdr_free(&pnm_img);

        /*
         * divide avgSum by denominator to get fraction 0 - 1
         * divide by imgPxlCount to get average of all pixels
         */
        return avgSum / (imgPxlCount * imgDenom);
}
