/******************************************************************************%
**
**    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
**
**    Modified by Simon Proud (simon.proud@physics.ox.ac.uk) July 2015
**    Added support for reading SEVIRI data in HRIT format.
**
*******************************************************************************/
#include "internal.h"
#include "seviri_native_util.h"


/*******************************************************************************
 * Compute Julian day from the TIME_CDS_SHORT type.
 ******************************************************************************/
static double TIME_CDS_SHORT_to_jtime(const struct seviri_TIME_CDS_SHORT_data *d) {

     double jtime_epoch = (double) snu_cal_to_jul_day(1958, 1, 1) - .5;

     return jtime_epoch + d->day + d->msec / 1.e3 / 60. / 60. / 24.;
}



/*******************************************************************************
 * Main pre-processing function which includes the computation of Julian Day,
 * latitude, longitude, solar zenith and azimuth angles, viewing zenith and
 * and azimuth angles, and either radiance, reflectance, or brightness
 * temperature for each requested channel.
 *
 * d          : The main input SEVIRI native level 1.5 struct
 * d2          : The struct containing the preprocessed output
 * band_units     : Array of band_unit types of length n_bands
 * do_not_alloc     : Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns     : Non-zero on error
 ******************************************************************************/
int seviri_native_preproc(const struct seviri_native_data *d,
                          struct seviri_preproc_data *d2,
                          const enum seviri_units *band_units, int do_not_alloc) {

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

     double jtime;
     double jtime2;

     double jtime_end;
     double jtime_start;


/*      SRP: Add in new variables to store the image start/stop time*/
     double jtime_i;
     double jtime_end_i;
     double jtime_start_i;

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

     double dd;

     double day_of_year;

     double R;

     double nu;
     double nu3;

     double L;


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
               fprintf(stderr, "ERROR: band ID %d, does not support BT units\n",
                       d->image.band_ids[i]);
               return -1;
          }

          if (( d->image.band_ids[i] >= 4 && d->image.band_ids[i] <= 11)
                                          && band_units[i] == SEVIRI_UNIT_BRF) {
               fprintf(stderr, "ERROR: band ID %d, does not support BRF units\n",
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


     snu_init_array_d(d2->time, length, d2->fill_value);
     snu_init_array_f(d2->lat,  length, d2->fill_value);
     snu_init_array_f(d2->lon,  length, d2->fill_value);
     snu_init_array_f(d2->sza,  length, d2->fill_value);
     snu_init_array_f(d2->saa,  length, d2->fill_value);
     snu_init_array_f(d2->vza,  length, d2->fill_value);
     snu_init_array_f(d2->vaa,  length, d2->fill_value);

     snu_init_array_f(d2->data2, d->image.n_bands * length, d2->fill_value);


     /*-------------------------------------------------------------------------
      * Compute the day of the year for the center of the image scan.
      *-----------------------------------------------------------------------*/
     jtime_start = TIME_CDS_SHORT_to_jtime(
          &d->trailer.ImageProductionStats.ActScanForwardStart);
     jtime_end   = TIME_CDS_SHORT_to_jtime(
          &d->trailer.ImageProductionStats.ActScanForwardEnd);

     jtime = (jtime_start + jtime_end) / 2.;

     snu_jul_to_cal_date((long) floor(jtime       + .5), &year, &month, &day);
     day_of_year = jtime       - (snu_cal_to_jul_day(year, 1, 0) - .5);

/*     SRP:     Store the start/stop times so they're not overwritten by the orbit*/
/*          polynomial values below.*/
     jtime_start_i = jtime_start;
     jtime_end_i = jtime_end;
     jtime_i = jtime;


     /*-------------------------------------------------------------------------
      * Compute the satellite position vector in Cartesian coordinates (km).
      *-----------------------------------------------------------------------*/
     for (i = 0; i < 100; ++i) {
          jtime_start = TIME_CDS_SHORT_to_jtime(
               &d->header.SatelliteStatus.OrbitPolynomial[i].StartTime);
          jtime_end   = TIME_CDS_SHORT_to_jtime(
               &d->header.SatelliteStatus.OrbitPolynomial[i].EndTime);
          if (jtime >= jtime_start && jtime <= jtime_end)
               break;
     }

     if (i == 100) {
          fprintf(stderr, "ERROR: image time is out of range of supplied orbit "
                  "polynomials\n");
          return -1;
     }

     t = (jtime - (jtime_start + jtime_end)   / 2.) /
                 ((jtime_end   - jtime_start) / 2.);

     X = d->header.SatelliteStatus.OrbitPolynomial[i].X[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].X[1] * t;
     Y = d->header.SatelliteStatus.OrbitPolynomial[i].Y[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].Y[1] * t;
     Z = d->header.SatelliteStatus.OrbitPolynomial[i].Z[0] +
         d->header.SatelliteStatus.OrbitPolynomial[i].Z[1] * t;


     /*-------------------------------------------------------------------------
      * Compute latitude and longitude and solar and sensor zenith and azimuth
      * angles.
      *-----------------------------------------------------------------------*/
     lon0 = d->header.ImageDescription.LongitudeOfSSP;

     for (i = 0; i < d->image.n_lines; ++i) {
          ii = d->image.i_line + i;

/*      SRP: Change times from orbit polynomial to image start/stop scan*/
          jtime2 = jtime_start_i + (double) ii / (double) (IMAGE_SIZE_VIR_LINES - 1) *
                   (jtime_end_i - jtime_start_i);

          for (j = 0; j < d->image.n_columns; ++j) {
               i_image = i * d->image.n_columns + j;

               snu_line_column_to_lat_lon(ii + 1, d->image.i_column + j + 1,
                                          &d2->lat[i_image], &d2->lon[i_image],
                                          lon0, &nav_scaling_factors_vir);

               if (d2->lat[i_image] != FILL_VALUE_F && d2->lon[i_image] != FILL_VALUE_F) {
                    d2->time[i_image] = jtime2;

/*     SRP: Changed jtime here too.*/
                    snu_solar_params2(jtime_i, d2->lat[i_image] * D2R,
                                      d2->lon[i_image] * D2R, &mu0, &theta0,
                                      &phi0, NULL);
                    d2->sza[i_image] = theta0 * R2D;
                    d2->saa[i_image] = phi0   * R2D;

                    d2->saa[i_image] = d2->saa[i_image] + 180.;
                    if (d2->saa[i_image] > 360.)
                         d2->saa[i_image] = d2->saa[i_image] - 360.;

                    snu_vza_and_vaa(d2->lat[i_image], d2->lon[i_image], 0.,
                                    X, Y, Z, &d2->vza[i_image], &d2->vaa[i_image]);

                    d2->vaa[i_image] = d2->vaa[i_image] + 180.;
                    if (d2->vaa[i_image] > 360.)
                         d2->vaa[i_image] = d2->vaa[i_image] - 360.;
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
               slope  = d->header.RadiometricProcessing.
                    Level1_5ImageCalibration[d->image.band_ids[i] - 1].Cal_Slope;
               offset = d->header.RadiometricProcessing.
                    Level1_5ImageCalibration[d->image.band_ids[i] - 1].Cal_Offset;

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;

                         if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                             d->image.data_vir[i][i_image] > 0) {
                              d2->data[i][i_image] =
                                   d->image.data_vir[i][i_image] * slope + offset;
                         }
                    }
               }
          }
     }


     /*-------------------------------------------------------------------------
      * Compute reflectance for the bands requested.
      *
      * Ref: PDF_MSG_SEVIRI_RAD2REFL, Page 8
      *-----------------------------------------------------------------------*/
     dd = 1. / sqrt(snu_solar_distance_factor2(day_of_year));

     a = PI * dd * dd;
     for (i = 0; i < d->image.n_bands; ++i) {
          if (band_units[i] == SEVIRI_UNIT_BRF) {
               slope  = d->header.RadiometricProcessing.
                    Level1_5ImageCalibration[d->image.band_ids[i] - 1].Cal_Slope;
               offset = d->header.RadiometricProcessing.
                    Level1_5ImageCalibration[d->image.band_ids[i] - 1].Cal_Offset;

               b = a / band_solar_irradiance[i_sat][d->image.band_ids[i] - 1];

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;

                         if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                             d->image.data_vir[i][i_image] > 0 &&
                             d2->sza[i_image] >= 0. && d2->sza[i_image] < 90.) {
                              R = d->image.data_vir[i][i_image] * slope + offset;

                              d2->data[i][i_image] =
                                   b * R / cos(d2->sza[i_image] * D2R);
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
               slope  = d->header.RadiometricProcessing.
                    Level1_5ImageCalibration[d->image.band_ids[i] - 1].Cal_Slope;
               offset = d->header.RadiometricProcessing.
                    Level1_5ImageCalibration[d->image.band_ids[i] - 1].Cal_Offset;
/*
               nu = 1.e4 / channel_center_wavelength[d->image.band_ids[i] - 1];
*/
               nu = bt_nu_c[i_sat][d->image.band_ids[i] - 1];

               nu3 = nu * nu * nu;

               a = bt_A[i_sat][d->image.band_ids[i] - 1];
               b = bt_B[i_sat][d->image.band_ids[i] - 1];

               for (j = 0; j < d->image.n_lines; ++j) {
                    for (k = 0; k < d->image.n_columns; ++k) {
                         i_image = j * d->image.n_columns + k;

                         if (d->image.data_vir[i][i_image] != FILL_VALUE_US &&
                             d->image.data_vir[i][i_image] > 0) {
                              L = d->image.data_vir[i][i_image] * slope + offset;

                              d2->data[i][i_image] =
                                   (c2 * nu / log(1. + nu3 * c1 / L) - b) / a;
                         }
                    }
               }
          }
     }

    /*-------------------------------------------------------------------------
      * Extract the raw pixel counts only. Do not scale or transform in any way
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

     return 0;
}


/*******************************************************************************
 * Convenience function that calls both seviri_native_read_hrit() and 
 * seviri_native_preproc() as this is likely the most common usage scenario.
 *
 * indir     : Input directory that contains the HRIT files
 * timeslot     : The timeslot for processing in the form YYYYMMDDHHMM
 * satnum     : The satellite ID in the form "MSGx", with x=1,2,3,4
 * preproc     : The struct containing the preprocessed output
 * n_bands     : Described in the seviri_native_read() header (read_write.c)
 * band_ids     :      ''
 * band_units     : Array of band_unit types of length n_bands
 * bounds     : Described in the seviri_native_read() header (read_write.c)
 * line0     :      ''
 * line1     :      ''
 * column0     :      ''
 * column1     :      ''
 * lat0          :      ''
 * lat1          :      ''
 * lon0          :      ''
 * lon1          :      ''
 * do_not_alloc     : Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns     : Non-zero on error
 ******************************************************************************/
int seviri_read_and_preproc_hrit(const char *indir,const char *timeslot,const int satnum,
                            struct seviri_preproc_data *preproc,
                            uint n_bands, const uint *band_ids,
                            const enum seviri_units *band_units,
                            enum seviri_bounds bounds,
                            uint line0, uint line1, uint column0, uint column1,
                            double lat0, double lat1, double lon0, double lon1,
                            int do_not_alloc) {

     struct seviri_native_data native;
     if (seviri_native_read_hrit(indir, timeslot, satnum, &native, n_bands, band_ids, bounds,
          line0, line1, column0, column1, lat0, lat1, lon0, lon1)) {
          fprintf(stderr, "ERROR: seviri_native_read()\n");
     return -1;
     }

     if (seviri_native_preproc(&native, preproc, band_units, do_not_alloc)) {
         fprintf(stderr, "ERROR: seviri_native_preproc()\n");
          return -1;
     }

/*      We cannot use seviri_native_free as not all data was*/
/*      read from the HRIT file (missing headers).*/
/*      So manually free image data instead.*/
     int i,proc_hrv=0;
     for (i = 0; i < native.image.n_bands; ++i)
          if (native.image.band_ids[i]<12)free(native.image.data_vir[i]);
          if (native.image.band_ids[i]==12)proc_hrv=1;
     free(native.image.data_vir);
     if (proc_hrv==1)free(native.image.data_hrv);
     free(native.image.dimens);

     return 0;
}

/*******************************************************************************
 * Convenience function that calls both seviri_native_read() and seviri_native_
 * preproc() as this is likely the most common usage scenario.
 *
 * filename     : Native SEVIRI level 1.5 filename
 * preproc     : The struct containing the preprocessed output
 * n_bands     : Described in the seviri_native_read() header (read_write.c)
 * band_ids     :      ''
 * band_units     : Array of band_unit types of length n_bands
 * bounds     : Described in the seviri_native_read() header (read_write.c)
 * line0     :      ''
 * line1     :      ''
 * column0     :      ''
 * column1     :      ''
 * lat0          :      ''
 * lat1          :      ''
 * lon0          :      ''
 * lon1          :      ''
 * do_not_alloc     : Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns     : Non-zero on error
 ******************************************************************************/
int seviri_read_and_preproc(const char *filename,
                            struct seviri_preproc_data *preproc,
                            uint n_bands, const uint *band_ids,
                            const enum seviri_units *band_units,
                            enum seviri_bounds bounds,
                            uint line0, uint line1, uint column0, uint column1,
                            double lat0, double lat1, double lon0, double lon1,
                            int do_not_alloc) {

     struct seviri_native_data native;

     if (seviri_native_read(filename, &native, n_bands, band_ids, bounds,
                            line0, line1, column0, column1, lat0, lat1, lon0, lon1)) {
          fprintf(stderr, "ERROR: seviri_native_read()\n");
          return -1;
     }

     if (seviri_native_preproc(&native, preproc, band_units, do_not_alloc)) {
          fprintf(stderr, "ERROR: seviri_native_preproc()\n");
          return -1;
     }

     seviri_native_free(&native);

     return 0;
}

/*******************************************************************************
 * Helper function that examines if input file is HRIT or NAT and calls the
 * appropriate processing functions.
 *
 * filename     : Native SEVIRI level 1.5 filename
 * preproc     : The struct containing the preprocessed output
 * n_bands     : Described in the seviri_native_read() header (read_write.c)
 * band_ids     :      ''
 * band_units     : Array of band_unit types of length n_bands
 * bounds     : Described in the seviri_native_read() header (read_write.c)
 * line0     :      ''
 * line1     :      ''
 * column0     :      ''
 * column1     :      ''
 * lat0          :      ''
 * lat1          :      ''
 * lon0          :      ''
 * lon1          :      ''
 * do_not_alloc     : Flag indicating not to allocate space for the output data.
 *                Useful for avoiding unnecessary memory allocations and use.
 *
 * returns     : Non-zero on error
 ******************************************************************************/
int seviri_read_and_preproc_main(const char *filename,
                            struct seviri_preproc_data *preproc,
                            uint n_bands, const uint *band_ids,
                            const enum seviri_units *band_units,
                            enum seviri_bounds bounds,
                            uint line0, uint line1, uint column0, uint column1,
                            double lat0, double lat1, double lon0, double lon1,
                            int do_not_alloc) {


	if(strstr(filename, ".nat") != NULL)
	{
		seviri_read_and_preproc(filename,preproc,n_bands,band_ids,band_units,bounds,line0,line1,column0,column1,lat0,lat1,lon0,lon1,do_not_alloc);
	}
	else 
	{
		int startfnam, satnum;
		char *ptr;
		char timeslot[13];
		char *indir;
		ptr=strstr(filename,"H-000-MSG");
		if (ptr==NULL){printf("Incorrectly formatted HRIT! Quitting\n");return -1;}
		startfnam=ptr-filename;
		indir=(char*)malloc(sizeof(char)*startfnam);
		strncpy(indir,filename,startfnam);
		int gopos=startfnam+46;
		strncpy(timeslot,filename+gopos,12);
		timeslot[12]='\0';
		satnum=atoi(&filename[startfnam+9]);
		seviri_read_and_preproc_hrit(indir,timeslot,satnum, preproc, n_bands, band_ids,band_units, bounds,line0, line1, column0, column1,lat0,lat1,lon0,lon1,do_not_alloc);
	};
     return 0;
}

/*******************************************************************************
 * Free memory allocated by seviri_native_preproc() for the pre-processing
 * output in a struct seviri_preproc_data type.
 *
 * preproc     : The struct containing the preprocessed data
 *
 * returns     : Non-zero on error
 ******************************************************************************/
int seviri_preproc_free(struct seviri_preproc_data *d) {

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
