#ifndef HRIT_ANC_FUNCS_H
#define HRIT_ANC_FUNCS_H

#include "seviri_native_util.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}

int ishrv(int band);
char* chan_name(int cnum,int bob);
int assemble_fnames(char ****fnam, char *indir, char *timeslot, int nbands, uint *bids, int sat);
int assemble_proname(char **pnam, char *indir, char *timeslot, int sat);
int assemble_epiname(char **enam, char *indir, char *timeslot, int sat);
int read_data_oneseg(char* fname,int segnum,int cnum,struct seviri_native_data *d,struct seviri_dimension_data *dims);

#endif

#endif /* HRIT_ANC_FUNCS_H */
