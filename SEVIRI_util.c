/*******************************************************************************
 *
 *    Copyright (C) 2015-2018 Simon Proud <simon.proud@physics.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 *******************************************************************************
 *
 *   This program will read a NAT or HRIT file and save the data into
 *   a specific output format, one of HDF5, NetCDF or TIFF.
 *   A text file is used as a driver to describe the read/write.
 *   The name of the driver file should be given as the argument when
 *   running this program. e.g: ./SEVIRI_tool driver_file_name
 *
 *   Text file format:
 *   Line 1,  Input data format: HRIT or NAT
 *   Line 2,  Input directory (if HRIT) or input file (if NAT)
 *   Line 3,  Timeslot (if HRIT, YYYYMMDDHHMM) or blank (if NAT)
 *   Line 4,  Satellite number (if HRIT, 1/2/3/4) or blank (if nat)
 *   Line 5,  Bands to read (HRIT and NAT) in format: 11010100100
 *            1 = read this band, 0 = do not read this band
 *            max length: 11 chars. If shorter, defaults to 0 for missing bands
 *   Line 6,  Output data format: HDF, CDF or TIF
 *   Line 7,  Output data type: CNT, RAD or RBT (for count, radiance, refl/bt)
 *   Line 8,  Output directory
 *   Line 9,  Initial line
 *   Line 10, Final line
 *   Line 11, Initial column
 *   Line 12, Final column
 *            If final vals < initial vals: Program will quit
 *            If initial vals are < 0:      Program assumes initial vals = 0
 *            If final vals are > 3711:     Program assumes final vals = 3711
 *   Special values:
 *            Initial line = -200 means actual scanned image will be processed
 *            Initial line = -100 means full disk will be processed
 *   Subsequent lines,
 *            The remaining lines can be used to define optional outputs
 *            These are: time, lat, lon, sza, saa, vza, vaa
 *            List one of these per line to save the appropriate variable
 *            Adding a line with 'compress' to the driver will compress
 *            the output data, saves disk space at expense of read/write time
 *            calib will apply GSICS calibration coefficients.
 *
 *
 *******************************************************************************
 *   Example file:
 *   HRT
 *   $HOME/my_data/RAW_SEVIRI_data/
 *   200603031200
 *   1
 *   11100011011
 *   HDF
 *   RBT
 *   $HOME/my_data/Processed_SEVIRI_data/
 *   500
 *   1000
 *   1000
 *   2000
 *   lat
 *   lon
 *   vza
 *   vaa
 *   compress
 *   calib
 *
 *   This example will read HRIT data from 12:00 UTC on 3rd March 2006
 *   Will save in the HDF5 file format
 *   Will convert into reflectance or brightness temp (depending on channel)
 *   Will work on a 500x1000 segment of the image
 *   As well as saving channels the viewing angles (vza + vaa) and
 *   the geoinfo (lat + lon) will be saved.
 *   The output file will be compressed to save disk space.
 *
 ******************************************************************************/

#include "SEVIRI_util.h"

int main(int argc, char *argv[])
{
     struct driver_data driver;

     /* This struct will contain the image data and some metadata. */
     struct seviri_preproc_data preproc;

     /* Char array to store information required for parallax correction */
     char satposstr[128];

     /* Parse the input driver file into the driver structure */
     if (argc!=2){show_usage();exit(-1);}
     if (parse_driver(argv[1],&driver)!=0) {E_L_R();}
     if (VERBOSE) if (print_driver(driver)!=0) {E_L_R();}

     /* Run the appropriate processing chain, HRIT or NAT */
     if (driver.infrmt==SEVIRI_INFILE_HRIT) if (run_sev_hrit(driver,&preproc, satposstr)!=0) {E_L_R();}
     if (driver.infrmt==SEVIRI_INFILE_NAT) if (run_sev_native(driver,&preproc, satposstr)!=0) {E_L_R();}

     /* If we're in verbose mode then print info about a sample pixel in the preprocessed data
        By default we'll examine the central pixel in the image */
     if (VERBOSE) if (print_preproc_out(driver, preproc, preproc.n_lines/2-1, preproc.n_columns/2-1)!=0) {E_L_R();}

     if (driver.outfrmt==SEVIRI_OUTFILE_HDF)if (save_sev_hdf(driver,preproc)!=0) {E_L_R();}
     if (driver.outfrmt==SEVIRI_OUTFILE_CDF)if (save_sev_cdf(driver,preproc)!=0) {E_L_R();}
     if (driver.outfrmt==SEVIRI_OUTFILE_TIF)if (save_sev_tiff(driver,preproc)!=0) {E_L_R();}

     if (free_driver(&driver)!=0) {E_L_R();}
     /* Free memory allocated by seviri_read_and_preproc(). */
     if (seviri_preproc_free(&preproc)!=0) {E_L_R();}

     return 0;
}
