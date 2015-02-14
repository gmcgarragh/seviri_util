/******************************************************************************%
**
**    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/
#include "internal.h"
#include "seviri_native_util.h"


/* MSG-1, MSG-2, MSG-3, MSG-4 */
const ushort n_satellites = 4;

/* Located in the Native MSG level 1.5 header.  Used to find the index to the
   satellite dependent constants below. */
const ushort satellite_ids[] = {321, 322, 323, 324};


/* Ref: PDF_TEN_05105_MSG_IMG_DATA, Table 1  */
const double channel_center_wavelength[SEVIRI_N_BANDS] =
     {0.635, 0.81, 1.64, 3.92, 6.25, 7.35, 8.70, 9.66, 10.80, 12.00,
      13.40, 0.75};

/* Ref: PDF_MSG_SEVIRI_RAD2REFL, Table 1 */
const double band_solar_irradiance[][SEVIRI_N_BANDS] =
{
     /* MSG-1, 321 */
     {65.2296, 73.0127, 62.3715, -999., -999., -999., -999., -999., -999.,
      -999., 78.7599},
     /* MSG-2, 322 */
     {65.2065, 73.1869, 61.9923, -999., -999., -999., -999., -999., -999.,
      -999., 79.0113},
     /* MSG-3, 323 */
     {65.5148, 73.1807, 62.0208, -999., -999., -999., -999., -999., -999.,
      -999., 78.9416},
     /* MSG-4, 324 */
     {65.2656, 73.1692, 61.9416, -999., -999., -999., -999., -999., -999.,
      -999., 79.0035}
};

/* Ref: ROSENFELD_D_2004, Page 6 */

const double bt_nu_c[][SEVIRI_N_BANDS] =
{
     /* MSG-1, 321 */
     {0.0, 0.0, 0.0, 2569.094, 1598.566, 1362.142, 1149.083, 1034.345, 930.659,
      839.661, 752.381, 0.0},
     /* MSG-2, 322 */
     {0.0, 0.0, 0.0, 2568.832, 1600.548, 1360.330, 1148.620, 1035.289, 931.700,
      836.445, 751.792, 0.0}
};

const double bt_A[][SEVIRI_N_BANDS] =
{
     /* MSG-1, 321 */
     {0.0, 0.0, 0.0, 0.9959, 0.9963, 0.9991, 0.9996, 0.9999, 0.9983, 0.9988,
      0.9981, 0.0},
     /* MSG-2, 322 */
     {0.0, 0.0, 0.0, 0.9954, 0.9963, 0.9991, 0.9996, 0.9999, 0.9983, 0.9988,
      0.9981, 0.0}
};

const double bt_B[][SEVIRI_N_BANDS] =
{
     /* MSG-1, 321 */
     {0.0, 0.0, 0.0, 3.471, 2.219, 0.485, 0.181, 0.060, 0.627, 0.397,
      0.576, 0.0},
     /* MSG-2, 322 */
     {0.0, 0.0, 0.0, 3.438, 2.185, 0.470, 0.179, 0.056, 0.640, 0.408,
      0.561, 0.0}
};

/* Ref: PDF_CGMS_LRIT_HRIT_2_6, Page 36 */
const struct nav_scaling_factors nav_scaling_factors_vir =
     {-781648343., -781648343., 1856., 1856.};
const struct nav_scaling_factors nav_scaling_factors_hrv =
     {-2344944937., -2344944937., 5566., 5566};
