/*
Example program calling seviri_native_util to read a native SEVIRI level 1.5
image file and preprocess it to obtain several fields.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Include the seviri_native_util interface header file. */
#include "seviri_native_util.h"

struct bands_st{
	unsigned int nbands;
	unsigned int *band_ids;
};

void parsebands(char *bands, struct bands_st *inbands)
{
	int len		=	strlen(bands);
	int recval	=	11;
	int i		=	0;
	int act_bands	=	0;
	if (len<11) recval=len;

	inbands->band_ids = malloc(sizeof(unsigned int));
	for (i=0;i<len;i++) {
		if (strcmp(&bands[i],"1")!=-1) {
			act_bands++;
			inbands->band_ids = (unsigned int*) realloc (inbands->band_ids, act_bands * sizeof(unsigned int));
			inbands->band_ids[act_bands-1]=i+1;
		}
	}
	inbands->nbands	=	act_bands;
	return;
}

int main(int argc, char *argv[]) {


// Set up the various band names and output types. Used for printing results.
     char bnames[12][7]; 
     strcpy(bnames[0],"VIS006");     strcpy(bnames[1], "VIS008");     strcpy(bnames[2], "VIS016");     strcpy(bnames[3], "IR_039");
     strcpy(bnames[4], "WV_063");     strcpy(bnames[5], "WV_073");     strcpy(bnames[6], "IR_087");     strcpy(bnames[7], "IR_097");
     strcpy(bnames[8], "IR_108");     strcpy(bnames[9], "IR_120");     strcpy(bnames[10], "IR_134");
     char btypesa[3][4];
     char btypesb[3][4];
     strcpy(btypesa[0], "CNT");     strcpy(btypesa[1], "RAD");     strcpy(btypesa[2], "BRF");
     strcpy(btypesb[0], "CNT");     strcpy(btypesb[1], "RAD");     strcpy(btypesb[2], "BT");
     struct bands_st inbands;

// Filename of the .nat file to be read.
     char *filename = argv[1];

// Bands to be read from file in form 11010110111
// where 1 = read and 0 = don't read. Ignores HRV
     char *bands = argv[2];
     parsebands(bands,&inbands);

// Set output units: 0 for raw count, 1 for radiance, 2 for BRF/BT
     int outunits = atoi(argv[3]);
// Set up the band units. Counts or Radiances are easy, for BRF/BT must subset by channel
     enum seviri_units band_units[inbands.nbands];
     int i;
     for (i=0;i<inbands.nbands;i++){
          if (outunits==0) band_units[i]=SEVIRI_UNIT_CNT;
          if (outunits==1) band_units[i]=SEVIRI_UNIT_RAD;
          if (outunits==2 && inbands.band_ids[i]<4) band_units[i]=SEVIRI_UNIT_BRF;
          if (outunits==2 && inbands.band_ids[i]>=4 && inbands.band_ids[i]<12) band_units[i]=SEVIRI_UNIT_BT;
     }

// If argv[4] == 1 then print extra info on pixel (lat/lon etc)
     int printmore = atoi(argv[4]);


     unsigned int i_line;
     unsigned int i_column;
     unsigned int i_pixel;

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
     if (seviri_read_and_preproc(filename, &preproc, inbands.nbands, inbands.band_ids,
                                 band_units, SEVIRI_BOUNDS_LINE_COLUMN,
                                 line0, line1, column0, column1,
                                 0., 0., 0., 0., 0)) {
          fprintf(stderr, "ERROR: seviri_read_and_preproc()\n");
          return -1;
     }

     /* Print the values for the central pixel. */
     i_line   = 56;
     i_column = 59;
     i_pixel  = i_line * preproc.n_columns + i_column;
     if (printmore==1){
          printf("i_line:			%d\n", i_line);
          printf("i_column:		%d\n", i_column);
          printf("i_pixel:		%d\n", i_pixel);
          printf("Julian Day Number:	%f\n", preproc.time[i_pixel]);
          printf("latitude:		%f\n", preproc.lat [i_pixel]);
          printf("longitude:		%f\n", preproc.lon [i_pixel]);
          printf("solar zenith angle:	%f\n", preproc.sza [i_pixel]);
          printf("solar azimuth angle:	%f\n", preproc.saa [i_pixel]);
          printf("viewing zenith angle:	%f\n", preproc.vza [i_pixel]);
          printf("viewing azimuth angle:	%f\n", preproc.vaa [i_pixel]);
          printf("\n");
     }

// Print the image values for each band that was processed.
     for (i=0;i<inbands.nbands;i++){
     if (inbands.band_ids[i]<=3) printf("%s, %s:		%08.4f\n",bnames[inbands.band_ids[i]-1],btypesa[band_units[i]],preproc.data[i][i_pixel]);
     if (inbands.band_ids[i]>3 && band_units[i]<3) printf("%s, %s:		%08.4f\n",bnames[inbands.band_ids[i]-1],btypesb[band_units[i]],preproc.data[i][i_pixel]);
     if (inbands.band_ids[i]>3 && band_units[i]>=3) printf("%s, %s:		%08.4f\n",bnames[inbands.band_ids[i]-1],btypesb[band_units[i]-1],preproc.data[i][i_pixel]);}


     /* Free memory allocated by seviri_read_and_preproc(). */
     seviri_preproc_free(&preproc);

     return 0;
}
