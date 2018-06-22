/******************************************************************************%
 *
 *    Copyright (C) 2014-2018 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef MISC_UTIL_H
#define MISC_UTIL_H

#include "external.h"

#ifdef __cplusplus
extern "C" {
#endif

double snu_get_ar_val(double ac, double bc, double g0);
double snu_get_br_val(double bc, double gs);
int snu_is_little_endian(void);
double snu_rint(double x);
void snu_init_array_uc(uchar *a, uint n, uchar x);
void snu_init_array_us(ushort *a, uint n, ushort x);
void snu_init_array_f(float *a, uint n, float x);
void snu_init_array_d(double *a, uint n, double x);
void snu_jul_to_cal_date(long jul, int *y, int *m, int *d);
long snu_cal_to_jul_day(int y, int m, int d);
double snu_solar_distance_factor2(double jday);


#ifdef __cplusplus
}
#endif

#endif /* MISC_UTIL_H */
