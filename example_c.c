/*
Example program calling seviri_util to read a SEVIRI level 1.5 file in either
the HRIT or native format image file and preprocess it to obtain several fields.
*/

#include <stdio.h>
#include <stdlib.h>

/* Include the seviri_util interface header file. */
#include "seviri_util.h"


int main(int argc, char *argv[])
{
     /* Filename of the file to be read. */
     char *filename;

     unsigned int i_line;
     unsigned int i_column;
     unsigned int i_pixel;

     /* We want to read 4 bands: 0.635 and 1.64 reflectance and 8.70 and 10.80
        brightness temperature. */
     unsigned int n_bands = 4;

     unsigned int band_ids[] = {1, 3, 7, 9};

     enum seviri_units band_units[] = {SEVIRI_UNIT_BRF,
                                       SEVIRI_UNIT_BRF,
                                       SEVIRI_UNIT_BT,
                                       SEVIRI_UNIT_BT};

     /* We want to read a sub-image defined by pixel coordinates. */
     int line0   = 1899;
     int line1   = 2199;
     int column0 = 1700;
     int column1 = 2299;

     /* This struct will contain the image data and some metadata. */
     struct seviri_preproc_data preproc;

     /* Get the filename of the file to be read from the command line. */
     if (argc < 2) {
          fprintf(stderr, "ERROR: Filename missing from command line.\n");
          exit(1);
     }

     filename = argv[1];

     /* Read and preprocess the data.  Note: the last four arguments are unused
        in this case. Please see the function header for a complete discussion
        of the arguments. */
     if (seviri_read_and_preproc(filename, &preproc, n_bands, band_ids,
                                 band_units, SEVIRI_BOUNDS_LINE_COLUMN,
                                 line0, line1, column0, column1,
                                 0., 0., 0., 0., 0)) {
          fprintf(stderr, "ERROR: seviri_read_and_preproc_main()\n");
          exit(1);
     }

     /* Print the values for the central pixel. */
     i_line   = preproc.n_lines / 2;
     i_column = preproc.n_columns / 2;
     i_pixel  = i_line * preproc.n_columns + i_column;

     printf("i_line:                       %4d\n", i_line);
     printf("i_column:                     %4d\n", i_column);
     printf("i_pixel:                      %8d\n", i_pixel);
     printf("Julian Day Number:            % .8e\n", preproc.time[i_pixel]);
     printf("latitude:                     % .8e\n", preproc.lat [i_pixel]);
     printf("longitude:                    % .8e\n", preproc.lon [i_pixel]);
     printf("solar zenith angle:           % .8e\n", preproc.sza [i_pixel]);
     printf("solar azimuth angle:          % .8e\n", preproc.saa [i_pixel]);
     printf("viewing zenith angle:         % .8e\n", preproc.vza [i_pixel]);
     printf("viewing azimuth angle:        % .8e\n", preproc.vaa [i_pixel]);
     printf("0.635 reflectance:            % .8e\n", preproc.data[0][i_pixel]);
     printf("1.64  reflectance:            % .8e\n", preproc.data[1][i_pixel]);
     printf("8.70  brightness temperature: % .8e\n", preproc.data[2][i_pixel]);
     printf("10.80 brightness temperature: % .8e\n", preproc.data[3][i_pixel]);

     /* Free memory allocated by seviri_read_and_preproc(). */
     seviri_preproc_free(&preproc);

     return 0;
}
