#ifndef HRIT_FUNCS_H
#define HRIT_FUNCS_H

#include "seviri_native_util.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif


void parseHeader(FILE *fp, long hdrlen, struct seviri_auxillary_io_data *aux);
int read_hrit_prologue(const char *fname, struct seviri_native_data *d,struct seviri_auxillary_io_data *aux);
int read_hrit_epilogue(const char *fname, struct seviri_native_data *d,struct seviri_auxillary_io_data *aux);
int seviri_native_read_hrit(const char *indir, const char *timeslot, int *satnum, struct seviri_native_data *d,
		uint n_bands, const uint *band_ids,enum seviri_bounds bounds,uint line0, uint line1, uint column0,
		uint column1, double lat0, double lat1, double lon0, double lon1);


#ifdef __cplusplus
}
#endif

#endif /* HRIT_FUNCS_H */
