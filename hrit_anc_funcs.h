/******************************************************************************%
 *
 *    Copyright (C) 2015-2018 Simon Proud (simon.proud@physics.ox.ac.uk)
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef HRIT_ANC_FUNCS_H
#define HRIT_ANC_FUNCS_H

#include "external.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif


int is_hrv(int band);
const char *chan_name(int cnum);
char *extract_path_sat_id_timeslot(const char *filename, int *sat_id, char *timeslot, int *rss, int *iodc);
int assemble_fnames(char ****fnam, const char *indir, const char *timeslot,
                    int nbands, const uint *bids, int sat, int rss, int iodc);
int assemble_epiname(char **enam, const char *indir, const char *timeslot, int sat, int rss, int iodc);
int assemble_proname(char **pnam, const char *indir, const char *timeslot, int sat, int rss, int iodc);
int read_data_oneseg(char *fname, int segnum, int cnum, struct seviri_data *d, int rss);


#ifdef __cplusplus
}
#endif

#endif /* HRIT_ANC_FUNCS_H */
