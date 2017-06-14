/*
 * Date created: 1/28/2015
 * By: Saurav Acharya and Jeanne-Marie Musca
 *
 * UArray2 Implementation
 *         Store and access any objects of same type in a 2d array.
 */

#include <stdio.h>
#include <stdlib.h>
#include "uarray.h"
#include "uarray2.h"
#include "mem.h"
#include <assert.h>

struct UArray2_T {
        int      width;
        int      height;
        int      size;
        UArray_T uArr;
};

/* Converts a 2d index (row, col) to a 1d index (i).
 * Parameters:
 *      col, row: index to be converted
 *      width: width of 2d array whose index is being converted
 */
static int convertIndex(int col, int row, int width) {
        return row * width + col;
}

/* Creates empty array with given dimensions & size of object types.
 * Parameters:
 *         col, row: given dimensions for 2d array
 *         size: size of object type that will be stores
 */
extern UArray2_T UArray2_new(int col, int row, int size)
{
        assert(row >= 0 && col >= 0);
        assert(size > 0);

        UArray2_T uArray2;
        NEW(uArray2);

        uArray2->height = row;
        uArray2->width = col;
        uArray2->size = size;
        uArray2->uArr = UArray_new(row * col, size);

        return uArray2;
}

/* Frees all memory allocated in the heap by array elements.
 * Parameter:
 *         uArray2: array to be freed
 */
extern void UArray2_free(UArray2_T *uArray2)
{
        assert(uArray2);
        assert(*uArray2);
        
        UArray_free(&((*uArray2)->uArr));
        FREE(*uArray2);
}

/* Returns the array width.
 * Parameter:
 *         uArray2: array to whose width is being returned
 */
extern int UArray2_width(UArray2_T uArray2)
{
        assert(uArray2);       
        return uArray2->width;
}

/* Returns the array height.
 * Parameter:
 *         uArray2: array to whose height is being returned
 */
extern int UArray2_height(UArray2_T uArray2)
{
        assert(uArray2); 
        return uArray2->height;
}

/* Returns the stored object type size.
 * Parameter:
 *         uArray2: array that is storing the objects
 */
extern int UArray2_size(UArray2_T uArray2)
{
        assert(uArray2); 
        return uArray2->size;
}

/* Returns a pointer to the object at the provided index.
 * Parameters:
 *        uArray2: array storing the object
 *        col & row: index for object
 */
extern void *UArray2_at(UArray2_T uArray2, int col, int row)
{
        assert(uArray2);
        assert(col >= 0 && col < uArray2->width);
        assert(row >= 0 && row < uArray2->height);
       
        return (void *)UArray_at(uArray2->uArr,
                                 convertIndex(col, row, uArray2->width));
}

/* Mapping function: iterate over objects in the array and apply given
 * function. Row indices increment faster than column indices.
 * Parameters:
 *       uArray2: array whose objects are being iterated over
 *       apply: provided function, to be applied to all objects in array
 *              - col, row: index of current object
 *              - uArr2: array
 *              - elem_ptr: pointer to current object
 *              - cl: closure
 *       cl: closure
 */
extern void UArray2_map_col_major(UArray2_T uArray2,
                                  void apply(int col, int row, UArray2_T uArr2,
                                             void *elem_ptr, void *cl),
                                  void *cl)
{
        assert(uArray2); 

        for(int c = 0; c < uArray2->width; c++) {
                for(int r = 0; r < uArray2->height; r++) {
                        apply(c, r, uArray2, UArray2_at(uArray2, c, r), cl);
                }
        }
}

/* Mapping function: iterate over elements in the array and apply given
 * function. Column indices increment faster than row indices.
 * Parameters:
 *        uArray2: array whose objects are being iterated over
 *        apply: provided function, to be applied to all objects in array
 *             - col, row: index of current object
 *             - uArr2: array
 *             - elem_ptr: pointer to current object
 *             - cl: closure
 *        cl: closure
 */
extern void UArray2_map_row_major(UArray2_T uArray2,
                                  void apply(int col, int row, UArray2_T uArr2,
                                             void *elem_ptr,  void *cl),
                                  void *cl)
{
        assert(uArray2); 
        
        for(int r = 0; r < uArray2->height; r++) {
                for(int c = 0; c < uArray2->width; c++) {
                        apply(c, r, uArray2, UArray2_at(uArray2, c, r), cl);
                }
        }
}

