/******************************************************************************%
**
**    Copyright (C) 2015 Simon Proud (simon.proud@physics.ox.ac.uk)
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#ifndef HRIT_FUNCS_H
#define HRIT_FUNCS_H

#include "external.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif


int read_hrit_epilogue(const char *fname, struct seviri_data *d,
                       struct seviri_auxillary_io_data *aux);
int read_hrit_prologue(const char *fname, struct seviri_data *d,
                       struct seviri_auxillary_io_data *aux);
int alloc_imagearr(uint nbands, const uint *band_ids, struct seviri_data *d);
int seviri_read_hrit(const char *indir, const char *timeslot, int sat,
     struct seviri_data *d, uint n_bands, const uint *band_ids,
     enum seviri_bounds bounds, uint line0, uint line1, uint column0,
     uint column1, double lat0, double lat1, double lon0, double lon1);

#ifdef __cplusplus
}
#endif

#endif /* HRIT_FUNCS_H */
