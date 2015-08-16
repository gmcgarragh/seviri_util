/******************************************************************************%
**
**    Copyright (C) 2015 Simon Proud <simon.proud@physics.ox.ac.uk>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
**    This file contains the main interface to the SEVIRI reading functions.
**    It also contains the functions used to save data in the various formats.
**
*******************************************************************************/

#include "SEVIRI_util.h"
#include <tiffio.h>
#include <netcdf.h>
#include <hdf5.h>

/******************************************************************************%
      Wrapper for the native reader. Converts the driver info into something
      that the reader can understand.
      Inputs:
            driver:      Structure containing the driver info
            preproc:     Main structure that will contain the SEVIRI data
      Outputs:
            integer:     Returns 0 if successful, otherwise -1

*******************************************************************************/
int run_sev_native(struct driver_data driver,struct seviri_preproc_data *preproc)
{
     if (seviri_read_and_preproc(driver.infdir,preproc, driver.sev_bands.nbands, driver.sev_bands.band_ids,
     driver.outtype, driver.bounds,driver.iline, driver.fline, driver.icol, driver.fcol,0., 0., 0., 0., 0))
     {E_L_R();}
     return 0;
}

/******************************************************************************%
      Wrapper for the HRIT reader. Converts the driver info into something
      that the reader can understand.
      Inputs:
            driver:      Structure containing the driver info
            preproc:     Main structure that will contain the SEVIRI data
      Outputs:
            integer:     Returns 0 if successful, otherwise -1

*******************************************************************************/
int run_sev_hrit(struct driver_data driver,struct seviri_preproc_data *preproc)
{
     if (seviri_read_and_preproc_hrit(driver.infdir,driver.timeslot,driver.satnum, preproc, driver.sev_bands.nbands, driver.sev_bands.band_ids,
     driver.outtype, driver.bounds,driver.iline, driver.fline, driver.icol, driver.fcol,0., 0., 0., 0., 0))
     {E_L_R();}
     return 0;
}

/******************************************************************************%
      Initialises the array that will contain output data for the TIFF writer
      All elements of the output array will contain the fill value.
      Inputs:
            outline:     The float array that will contain the data
            sizer:       Size of the output array
            filler:      The fill value to be placed into each array element
      Outputs:
            integer:     Returns 0 if successful

*******************************************************************************/
static int init_outline(float* outline,unsigned int sizer,float filler)
{
     int i=0;
     for (i=0;i<sizer;i++)outline[i]=filler;
     return 0;
}

/******************************************************************************%
      Writes the processed SEVIRI data (and any ancilliary data) into a TIFF
      file. All data is saved as floating point type.
      Inputs:
            driver:     The float array that will contain the data
            preproc:     Main structure that will contain the SEVIRI data
      Outputs:
            integer:     Returns 0 if successful, otherwise -1

*******************************************************************************/
int save_sev_tiff(struct driver_data driver,struct seviri_preproc_data preproc)
{
     char outstr[2048];
     int i=0, j=0, nbands=preproc.n_bands, k=0,band=0;
     long int col=0;

     for (i=0;i<7;i++)if (driver.ancsave[i]==1)nbands+=1;

     TIFF *tif=TIFFOpen(driver.outf, "w");
/*     Set up the tags for the TIFF file, ensures correct size and number of bands.*/
     TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, preproc.n_columns);
     TIFFSetField(tif, TIFFTAG_IMAGELENGTH, preproc.n_lines);
     TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, nbands);
     TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 32);
     TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT,SAMPLEFORMAT_IEEEFP);
     TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
     TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
     TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
     if (driver.compression==1) TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
     if (driver.infrmt==SEVIRI_INFILE_HRIT) sprintf(outstr,"Image created with the SEVIRI reader utility. This file was produced from MSG%i data in timeslot %s.",driver.satnum,driver.timeslot);
     else if (driver.infrmt==SEVIRI_INFILE_NAT) sprintf(outstr,"Image created with the SEVIRI reader utility. This file was produced from %s.",driver.infdir);
     else  sprintf(outstr,"Image created with the SEVIRI reader utility.");
     TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, outstr);
/*     Begin saving the actual image data */
     float oneline[nbands*preproc.n_columns];
     for (i=0;i<preproc.n_lines;i++) {
          if (init_outline(oneline,nbands*preproc.n_columns,preproc.fill_value)!=0) {E_L_R();}

          k=0;
          for (j=0;j<nbands*preproc.n_columns;j=j+nbands) {
               band=0;
               col = (i*preproc.n_columns)+k;
               for (band=0;band<preproc.n_bands;band++) oneline[j+band]=(float)preproc.data[band][col];
               if(driver.ancsave[0]==1){oneline[j+band]=(float)preproc.time[col];band++;}
               if(driver.ancsave[1]==1){oneline[j+band]=(float)preproc.lat[col];band++;}
               if(driver.ancsave[2]==1){oneline[j+band]=(float)preproc.lon[col];band++;}
               if(driver.ancsave[3]==1){oneline[j+band]=(float)preproc.sza[col];band++;}
               if(driver.ancsave[4]==1){oneline[j+band]=(float)preproc.saa[col];band++;}
               if(driver.ancsave[5]==1){oneline[j+band]=(float)preproc.vza[col];band++;}
               if(driver.ancsave[6]==1){oneline[j+band]=(float)preproc.vaa[col];band++;}
               k=k+1;
          }
          if (TIFFWriteScanline(tif, oneline, i, 0) !=1) {E_L_R();}
     }
     TIFFClose(tif);
     return 0;
}

/******************************************************************************%
      Writes the processed SEVIRI data (and any ancilliary data) into a NetCDF
      file. All data is saved as floating point type, aside from "Time" (double)
      Inputs:
            driver:     The float array that will contain the data
            preproc:     Main structure that will contain the SEVIRI data
      Outputs:
            integer:     Returns 0 if successful, otherwise -1

*******************************************************************************/
int save_sev_cdf(struct driver_data driver,struct seviri_preproc_data preproc)
{

     static float cnt_range[]     = {0.0, 1024.0};
     static float rad_range[]     = {0.0, 2000.0};
     static float brf_range[]     = {-0.5, 5.0};
     static float bt_range[]          = {150.0, 400.0};

     static float latlon_range[]     = {-90.0, 90.0};
     static float zen_range[]     = {-180.0, 180.0};
     static float azi_range[]     = {0.0, 360.0};

     static char title_cnt[]          = "SEVIRI data in raw count format.";
     static char title_rad[]          = "SEVIRI data in radiance format";
     static char title_brf[]          = "SEVIRI data in solar reflectance format";
     static char title_bt[]          = "SEVIRI data in brightness temperature format";

     int nbands=preproc.n_bands,i=0;
     for (i=0;i<7;i++)if (driver.ancsave[i]==1)nbands+=1;
     int ncid, x_dimid, y_dimid;
     int dimids[2];
     int *varid;
     varid     =     (int*) malloc(sizeof(int)*nbands);

/*     Create the NetCDF file and initialise the data*/
     if(nc_create(driver.outf, NC_CLOBBER|NC_NETCDF4 , &ncid)) {E_L_R();};
     if(nc_def_dim(ncid, "x", preproc.n_lines, &x_dimid)) {E_L_R();};
     if(nc_def_dim(ncid, "y", preproc.n_columns, &y_dimid)) {E_L_R();};
     dimids[0] = x_dimid;
     dimids[1] = y_dimid;


/*     Initialise each variable, loop first over all bands included in the preproc data*/
     for (i=0;i<preproc.n_bands;i++) {
          if(nc_def_var(ncid, bnames[driver.sev_bands.band_ids[i-1]], NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          if(nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value)) {E_L_R();};
          if (driver.outtype[i] == SEVIRI_UNIT_CNT){if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, cnt_range)) {E_L_R();};if(nc_put_att_text (ncid, NC_GLOBAL, "title",strlen(title_cnt), title_cnt)) {E_L_R();};}
          if (driver.outtype[i] == SEVIRI_UNIT_RAD){if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, rad_range)) {E_L_R();};if(nc_put_att_text (ncid, NC_GLOBAL, "title",strlen(title_rad), title_rad)) {E_L_R();};}
          if (driver.outtype[i] == SEVIRI_UNIT_BRF){if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, brf_range)) {E_L_R();};if(nc_put_att_text (ncid, NC_GLOBAL, "title",strlen(title_brf), title_brf)) {E_L_R();};}
          if (driver.outtype[i] == SEVIRI_UNIT_BT){if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, bt_range)) {E_L_R();};if(nc_put_att_text (ncid, NC_GLOBAL, "title",strlen(title_bt), title_bt)) {E_L_R();};}
     }

/*     Now initialise the ancilliary data*/
     if(driver.ancsave[0]==1) {
          if(nc_def_var(ncid, "Time", NC_DOUBLE, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          i++;
     }
     if(driver.ancsave[1]==1) {
          if(nc_def_var(ncid, "Latitude", NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value);
          if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, latlon_range)) {E_L_R();};
          i++;
     }
     if(driver.ancsave[2]==1) {
          if(nc_def_var(ncid, "Longitude", NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value);
          if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, latlon_range)) {E_L_R();};
          i++;
     }
     if(driver.ancsave[3]==1) {
          if(nc_def_var(ncid, "Solar Zenith Angle", NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value);
          if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, zen_range)) {E_L_R();};
          i++;
     }
     if(driver.ancsave[4]==1) {
          if(nc_def_var(ncid, "Solar Azimuth Angle", NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value);
          if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, azi_range)) {E_L_R();};
          i++;
     }
     if(driver.ancsave[5]==1) {
          if(nc_def_var(ncid, "View Zenith Angle", NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value);
          if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, zen_range)) {E_L_R();};
          i++;
     }
     if(driver.ancsave[6]==1) {
          if(nc_def_var(ncid, "View Azimuth Angle", NC_FLOAT, 2,dimids, &varid[i])) {E_L_R();};
          if (driver.compression==1) if(nc_def_var_deflate(ncid, varid[i], 1,1,2)) {E_L_R();};
          i++;
          nc_put_att_float(ncid, varid[i], "_FillValue",NC_FLOAT, 1, &preproc.fill_value);
          if(nc_put_att_float(ncid, varid[i], "valid_range",NC_FLOAT, 2, azi_range)) {E_L_R();};
     }

     if(nc_enddef(ncid)) {E_L_R();};

/*     This will actually put the data into the file*/
     for (i=0;i<preproc.n_bands;i++)
          if(nc_put_var_float(ncid, varid[i], &preproc.data[i][0])) {E_L_R();};

     if(driver.ancsave[0]==1){if(nc_put_var_double(ncid, varid[i], &preproc.time[0])) {E_L_R();};i++;}
     if(driver.ancsave[1]==1){if(nc_put_var_float(ncid, varid[i], &preproc.lat[0])) {E_L_R();};i++;}
     if(driver.ancsave[2]==1){if(nc_put_var_float(ncid, varid[i], &preproc.lon[0])) {E_L_R();};i++;}
     if(driver.ancsave[3]==1){if(nc_put_var_float(ncid, varid[i], &preproc.sza[0])) {E_L_R();};i++;}
     if(driver.ancsave[4]==1){if(nc_put_var_float(ncid, varid[i], &preproc.saa[0])) {E_L_R();};i++;}
     if(driver.ancsave[5]==1){if(nc_put_var_float(ncid, varid[i], &preproc.vza[0])) {E_L_R();};i++;}
     if(driver.ancsave[6]==1){if(nc_put_var_float(ncid, varid[i], &preproc.vaa[0])) {E_L_R();};i++;}

/*     Now we are done, so close the file*/
     if(nc_close(ncid)) {E_L_R();};

     return 0;
}

/******************************************************************************%
      Writes the processed SEVIRI data (and any ancilliary data) into an HDF5
      file. All data is saved as floating point type, aside from "Time" (double)
      Inputs:
            driver:     The float array that will contain the data
            preproc:     Main structure that will contain the SEVIRI data
      Outputs:
            integer:     Returns 0 if successful, otherwise -1

*******************************************************************************/
int save_sev_hdf(struct driver_data driver,struct seviri_preproc_data preproc)
{
/*
     static float cnt_range[]     = {0.0, 1024.0};
     static float rad_range[]     = {0.0, 2000.0};
     static float brf_range[]     = {-0.5, 5.0};
     static float bt_range[]          = {150.0, 400.0};

     static float latlon_range[]     = {-90.0, 90.0};
     static float zen_range[]     = {-180.0, 180.0};
     static float azi_range[]     = {0.0, 360.0};

     static char title_cnt[]          = "SEVIRI data in raw count format.";
     static char title_rad[]          = "SEVIRI data in radiance format";
     static char title_brf[]          = "SEVIRI data in solar reflectance format";
     static char title_bt[]          = "SEVIRI data in brightness temperature format";
*/
     int nbands=preproc.n_bands,i=0;
     for (i=0;i<7;i++)if (driver.ancsave[i]==1)nbands+=1;
     int *varid;
     varid     =     (int*) malloc(sizeof(int)*nbands);
     varid = varid;

/*     Create the HDF5 file and initialise the data*/
     hid_t      outfile;
     outfile = H5Fcreate(driver.outf,H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
     herr_t      status;
     hid_t     dataspace,dataset,dcpl;
     hsize_t dims[2]={preproc.n_lines,preproc.n_columns};

     hsize_t     chunk[2] = {(dims[0]/100), (dims[1]/100)};

     status = status;

/*     Set up some basic properties common to all the datasets*/
     dcpl = H5Pcreate (H5P_DATASET_CREATE);
     if (driver.compression==1) status = H5Pset_shuffle(dcpl);
     if (driver.compression==1) status = H5Pset_deflate(dcpl, 2);
     status = H5Pset_chunk (dcpl, 2, chunk);
     H5Pset_fill_value(dcpl, H5T_NATIVE_FLOAT, &preproc.fill_value);

/*     Set up dataspaces for the SEVIRI band data and write to the file.*/
     for (i=0;i<preproc.n_bands;i++) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,bnames[driver.sev_bands.band_ids[i-1]],H5T_NATIVE_FLOAT,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.data[i][0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
     }

/*     Set up dataspaces for the ancilliary data and write to the file.*/
     if(driver.ancsave[0]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"Time",H5T_NATIVE_DOUBLE,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_DOUBLE,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.time[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }
     if(driver.ancsave[1]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"Latitude",H5T_NATIVE_FLOAT,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.lat[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }
     if(driver.ancsave[2]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"Longitude",H5T_NATIVE_FLOAT,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.lon[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }
     if(driver.ancsave[3]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"Solar Zenith Angle",H5T_NATIVE_FLOAT,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.sza[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }
     if(driver.ancsave[4]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"Solar Azimuth Angle",H5T_NATIVE_DOUBLE,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.saa[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }
     if(driver.ancsave[5]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"View Zenith Angle",H5T_NATIVE_FLOAT,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.vza[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }
     if(driver.ancsave[6]==1) {
          dataspace=H5Screate_simple(2,dims,dims);
          dataset=H5Dcreate2(outfile,"View Azimuth Angle",H5T_NATIVE_FLOAT,dataspace,H5P_DEFAULT,dcpl,H5P_DEFAULT);
          status=H5Dwrite(dataset,H5T_NATIVE_FLOAT,H5S_ALL,H5S_ALL,H5P_DEFAULT,&preproc.vaa[0]);
          status=H5Sclose(dataspace);
          status=H5Dclose(dataset);
          i++;
     }

/*     Now we are done, so close the file*/
     status=H5Fclose(outfile);
     return 0;
}
