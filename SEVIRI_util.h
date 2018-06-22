/******************************************************************************%
 *
 *    Copyright (C) 2015-2018 Simon Proud <simon.proud@physics.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "seviri_util.h"


/* Change the variable below (0 or 1) to control if the extra information is printed */

#define VERBOSE 1

/* Define some useful types that are using during processing. */
enum seviri_intypes {SEVIRI_INFILE_HRIT, SEVIRI_INFILE_NAT, N_SEVIRI_INTYPES};
enum seviri_outtypes{SEVIRI_OUTFILE_HDF, SEVIRI_OUTFILE_CDF, SEVIRI_OUTFILE_TIF, N_SEVIRI_OUTTYPES};
enum sat_nums       {SAT_MSG1, SAT_MSG2, SAT_MSG3, SAT_MSG4, N_SEVIRI_SATNUMS};


extern const char *bnames[];


/*  Struct that contains the band information, both number of bands and which to process */
struct bands_st {
     unsigned int nbands;
     unsigned int *band_ids;
};

/*  Struct that contains the driver info, main info needed by the utility. */
struct driver_data {
     int               infrmt;
     char              *infdir;
     char              *timeslot;
     int               satnum;
     char              read_bands[12];
     struct            bands_st sev_bands;
     int               outfrmt;
     enum seviri_units *outtype;
     char              *outf;
     int               bounds;
     int               iline;
     int               icol;
     int               fline;
     int               fcol;
     /* ancsave contains: time,lat,lon,sza,saa,vza,vaa */
     int               ancsave[7];
     int               compression;
     int               do_calib;
};


/*   In SEVIRI_tool_funcs.c */
void show_usage();
int print_driver(struct driver_data driver);
int free_driver(struct driver_data *driver);
int parse_driver(char *fname,struct driver_data *driver);
int print_preproc_out(struct driver_data, struct seviri_preproc_data preproc, unsigned int i_line, unsigned int i_column);

/*   In SEVIRI_tool_prog.c */
int run_sev_native(struct driver_data driver,struct seviri_preproc_data *preproc);
int run_sev_hrit(struct driver_data driver,struct seviri_preproc_data *preproc);

int save_sev_tiff(struct driver_data driver,struct seviri_preproc_data preproc);
int save_sev_cdf(struct driver_data driver,struct seviri_preproc_data preproc);
int save_sev_hdf(struct driver_data driver,struct seviri_preproc_data preproc);
