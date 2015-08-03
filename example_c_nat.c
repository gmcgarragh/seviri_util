/*
Example program calling seviri_native_util to read a native SEVIRI level 1.5
image file and preprocess it to obtain several fields.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include the seviri_native_util interface header file. */
#include "seviri_native_util.h"

int main(int argc, char *argv[]) {


/* Filename of the .nat file to be read.*/
     char *filename = argv[1];

     int nbands = 4;
     unsigned int band_ids[] = { 1,3,7,9};


/*      Set up the band units. Counts or Radiances are easy, for BRF/BT must subset by channel*/
     enum seviri_units band_units[] = {SEVIRI_UNIT_BRF,SEVIRI_UNIT_BRF,SEVIRI_UNIT_BT,SEVIRI_UNIT_BT,SEVIRI_UNIT_BT};
     
     unsigned int i_line,i_column,i_pixel;

     /* We want to read a sub-image defined by pixel coordinates. */
     int line0   = 1800;
     int line1   = 1900;
     int column0 = 1800;
     int column1 = 1903;

     /* This struct will contain the image data and some metadata. */
     struct seviri_preproc_data preproc;
     
     /* Read and preprocess the data.  Note: the last four arguments are unused
        in this case. Please see the function header for a complete discussion
        of the arguments. */

/*     if (seviri_read_and_preproc(filename, &preproc, nbands, band_ids,*/
/*                                 band_units, SEVIRI_BOUNDS_LINE_COLUMN,*/
/*                                 line0, line1, column0, column1,*/
/*                                 0., 0., 0., 0., 0)) {*/
/*          fprintf(stderr, "ERROR: seviri_read_and_preproc()\n");*/
/*          return -1;*/
/*     }*/

	seviri_read_and_preproc_main(filename, &preproc, nbands, band_ids,
                                 band_units, SEVIRI_BOUNDS_LINE_COLUMN,
                                 line0, line1, column0, column1,
                                 0., 0., 0., 0., 0);
     
     /* Print the values for the central pixel. */
     i_line   = preproc.n_lines / 2;
     i_column = preproc.n_columns / 2;
     i_pixel  = i_line * preproc.n_columns + i_column;

     printf("i_line:			%d\n", i_line);
     printf("i_column:		%d\n", i_column);
     printf("i_pixel:		%d\n", i_pixel);
     printf("Julian Day Number:	%f\n", preproc.time[i_pixel]);
     printf("latitude:		%f\n", preproc.lat [i_pixel]);
     printf("longitude: 		%f\n", preproc.lon [i_pixel]);
     printf("solar zenith angle:	%f\n", preproc.sza [i_pixel]);
     printf("solar azimuth angle:	%f\n", preproc.saa [i_pixel]);
     printf("viewing zenith angle:	%f\n", preproc.vza [i_pixel]);
     printf("viewing azimuth angle:	%f\n", preproc.vaa [i_pixel]);
     printf("\n");

/*      Print the image values for each band that was processed.*/
     printf("0.635 reflectance:		%08.4f\n",preproc.data[0][i_pixel]);
     printf("1.64  reflectance:		%08.4f\n",preproc.data[1][i_pixel]);
     printf("8.70  brightness temperature:	%08.4f\n",preproc.data[2][i_pixel]);
     printf("10.80 brightness temperature:	%08.4f\n",preproc.data[3][i_pixel]);
    

    /* Free memory allocated by seviri_read_and_preproc(). */
     seviri_preproc_free(&preproc);

     return 0;
}
