#ifndef HRIT_ANC_FUNCS_H
#define HRIT_ANC_FUNCS_H

#include "seviri_native_util.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif

int ishrv(int band);
char* chan_name(int cnum);
int assemble_fnames(char ****fnam, const char *indir, const char *timeslot, int nbands, const uint *bids, int sat);
int assemble_proname(char **pnam, const char *indir, const char *timeslot, int sat);
int assemble_epiname(char **enam, const char *indir, const char *timeslot, int sat);
int read_data_oneseg(char* fname,int segnum,int cnum,struct seviri_native_data *d,struct seviri_dimension_data *dims);

#ifdef __cplusplus
}
#endif

#endif /* HRIT_ANC_FUNCS_H */
