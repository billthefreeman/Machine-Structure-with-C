/*
 * Date created: 2/07/2015
 * By: Feiyu Lu and Jeanne-Marie Musca
 *
 * UArray2b Implementation
 *         Store and access any objects of same type in a 2d array.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "uarray.h"
#include "uarray2.h"
#include "uarray2b.h"
#include "mem.h"
#include "assert.h" 

/* constant of max default blocksize: 64 KB */
#define DEF_MAXBLK  (64*pow(2,10))

struct UArray2b_T {
        int     width;
        int     height;
        int     size;
        int     blocksize;
        
        /* height of 2d array */
        int     a2height; /* height/blocksize */
        int     a2width;  /* width = width/blocksize */
        
        UArray2_T blocks;
         /* Contains UArray_Ts of length = blocksize*blocksize and size = size.
          *  
          * Element (col, row) in the world of ideas maps to 
          * blocks[col / blk][row / blk][blk * (col % blk) + row % blk]
          * where blk is blocksize, and the square brackets stand for access
          * to a Uarray2_T struct and then to a Hanson UArray_T 
          */
};

/* Given an index, returns the correct block in the 2d array
 * Parameters:
 *      col, row: index to be converted
 *      a: blocked array whose block we want 
 */
static inline UArray_T block(UArray2b_T a, int col, int row)
{
        UArray_T *pblock = (UArray_T *) UArray2_at(a->blocks, col, row); 
        return *pblock;
}

/* Creates empty blocked 2d array with given dimensions & size of object types.
 * blocksize as large as possible provided block occupies at most 64KB 
 * Parameters:
 *      width, height: dimensions of 2d array
 *      size: size of object type to be stored in 2d array.
 */
UArray2b_T UArray2b_new_64K_block(int width, int height, int size)
{
        int blocksize = 0;
        
        /* Find the largest dimension with which we could make a block */
        if(height > width) {
                blocksize = height;
        } else {
                blocksize = width;
        
        }

        /* Check proposed blocksize, must not be larger than 64KB */
        if(blocksize*blocksize*size > DEF_MAXBLK) {
                blocksize = sqrt(DEF_MAXBLK/ size);

        } 

        return UArray2b_new(width, height, size, blocksize);
}

/* Creates empty array with given dimensions, blocksize & size of object types.
 * Parameters:
 *         col, row: given dimensions for 2d array
 *         size: size of object type that will be stores
 *         blocksize: size of blocks within 2d arra
 */
 UArray2b_T UArray2b_new(int width, int height, int size, int blocksize)
 {       
        assert(height >= 0 && width >= 0);
        assert(blocksize >= 0);
        assert(size > 0);
        

        if(blocksize == 0) {
                UArray2b_new_64K_block(width, height, size);
        }
        
        UArray2b_T uarray2b;
        NEW(uarray2b);

        uarray2b->height = height;
        uarray2b->width = width;
        uarray2b->size = size;
        uarray2b->blocksize = blocksize;
        uarray2b->a2height = height % blocksize== 0 ? height / blocksize : 
                                height / blocksize+1;
        uarray2b->a2width= width % blocksize== 0 ? width / blocksize : 
                                width / blocksize + 1;

        /* create 2d array that stores UArray blocks*/ 
        uarray2b->blocks = UArray2_new(uarray2b->a2width, uarray2b->a2height, 
         sizeof(UArray_T));

        /* Initialize blocks in 2d array */
        for(int row = 0; row < uarray2b->a2height; row++) {
                for(int col = 0; col < uarray2b->a2width; col++) {
                        UArray_T *blockp = 
                                UArray2_at(uarray2b->blocks, col, row);
                        *blockp = UArray_new(blocksize*blocksize, size);
                }
        }
        return uarray2b;
        

}
/* Frees all memory allocated in the heap by array elements.
 * Parameter:
 *         UArray2b: array to be freed
 */
 void UArray2b_free(UArray2b_T *uarray2b)
 {      
        assert(uarray2b && *uarray2b);

        for(int row = 0; row < (*uarray2b)->a2height; row++) {
                for(int col = 0; col < (*uarray2b)->a2width; col++) {
                        UArray_T p = block(*uarray2b, col, row);
                        UArray_free(&p);

                }
        }    
        
        UArray2_free(&(*uarray2b)->blocks);
        FREE(*uarray2b);
}

/* Returns the array width.
 * Parameter:
 *         UArray2b: array to whose width is being returned
 */
 int UArray2b_width(UArray2b_T uarray2b)
 {
        assert(uarray2b);
        return uarray2b->width;
}

/* Returns the array height.
 * Parameter:
 *         UArray2b: array to whose height is being returned
 */
 int UArray2b_height(UArray2b_T uarray2b)
 {
        assert(uarray2b);
        return uarray2b->height;
}

/* Returns the stored object type size.
 * Parameter:
 *         UArray2b: array that is storing the objects
 */
 int UArray2b_size(UArray2b_T uarray2b)
 {
        assert(uarray2b);
        return uarray2b->size;
}

int UArray2b_blocksize(UArray2b_T uarray2b)
{
        assert(uarray2b);
        return uarray2b->blocksize;
}

/* Returns a pointer to the object at the provided index.
 * Parameters:
 *        UArray2b: array storing the object
 *        col & row: index for object
 * For cell at index (i,j),the block's at index (i / blocksize, j / blocksize).
 * Within block, cell at index (blocksize *(i % blocksize) + j% blocksize).
 */
 void *UArray2b_at(UArray2b_T uarray2b, int col, int row)
 {       
        int blk_size = uarray2b->blocksize;

        assert(uarray2b);

        assert(row >= 0 &&  row < uarray2b->height);
        assert(col >= 0 &&  col < uarray2b->width);
        
        UArray_T curr_block = block(uarray2b, col / blk_size, row / blk_size);

        return (void *)UArray_at(curr_block, blk_size * (row % blk_size) 
                                        + col % blk_size);
}

/* Mapping function: iterate over objects in the array and apply given
 * function. Iterates block by block.
 * Block row indices increment faster than column indices.
 * Parameters:
 *       UArray2b: array whose objects are being iterated over
 *       apply: provided function, to be applied to all objects in array
 *              - col, row: index of current object
 *              - uarray2b: array
 *              - elem: pointer to current object
 *              - cl: closure
 *       cl: closure
 */
 void UArray2b_map(UArray2b_T uarray2b,
                   void apply(int col, int row, 
                              UArray2b_T uarray2b, 
                              void *elem, void *cl), 
                   void *cl)
 {       
       assert(uarray2b);

        /* square root of length of 1d array representing block */
       int blksize = uarray2b->blocksize;

        /* dimensions of 2d array storing blocks*/
       int width = uarray2b->a2width;
       int height = uarray2b->a2height;

       /* Used in for-loop, used to avoid extra blocks */
       int interior = 0;
       int exterior = 0;

       for(int r = 0; r < height; r++) {
                for(int c = 0; c < width; c++) {
                        UArray_T p = block(uarray2b, c, r);

                        /* case 1: extra cells are at the bottom of block */
                        if((r >= height-1) && (c < width-1)) {
                                exterior = (uarray2b->height - blksize*r);
                                interior = blksize;


                        /* case 2: extra cells are at right edge of block */
                        } else if(r < height-1 && c >= width-1) {
                                exterior = blksize;
                                interior = (uarray2b->width - blksize*c);

                        /* case 3: extra cells at bottom & right of block */ 
                        } else if(r >= height-1 && c >= width-1) {
                                exterior = (uarray2b->height - blksize*r);
                                interior = (uarray2b->width - blksize*c);

                        /* case 4: interior block */       
                        } else{
                                exterior = blksize;
                                interior = blksize;
                        };

                        /* apply function to elements in UArray in 
                         * row major order 
                         */
                        for(int i = 0; i < exterior; i++) {
                                for(int j = 0; j< interior; j++) {

                                        apply(c*blksize+j,r*blksize+i, 
                                              uarray2b,
                                              UArray_at(p, i*blksize + j), 
                                              cl);

                                }
                        }     
                }                    
        }
}
