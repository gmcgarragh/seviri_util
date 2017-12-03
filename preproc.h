/******************************************************************************%
 *
 *    Copyright (C) 2014-2017 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef PREPROC_H
#define PREPROC_H

#include "external.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif


/* The pre-processed SEVIRI data struct with supporting time, lat/lon and
   geometry arrays and arrays of physical units. */

struct seviri_preproc_data {
     int memory_alloc_d;	/* non-zero if memory has been allocated */
     uint n_bands;		/* number of bands read in */
     uint n_lines;		/* number of lines read in */
     uint n_columns;		/* number of columns read in */
     float fill_value;		/* fill value of the image data */
				/* the following image arrays are n_lines * n_columns */
     double *time;		/* image of Julian Day Number */
     float *lat;		/* image of latitude */
     float *lon;		/* image of longitude */
     float *sza;		/* image of solar zenith angle (degrees: 0.0 -- 180.0) */
     float *saa;		/* image of solar azimuth angle  (degrees: 0.0 -- 360.0) */
     float *vza;		/* image of viewing zenith angle (degrees: 0.0 -- 180.0) */
     float *vaa;		/* image of viewing azimuth angle  (degrees: 0.0 -- 360.0) */
     float **data;		/* array of pointers to images of length n_bands */
     float  *data2;		/* array of image data of length n_bands * n_lines * n_columns */
};


int seviri_preproc(const struct seviri_data *d, struct seviri_preproc_data *d2,
                   const enum seviri_units *band_units, int rss, int do_not_alloc);
int seviri_read_and_preproc_nat(const char *filename,
                                struct seviri_preproc_data *preproc,
                                uint n_bands, const uint *band_ids,
                                const enum seviri_units *band_units,
                                enum seviri_bounds bounds, uint line0, uint line1,
                                uint column0, uint column1, double lat0, double lat1,
                                double lon0, double lon1, int do_not_alloc);
int seviri_read_and_preproc_hrit(const char *indir,const char *timeslot,const int satnum,
                                struct seviri_preproc_data *preproc,
                                uint n_bands, const uint *band_ids,
                                const enum seviri_units *band_units,
                                enum seviri_bounds bounds,
                                uint line0, uint line1, uint column0, uint column1,
                                double lat0, double lat1, double lon0, double lon1,
                                int rss, int do_not_alloc);
int seviri_preproc_free(struct seviri_preproc_data *d);
int seviri_read_and_preproc(const char *filename,
                            struct seviri_preproc_data *preproc,
                            uint n_bands, const uint *band_ids,
                            const enum seviri_units *band_units,
                            enum seviri_bounds bounds,
                            uint line0, uint line1, uint column0, uint column1,
                            double lat0, double lat1, double lon0, double lon1,
                            int do_not_alloc);
int seviri_get_dimens(const char *filename, uint *i_line, uint *i_column,
                      uint *n_lines, uint *n_columns, enum seviri_bounds bounds,
                      uint line0, uint line1, uint column0, uint column1,
                      double lat0, double lat1, double lon0, double lon1);


#ifdef __cplusplus
}
#endif

#endif /* PREPROC_H */
