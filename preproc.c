/*******************************************************************************
 *
 *    Copyright (C) 2014-2018 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#include "external.h"
#include "hrit_anc_funcs.h"
#include "internal.h"
#include "preproc.h"
#include "read_write_hrit.h"
#include "read_write_nat.h"


/*******************************************************************************
 * Compute Julian day from the TIME_CDS_SHORT type.
 ******************************************************************************/
static double TIME_CDS_SHORT_to_jtime(const struct seviri_TIME_CDS_SHORT_data *d)
{
     double jtime_epoch = (double) su_cal_to_jul_day(1958, 1, 1) - .5;

     return jtime_epoch + d->day + d->msec / 1.e3 / 60. / 60. / 24.;
}



/*******************************************************************************
 * This function computes the number of days since satellite launch, based upon
 * the image time and a const array storing launch dates. Needed for the NASA
 * VIS calibration routines.
 ******************************************************************************/
static long get_time_since_launch(const struct seviri_data *d) {

     short  satnum;
     double jtime_start, jtime_end, jtime;
     double dayssince;

     const  double launches[4] = {2452514.5,  // MSG-1
                                  2453725.5,  // MSG-2
                                  2456113.5,  // MSG-3
                                  2457218.5}; // MSG-4

     satnum = d->trailer.ImageProductionStats.SatelliteID;

     jtime_start = TIME_CDS_SHORT_to_jtime(
          &d->trailer.ImageProductionStats.ActScanForwardStart);
     jtime_end   = TIME_CDS_SHORT_to_jtime(
          &d->trailer.ImageProductionStats.ActScanForwardEnd);

     jtime = (jtime_start + jtime_end) / 2.;

     dayssince = jtime - launches[satnum-321];

     return dayssince;
}



/*******************************************************************************
 * This function generates an array of NASA calibration values for the VIS
 * channels based upon satellite ID, band and days since satellite launch.
 ******************************************************************************/
static float *get_nasa_calib(short satnum, int band_id, long dayssince) {

     static float retval[3];

     // Define arrays with g0, g1 and g2 values for each satellite
     // MSG-1 has two sets, one pre- and one post-IODC move.
     const double gval_met8_a1[3]  = {0.6208,   9.56e-6,  0.};
     const double gval_met8_a2[3]  = {0.4862,   -1e-6,    0.};
     const double gval_met8_a3[3]  = {8.66e-2,  0.,       0.};

     const double gval_met8_b1[3]  = {0.6715,   0.,       0.};
     const double gval_met8_b2[3]  = {0.4952,   0.,       0.};
     const double gval_met8_b3[3]  = {8.541e-2, 0.,       0.};

     const double gval_met9_a1[3]  = {0.5461,   4.602e-6, 0.};
     const double gval_met9_a2[3]  = {0.416,    5.57e-6,  0.};
     const double gval_met9_a3[3]  = {8.173e-2, 0.,       0.};

     const double gval_met10_a1[3] = {0.5638,   1.607e-5, 0.};
     const double gval_met10_a2[3] = {0.4444,   2.464e-6, 0.};
     const double gval_met10_a3[3] = {8.673e-2, 0.,       0.};

     const double gval_met11_a1[3] = {0.5758,   0.,       0.};
     const double gval_met11_a2[3] = {0.4334,   0.,       0.};
     const double gval_met11_a3[3] = {8.341e-2, 0.,       0.};

     const double solar_met8_a[3]  = {516.2,    354.6,    75.68};
     const double solar_met8_b[3]  = {516.2,    354.6,    75.68};
     const double solar_met9_a[3]  = {516.1,    355.4,    75.29};
     const double solar_met10_a[3] = {518.3,    355.4,    75.32};
     const double solar_met11_a[3] = {516.5,    355.3,    75.27};

     // Subtract 320 from satnum to give a value between 1-4
     satnum = satnum - 320;
     // Add 1 to band_id to give a value between 1-12
     band_id = band_id + 1;

     // The 'deep space' count is (possibly erroneously) given as 51 for all
     // bands and all sensors.
     retval[1] = 51;

     // MSG-1
     if (satnum==1) {
          if (dayssince>=5271) { // Pre-IODC
               if (band_id == 1) retval[0] = gval_met8_a1[0] +
                                             gval_met8_a1[1]*dayssince +
                                             gval_met8_a1[2]*dayssince*dayssince;
               if (band_id == 2) retval[0] = gval_met8_a2[0] +
                                             gval_met8_a2[1]*dayssince +
                                             gval_met8_a2[2]*dayssince*dayssince;
               if (band_id == 3) retval[0] = gval_met8_a3[0] +
                                             gval_met8_a3[1]*dayssince +
                                             gval_met8_a3[2]*dayssince*dayssince;
               retval[2] = solar_met8_a[band_id-1];
          }
          else { // Post-IODC
               if (band_id == 1) retval[0] = gval_met8_b1[0] +
                                             gval_met8_b1[1]*dayssince +
                                             gval_met8_b1[2]*dayssince*dayssince;
               if (band_id == 2) retval[0] = gval_met8_b2[0] +
                                             gval_met8_b2[1]*dayssince +
                                             gval_met8_b2[2]*dayssince*dayssince;
               if (band_id == 3) retval[0] = gval_met8_b3[0] +
                                             gval_met8_b3[1]*dayssince +
                                             gval_met8_b3[2]*dayssince*dayssince;
               retval[2] = solar_met8_b[band_id-1];
          }
     }

     // MSG-2
     else if (satnum==2) {
          if (band_id == 1) retval[0] = gval_met9_a1[0] +
		                        gval_met9_a1[1]*dayssince +
		                        gval_met9_a1[2]*dayssince*dayssince;
          if (band_id == 2) retval[0] = gval_met9_a2[0] +
		                        gval_met9_a2[1]*dayssince +
		                        gval_met9_a2[2]*dayssince*dayssince;
          if (band_id == 3) retval[0] = gval_met9_a3[0] +
		                        gval_met9_a3[1]*dayssince +
		                        gval_met9_a3[2]*dayssince*dayssince;
		retval[2] = solar_met9_a[band_id-1];
     }
     // MSG-3
     else if (satnum==3) {
          if (band_id == 1) retval[0] = gval_met10_a1[0] +
		                        gval_met10_a1[1]*dayssince +
		                        gval_met10_a1[2]*dayssince*dayssince;
          if (band_id == 2) retval[0] = gval_met10_a2[0] +
		                        gval_met10_a2[1]*dayssince +
		                        gval_met10_a2[2]*dayssince*dayssince;
          if (band_id == 3) retval[0] = gval_met10_a3[0] +
		                        gval_met10_a3[1]*dayssince +
		                        gval_met10_a3[2]*dayssince*dayssince;
		retval[2] = solar_met10_a[band_id-1];
     }
     // MSG-4
     else if (satnum==4) {
          if (band_id == 1) retval[0] = gval_met11_a1[0] +
		                        gval_met11_a1[1]*dayssince +
		                        gval_met11_a1[2]*dayssince*dayssince;
          if (band_id == 2) retval[0] = gval_met11_a2[0] +
		                        gval_met11_a2[1]*dayssince +
		                        gval_met11_a2[2]*dayssince*dayssince;
          if (band_id == 3) retval[0] = gval_met11_a3[0] +
		                        gval_met11_a3[1]*dayssince +
		                        gval_met11_a3[2]*dayssince*dayssince;
          retval[2] = solar_met11_a[band_id-1];
     }
     else {
          fprintf(stderr, "ERROR: Unrecognised satellite platform: %d\n", satnum);
          exit(-1);
     }

     return retval;
}



/*******************************************************************************
 * This function attempts to extract the GSICS calibration from the MPEF header
 * Should GSICS calibration be unavailable (true for VIS channels as mid-2018)
 * Then a NASA calibration will be used instead. If that fails then the default
 * IMPF calibration will be used instead as a fallback.
 ******************************************************************************/
static void get_cal_slope_and_offset(const struct seviri_data *d, int band_id,
                                     int do_gsics, double *slope, double *offset,
                                     int *do_nasa) {

     double ar, br;
     double ac, bc;
     double gs, go;

     ac = d->header.RadiometricProcessing.
          Level1_5ImageCalibration[band_id - 1].Cal_Offset;
     bc = d->header.RadiometricProcessing.
          Level1_5ImageCalibration[band_id - 1].Cal_Slope;

     if (! do_gsics) {
          *slope  = bc;
          *offset = ac;
     }
     else {
          gs = d->header.RadiometricProcessing.
               MPEFCalFeedback_data[band_id - 1].GSICSCalCoeff;
          go = d->header.RadiometricProcessing.
               MPEFCalFeedback_data[band_id - 1].GSICSOffsetCount;

          if (gs < 0.0000001) {
/*
               fprintf(stderr,"WARNING: GSICS coefficients unavailable. "
                              "Using IMPF calibration.\n");
*/
               *do_nasa = 1;
               *slope  = bc;
               *offset = ac;
          }
          else{
               br = su_get_br_val(bc, gs);
               ar = su_get_ar_val(ac, bc, go);
               *slope  = (bc / br);
               *offset = (ac-ar) / br;
          }
     }
}



/*******************************************************************************
 * Main pre-processing function which includes the computation of Julian Day,
 * latitude, longitude, solar zenith and azimuth angles, viewing zenith and
 * and azimuth angles, and either radiance, reflectance, or brightness
 * temperature for each requested channel.
 *
 * d		: The main input SEVIRI level 1.5 seviri_data struct
 * d2		: The struct containing the preprocessed output
 * band_units	: Array of band_unit types of length n_bands
 * do_not_alloc	: Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_preproc(const struct seviri_data *d, struct seviri_preproc_data *d2,
                   const enum seviri_units *band_units, int rss, int do_gsics,
                   char satposstr[128], int do_not_alloc)
{
     uint i;
     uint ii;
     uint j;
     uint k;

     uint length;

     uint i_sat;

     uint i_image;

     int year;
     int month;
     int day;

     const double c1 = 1.19104e-5;
     const double c2 = 1.43877;

     double a;
     double b;
     double c;
     double e;

     double jtime;
     double jtime2;

     double jtime_end;
     double jtime_start;
     double jtime_end2;
     double jtime_start2;

     double t;

     double X;
     double Y;
     double Z;

     double lon0;

     double mu0;
     double theta0;
     double phi0;

     double slope;
     double offset;

     double day_of_year;

     double R;

     double nu;

     double L;

     int nav_off = 0;

     int   do_nasa = 0;
     long  ldays   = 0;
     float *calivals;

     float orbalt;
     char tmpsatstr[12];


     if (rss)
          nav_off = 464 * 5;


     /*-------------------------------------------------------------------------
      * Find the index for our satellite to satellite dependent constants
      * defined in internal.c.
      *-----------------------------------------------------------------------*/
     for (i = 0; i < n_satellites; ++i) {
          if (d->header.SatelliteStatus.SatelliteId == satellite_ids[i])
               break;
     }

     if (i == n_satellites) {
          fprintf(stderr, "ERROR: Satellite ID not supported: %d\n",
                  d->header.SatelliteStatus.SatelliteId);
          return -1;
     }

     i_sat = i;


     /*-------------------------------------------------------------------------
      * Check if the requested units for each band are supported.
      *-----------------------------------------------------------------------*/
     for (i = 0; i < d->image.n_bands; ++i) {
          if (((d->image.band_ids[i] >= 1 && d->image.band_ids[i] <= 3) ||
                d->image.band_ids[i] == 12) && band_units[i] == SEVIRI_UNIT_BT) {
               fprintf(stderr, "ERROR: Band ID %d, does not support BT units\n",
                       d->image.band_ids[i]);
               return -1;
          }

          if (( d->image.band_ids[i] >= 4 && d->image.band_ids[i] <= 11)
                                          && band_units[i] == SEVIRI_UNIT_BRF) {
               fprintf(stderr, "ERROR: Band ID %d, does not support BRF units\n",
                       d->image.band_ids[i]);
               return -1;
          }
     }


     /*-------------------------------------------------------------------------
      * Set some characteristics supplied with the pre-processing results.
      *-----------------------------------------------------------------------*/
     d2->n_bands    = d->image.n_bands;
     d2->n_lines    = d->image.n_lines;
     d2->n_columns  = d->image.n_columns;
     d2->fill_value = FILL_VALUE_F;


     /*-------------------------------------------------------------------------
      * Allocate and initialize memory.
      *-----------------------------------------------------------------------*/
     length = d->image.n_lines * d->image.n_columns;

     if (do_not_alloc)
          d2->memory_alloc_d = 0;
     else {
          d2->memory_alloc_d = 1;

          d2->time  = malloc(length * sizeof(double));
          d2->lat   = malloc(length * sizeof(float));
          d2->lon   = malloc(length * sizeof(float));
          d2->sza   = malloc(length * sizeof(float));
          d2->saa   = malloc(length * sizeof(float));
          d2->vza   = malloc(length * sizeof(float));
          d2->vaa   = malloc(length * sizeof(float));

          d2->data2 = malloc(d->image.n_bands * length * sizeof(float *));
     }


     d2->data = malloc(d->image.n_bands * sizeof(float **));

     for (i = 0; i < d->image.n_bands; ++i)
          d2->data[i] = &d2->data2[i * length];


     su_init_array_d(d2->time, length, d2->fill_value);
     su_init_array_f(d2->lat,  length, d2->fill_value);
     su_init_array_f(d2->lon,  length, d2->fill_value);
     su_init_array_f(d2->sza,  length, d2->fill_value);
     su_init_array_f(d2->saa,  length, d2->fill_value);
     su_init_array_f(d2->vza,  length, d2->fill_value);
     su_init_array_f(d2->vaa,  length, d2->fill_value);

     su_init_array_f(d2->data2, d->image.n_bands * length, d2->fill_value);


     /*-------------------------------------------------------------------------
      * Compute the day of the year for the center of the image scan.
      *-----------------------------------------------------------------------*/
     jtime_start = TIME_CDS_SHORT_to_jtime(
          &d->trailer.ImageProductionStats.ActScanForwardStart);
     jtime_end   = TIME_CDS_SHORT_to_jtime(
          &d->trailer.ImageProductionStats.ActScanForwardEnd);

     jtime = (jtime_start + jtime_end) / 2.;

     su_jul_to_cal_date((long) floor(jtime       + .5), &year, &month, &day);
     day_of_year = jtime       - (su_cal_to_jul_day(year, 1, 0) - .5);


     /*-------------------------------------------------------------------------
      * Compute the satellite position vector in Cartesian coordinates (km).
      *-----------------------------------------------------------------------*/
     for (i = 0; i < 100; ++i) {
          jtime_start2 = TIME_CDS_SHORT_to_jtime(
               &d->header.SatelliteStatus.OrbitPolynomial[i].StartTime);
          jtime_end2   = TIME_CDS_SHORT_to_jtime(
               &d->header.SatelliteStatus.OrbitPolynomial[i].EndTime);
          if (jtime >= jtime_start2 && jtime <= jtime_end2)
               break;
     }

     if (i == 100) {
          fprintf(stderr, "ERROR: Image time is out of range of supplied orbit "
                  "polynomials\n");
          return -1;
     }

     t = (jtime - (jtime_start2 + jtime_end2)   / 2.) /
                 ((jtime_end2   - jtime_start2) / 2.);

     X = d->header.SatelliteStatus.OrbitPolynomial[i].X[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].X[1] * t;

     Y = d->header.SatelliteStatus.OrbitPolynomial[i].Y[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].Y[1] * t;

     Z = d->header.SatelliteStatus.OrbitPolynomial[i].Z[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].Z[1] * t;
/*
     X = d->header.SatelliteStatus.OrbitPolynomial[i].X[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].X[1] * t -
         0.5* d->header.SatelliteStatus.OrbitPolynomial[i].X[0];

     Y = d->header.SatelliteStatus.OrbitPolynomial[i].Y[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].Y[1] * t -
         0.5* d->header.SatelliteStatus.OrbitPolynomial[i].Y[0];

     Z = d->header.SatelliteStatus.OrbitPolynomial[i].Z[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].Z[1] * t -
         0.5* d->header.SatelliteStatus.OrbitPolynomial[i].Z[0];
*/

     /*-------------------------------------------------------------------------
      * Compute latitude and longitude and solar and sensor zenith and azimuth
      * angles.
      *-----------------------------------------------------------------------*/
     lon0 = d->header.ImageDescription.LongitudeOfSSP;

     for (i = 0; i < d->image.n_lines; ++i) {
          ii = d->image.i_line + i;

          jtime2 = jtime_start + (double) ii / (double) (IMAGE_SIZE_VIR_LINES - 1) *
                   (jtime_end - jtime_start);

          for (j = 0; j < d->image.n_columns; ++j) {
               i_image = i * d->image.n_columns + j;

               su_line_column_to_lat_lon(ii + 1 + nav_off, d->image.i_column + j + 1,
                                          &d2->lat[i_image], &d2->lon[i_image],
                                          lon0, &nav_scaling_factors_vir);

               if (d2->lat[i_image] != FILL_VALUE_F &&
                   d2->lon[i_image] != FILL_VALUE_F) {
                    d2->time[i_image] = jtime2;

                    su_solar_params2(jtime2, d2->lat[i_image] * D2R,
                                      d2->lon[i_image] * D2R, &mu0, &theta0,
                                      &phi0, NULL);
                    d2->sza[i_image] = theta0 * R2D;
                    d2->saa[i_image] = phi0   * R2D;

                    d2->saa[i_image] = d2->saa[i_image] + 180.;
                    if (d2->saa[i_image] > 360.)
                         d2->saa[i_image] = d2->saa[i_image] - 360.;

                    su_vza_and_vaa(d2->lat[i_image], d2->lon[i_image], 0.,
                                    X, Y, Z, &d2->vza[i_image], &d2->vaa[i_image]);

                    d2->vaa[i_image] = d2->vaa[i_image] + 180.;
                    if (d2->vaa[i_image] > 360.)
                         d2->vaa[i_image] = d2->vaa[i_image] - 360.;
               }
          }
     }


     /*-------------------------------------------------------------------------
      * Compute the satellite position string.
      *-----------------------------------------------------------------------*/

     // Satellite latitude, assumed to be 0.0N
     sprintf(tmpsatstr,"%010.7f",0.0);
     strncat(tmpsatstr,"\0",1);
     strncpy(satposstr,tmpsatstr,11);
     strncat(satposstr,",\0",2);

     // Satellite longitude, taken from L1.5 header
     sprintf(tmpsatstr,"%010.7f",lon0);
     strncat(tmpsatstr,"\0",1);
     strncat(satposstr,tmpsatstr,11);
     strncat(satposstr,",\0",2);

     // Satellite height, computed from orbit polynomial
     orbalt = sqrt(X*X + Y*Y + Z*Z);
     sprintf(tmpsatstr,"%010.2f",orbalt);
     strncat(tmpsatstr,"\0",1);
     strncat(satposstr,tmpsatstr,11);
     strncat(satposstr,",\0",2);

     // Equatorial radius from the L1.5 header
     sprintf(tmpsatstr,"%010.2f",d->header.GeometricProcessing.EquatorialRadius);
     strncat(tmpsatstr,"\0",1);
     strncat(satposstr,tmpsatstr,11);
     strncat(satposstr,",\0",2);

     // North polar radius from the L1.5 header
     // There is also a South polar radius in the header, unsure if these are
     // ever different.
     sprintf(tmpsatstr,"%010.2f",d->header.GeometricProcessing.NorthPolarRadius);
     strncat(tmpsatstr,"\0",1);
     strncat(satposstr,tmpsatstr,11);
     strncat(satposstr,"\0",1);

     for (i=strlen(satposstr);i<127;i++)strncat(satposstr,"_\0",2);


     /*-------------------------------------------------------------------------
      * Extract the raw pixel counts only. Do not scale or transform in any way.
      *-----------------------------------------------------------------------*/
     for (i = 0; i < d->image.n_bands; ++i) {
          if (band_units[i] == SEVIRI_UNIT_CNT) {

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;

                         if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                             d->image.data_vir[i][i_image] > 0) {
                              L = d->image.data_vir[i][i_image];

                              d2->data[i][i_image] = L;
                         }
                    }
               }
          }
     }


     /*-------------------------------------------------------------------------
      * Compute radiance for the bands requested.
      *
      * Ref: PDF_TEN_05105_MSG_IMG_DATA, Page 26
      *-----------------------------------------------------------------------*/
     for (i = 0; i < d->image.n_bands; ++i) {
          if (band_units[i] == SEVIRI_UNIT_RAD) {

               do_nasa = 0;
               get_cal_slope_and_offset(d, d->image.band_ids[i], do_gsics,
                                        &slope, &offset, &do_nasa);

               if (do_nasa) {
                    ldays    = get_time_since_launch(d);
                    calivals = get_nasa_calib(d->trailer.ImageProductionStats.SatelliteID,
                                              d->image.band_ids[i]-1, ldays);
               }

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;
                         if (do_nasa)
                              d2->data[i][i_image] = (d->image.data_vir[i][i_image] -
                                                      calivals[1]) * calivals[0];
                         else{
                              if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                                  d->image.data_vir[i][i_image] > 0) {
                                   d2->data[i][i_image] =
                                        d->image.data_vir[i][i_image] * slope + offset;
                              }
                         }
                    }
               }
          }
     }


     /*-------------------------------------------------------------------------
      * Compute reflectance or bidirectional reflectance factor (BRF) for the
      * bands requested.
      *
      * Ref: PDF_MSG_SEVIRI_RAD2REFL, Page 8
      *-----------------------------------------------------------------------*/
/*
     a = sqrt(su_solar_distance_factor2(day_of_year));
*/
     a = PI * sqrt(su_solar_distance_factor2(day_of_year));

     for (i = 0; i < d->image.n_bands; ++i) {
          if (band_units[i] == SEVIRI_UNIT_REF || band_units[i] == SEVIRI_UNIT_BRF) {

               do_nasa = 0;
               get_cal_slope_and_offset(d, d->image.band_ids[i], do_gsics,
                                        &slope, &offset, &do_nasa);

               b = a / band_solar_irradiance[i_sat][d->image.band_ids[i] - 1];

               if (do_nasa) {
                  ldays     =  get_time_since_launch(d);
                  calivals  =  get_nasa_calib(d->trailer.ImageProductionStats.SatelliteID,
                                              d->image.band_ids[i]-1, ldays);
               }

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;

                         if (do_nasa) {
                              if (d->image.data_vir[i][i_image] <= calivals[1]+1)
                                   d2->data[i][i_image] = FILL_VALUE_F;
                              else {
                                   R = (d->image.data_vir[i][i_image] -
                                        calivals[1]) * calivals[0];
                                   d2->data[i][i_image] = (R * a) / calivals[2];

                                   if (band_units[i] == SEVIRI_UNIT_BRF)
                                        d2->data[i][i_image] /= cos(d2->sza[i_image] * D2R);
                              }
                         }
                         else {
                              if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                                  d->image.data_vir[i][i_image] > 0 &&
                                  d2->sza[i_image] >= 0. && d2->sza[i_image] < 90.) {
                                   R = d->image.data_vir[i][i_image] * slope + offset;
                                   d2->data[i][i_image] = b * R;

                                   if (band_units[i] == SEVIRI_UNIT_BRF) {
/*
                                        d2->data[i][i_image] *= PI;
*/
                                        d2->data[i][i_image] /= cos(d2->sza[i_image] * D2R);
                                   }
                              }
                         }
                    }
               }
          }
     }


     /*-------------------------------------------------------------------------
      * Compute brightness temperature for the bands requested.
      *
      * Ref: PDF_TEN_05105_MSG_IMG_DATA, Page 26
      *-----------------------------------------------------------------------*/
     for (i = 0; i < d->image.n_bands; ++i) {
          if (band_units[i] == SEVIRI_UNIT_BT) {

               do_nasa = 0;
               get_cal_slope_and_offset(d, d->image.band_ids[i], do_gsics,
                                        &slope, &offset, &do_nasa);
/*
               nu = 1.e4 / channel_center_wavelength[d->image.band_ids[i] - 1];
*/
               nu = bt_nu_c[i_sat][d->image.band_ids[i] - 1];

               a = bt_A[i_sat][d->image.band_ids[i] - 1];
               b = bt_B[i_sat][d->image.band_ids[i] - 1];

               c = c2 * nu;
               e = nu * nu * nu * c1;

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;

                         if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                             d->image.data_vir[i][i_image] > 0) {
                              L = d->image.data_vir[i][i_image] * slope + offset;

                              d2->data[i][i_image] =
                                   (c / log(1. + e / L) - b) / a;
                         }
                    }
               }
          }
     }


     return 0;
}



/*******************************************************************************
 * Convenience function that calls both seviri_read_nat() and seviri_preproc()
 * as this is likely the most common usage scenario.
 *
 * filename	: Native SEVIRI level 1.5 filename
 * preproc	: The struct containing the preprocessed output
 * n_bands	: Described in the seviri_read_nat() header (read_write_nat.c)
 * band_ids	:      ''
 * band_units	: Array of band_unit types of length n_bands
 * bounds	: Described in the seviri_read_nat() header (read_write_nat.c)
 * line0	:      ''
 * line1	:      ''
 * column0	:      ''
 * column1	:      ''
 * lat0		:      ''
 * lat1		:      ''
 * lon0		:      ''
 * lon1		:      ''
 * do_gsics	: Flag indicating whether to apply GSICS or IMPF calibration.
 * satposstr	: String to store the satellite position information required for
 *                parallax correction.
 * do_not_alloc	: Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_read_and_preproc_nat(const char *filename,
                                struct seviri_preproc_data *preproc,
                                uint n_bands, const uint *band_ids,
                                const enum seviri_units *band_units,
                                enum seviri_bounds bounds,
                                uint line0, uint line1, uint column0, uint column1,
                                double lat0, double lat1, double lon0, double lon1,
                                int do_gsics, char satposstr[128], int do_not_alloc)
{
     struct seviri_data seviri;
     int rss=0;

     if (seviri_read_nat(filename, &seviri, n_bands, band_ids, bounds,
                     line0, line1, column0, column1, lat0, lat1, lon0, lon1)) {
          fprintf(stderr, "ERROR: seviri_read_nat()\n");
          return -1;
     }

     if (seviri_preproc(&seviri, preproc, band_units, rss, do_gsics, satposstr,
                        do_not_alloc)) {
          fprintf(stderr, "ERROR: seviri_preproc()\n");
          return -1;
     }

     seviri_free(&seviri);

     return 0;
}



/*******************************************************************************
 * Convenience function that calls both seviri_read_hrit() and seviri_preproc()
 * as this is likely the most common usage scenario.
 *
 * indir	: Input directory that contains the HRIT files
 * timeslot	: The timeslot for processing in the form YYYYMMDDHHMM
 * satnum	: The satellite ID in the form "MSGx", with x=1,2,3,4
 * preproc	: The struct containing the preprocessed output
 * n_bands	: Described in the seviri_read_nat() header (read_write_nat.c)
 * band_ids	:      ''
 * band_units	: Array of band_unit types of length n_bands
 * bounds	: Described in the seviri_read_nat() header (read_write_nat.c)
 * line0	:      ''
 * line1	:      ''
 * column0	:      ''
 * column1	:      ''
 * lat0		:      ''
 * lat1		:      ''
 * lon0		:      ''
 * lon1		:      ''
 * do_gsics	: Flag indicating whether to apply GSICS or IMPF calibration.
 * satposstr	: String to store the satellite position information required for
 *                parallax correction.
 * do_not_alloc	: Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_read_and_preproc_hrit(const char *indir, const char *timeslot,
                                 const int satnum,
                                 struct seviri_preproc_data *preproc,
                                 uint n_bands, const uint *band_ids,
                                 const enum seviri_units *band_units,
                                 enum seviri_bounds bounds,
                                 uint line0, uint line1, uint column0, uint column1,
                                 double lat0, double lat1, double lon0, double lon1,
                                 int rss, int iodc, int do_gsics, char satposstr[128],
                                 int do_not_alloc)
{
     int i, proc_hrv = 0;

     struct seviri_data seviri;

     if (seviri_read_hrit(indir, timeslot, satnum, &seviri, n_bands, band_ids,
          bounds, line0, line1, column0, column1, lat0, lat1, lon0, lon1, rss,
          iodc)) {
          fprintf(stderr, "ERROR: seviri_read()\n");
          return -1;
     }

     if (seviri_preproc(&seviri, preproc, band_units, rss, do_gsics, satposstr,
                        do_not_alloc)) {
         fprintf(stderr, "ERROR: seviri_preproc()\n");
          return -1;
     }

     /* We cannot use seviri_free as not all data was read from the HRIT file
        (missing headers).  So manually free image data instead. */
     for (i = 0; i < seviri.image.n_bands; ++i) {
          if (seviri.image.band_ids[i] < 12)
               free(seviri.image.data_vir[i]);
          else
               proc_hrv = 1;
     }

     free(seviri.image.data_vir);

     if (proc_hrv == 1)
          free(seviri.image.data_hrv);

     return 0;
}


/*******************************************************************************
 * Helper function that examines if input file is HRIT or NAT and calls the
 * appropriate processing functions.
 *
 * filename	: Native SEVIRI level 1.5 filename
 * preproc	: The struct containing the preprocessed output
 * n_bands	: Described in the seviri_read_nat() header (read_write_nat.c)
 * band_ids	:      ''
 * band_units	: Array of band_unit types of length n_bands
 * bounds	: Described in the seviri_read_nat() header (read_write_nat.c)
 * line0	:      ''
 * line1	:      ''
 * column0	:      ''
 * column1	:      ''
 * lat0		:      ''
 * lat1		:      ''
 * lon0		:      ''
 * lon1		:      ''
 * do_not_alloc	: Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_read_and_preproc(const char *filename,
                            struct seviri_preproc_data *preproc,
                            uint n_bands, const uint *band_ids,
                            const enum seviri_units *band_units,
                            enum seviri_bounds bounds,
                            uint line0, uint line1, uint column0, uint column1,
                            double lat0, double lat1, double lon0, double lon1,
                            int do_gsics, char satposstr[128], int do_not_alloc)
{
     char *indir;
     int satnum;
     int rss;
     int iodc;
     char timeslot[13];

     if (strstr(filename, ".nat") != NULL) {
          if (seviri_read_and_preproc_nat(filename, preproc, n_bands, band_ids,
               band_units, bounds, line0, line1, column0, column1, lat0, lat1,
               lon0, lon1, do_gsics, satposstr, do_not_alloc)) {
               fprintf(stderr, "ERROR: seviri_read_and_preproc_nat()\n");
               return -1;
          }
     }
     else {
          if ((indir = extract_path_sat_id_timeslot(filename, &satnum, timeslot,
               &rss, &iodc)) == NULL) {
               fprintf(stderr, "ERROR: extract_path_sat_id_timeslot()\n");
               return -1;
          }

          if (seviri_read_and_preproc_hrit(indir, timeslot, satnum, preproc,
               n_bands, band_ids, band_units, bounds, line0, line1, column0,
               column1, lat0, lat1, lon0, lon1, rss, iodc, do_gsics, satposstr,
               do_not_alloc)) {
               fprintf(stderr, "ERROR: seviri_read_and_preproc_hrit()\n");
               return -1;
          }

          free(indir);
     }

     return 0;
}



/*******************************************************************************
 * Free memory allocated by seviri_preproc() for the pre-processing output in a
 * struct seviri_preproc_data type.
 *
 * preproc	: The struct containing the preprocessed data
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_preproc_free(struct seviri_preproc_data *d)
{
     if (d->memory_alloc_d) {
          free(d->time);
          free(d->lat);
          free(d->lon);
          free(d->sza);
          free(d->saa);
          free(d->vza);
          free(d->vaa);
     }

     free(d->data);

     if (d->memory_alloc_d)
          free(d->data2);

     return 0;
}



/*******************************************************************************
 * Convenience function that returns the number of lines and columns in the
 * image to be read with the given choice of offset and dimension parameters.
 *
 * filename	: Native SEVIRI level 1.5 filename
 * i_line	: Output line offset to the beginning of the actual image
 *                within the full disk
 * i_column	: Output column offset to the beginning of the actual image
 *                within the full disk
 * n_lines	: Output number of lines of the actual image
 * n_columns	: Output number of columns of the actual image
 * bounds	: Described in the seviri_read_nat() header.
 * line0	:      ''
 * line1	:      ''
 * column0	:      ''
 * column1	:      ''
 * lat0		:      ''
 * lat1		:      ''
 * lon0		:      ''
 * lon1		:      ''
 * aux		: Seviri_auxillary_io_data struct containing information related
 *                to the read operation
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_get_dimens(const char *filename, uint *i_line, uint *i_column,
                      uint *n_lines, uint *n_columns, enum seviri_bounds bounds,
                      uint line0, uint line1, uint column0, uint column1,
                      double lat0, double lat1, double lon0, double lon1)
{
     char *indir;
     int satnum;
     char timeslot[13];
     int rss;
     int iodc;

     if (strstr(filename, ".nat") != NULL) {
          if (seviri_get_dimens_nat(filename, i_line, i_column, n_lines,
               n_columns, bounds, line0, line1, column0, column1, lat0, lat1,
               lon0, lon1)) {
               fprintf(stderr, "ERROR: seviri_get_dimens_nat()\n");
               return -1;
          }
     }
     else {
          if ((indir = extract_path_sat_id_timeslot(filename, &satnum, timeslot,
               &rss, &iodc)) == NULL) {
               fprintf(stderr, "ERROR: extract_path_sat_id_timeslot()\n");
               return -1;
          }

          if (seviri_get_dimens_hrit(indir, timeslot, satnum, i_line, i_column,
               n_lines, n_columns, bounds, line0, line1, column0, column1, lat0,
               lat1, lon0, lon1, rss)) {
               fprintf(stderr, "ERROR: seviri_get_dimens_hrit()\n");
               return -1;
          }

          free(indir);
     }

     return 0;
}
