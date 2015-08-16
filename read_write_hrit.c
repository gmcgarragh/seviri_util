/******************************************************************************%
**
**    Copyright (C) 2015 Simon Proud (simon.proud@physics.ox.ac.uk)
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include "external.h"
#include "hrit_anc_funcs.h"
#include "internal.h"
#include "read_write.h"
#include "read_write_hrit.h"


/*******************************************************************************
 * Read the EPI file associated with a SEVIRI timeslot. This file contains
 * information on the actual image scan
 *
 * fname:	Name of the EPI file to be read (including full directory)
 * d:		Main SEVIRI data structure
 * aux:		Auxiliary data structure
 *
 * returns:	Zero if successful, nonzero if error
 ******************************************************************************/
int read_hrit_epilogue(const char *fname, struct seviri_data *d,
                       struct seviri_auxillary_io_data *aux)
{
     FILE *fp;
     int out;
     const unsigned int probeSize = 4;
     long hdrlen = 10;
     unsigned char probeBuf[probeSize];

     /* Open epilogue */
     if ((fp = fopen(fname, "rb")) == NULL) {
          fprintf(stderr, "ERROR: Problem opening file for reading: %s ... %s\n",
                  fname, strerror(errno));
          return -1;
     }

     fseek(fp,0,SEEK_SET);
/*
     if (seviri_15HEADER_SatelliteStatus_read(fp, &d->header.SatelliteStatus, aux)) E_L_R();
*/
     /* Read first part of prologue and check contents to see if correct filetype. */
     out = fread((char *) probeBuf, probeSize,1,fp);
     out = out;

     if (probeBuf[0] == 0 && probeBuf[1] == 0 && probeBuf[2] == 16) {
          if (fxxxx_swap(&hdrlen, 4, 1,  fp, aux) < 0) {E_L_R();}
     }
     fseek(fp,hdrlen+1,SEEK_SET);
     if (fxxxx_swap(&d->trailer.ImageProductionStats.SatelliteID,          sizeof(short), 1,  fp, aux) < 0) {E_L_R();}

     if (fxxxx_swap(&d->trailer.ImageProductionStats.NominalImageScanning, sizeof(uchar), 1,  fp, aux) < 0) {E_L_R();}
     if (fxxxx_swap(&d->trailer.ImageProductionStats.ReducedScan,          sizeof(uchar), 1,  fp, aux) < 0) {E_L_R();}
     if (seviri_TIME_CDS_SHORT_read(fp, &d->trailer.ImageProductionStats.ActScanForwardStart,     aux))     {E_L_R();}
     if (seviri_TIME_CDS_SHORT_read(fp, &d->trailer.ImageProductionStats.ActScanForwardEnd,       aux))     {E_L_R();}

     fclose(fp);

     return 0;
}



/*******************************************************************************
 * Read the PRO file associated with a SEVIRI timeslot. This file contains
 * information on the planned image scan and calibration
 *
 * fname:	Name of the PRO file to be read (including full directory)
 * d:		Main SEVIRI data structure
 * aux:		Auxiliary data structure
 *
 * returns:	Zero if successful, nonzero if error
 ******************************************************************************/
int read_hrit_prologue(const char *fname, struct seviri_data *d,
                       struct seviri_auxillary_io_data *aux)
{
     FILE *fp;
     int out;
     const unsigned int probeSize = 4;
     long hdrlen = 10;
     unsigned char probeBuf[probeSize];

     /* Open prologue*/
     if ((fp = fopen(fname, "rb")) == NULL) {
          fprintf(stderr, "ERROR: Problem opening file for reading: %s ... %s\n",
                  fname, strerror(errno));
          return -1;
     }

     fseek(fp,0,SEEK_SET);

     /* Read first part of prologue and check contents to see if correct filetype*/
     out = fread((char *)probeBuf, probeSize,1,fp);
     out = out;

     if (probeBuf[0] == 0 && probeBuf[1] == 0 && probeBuf[2] == 16)
          if (fxxxx_swap(&hdrlen, 4, 1, fp, aux) < 0) {E_L_R();}
     fseek(fp, hdrlen, SEEK_SET);

     seviri_15HEADER_SatelliteStatus_read(fp,&d->header.SatelliteStatus, aux);

     /* Seek to the beginning of the useful part of the prologue (only care
        about image desc and radiometric calibration. This seek value should be
        constant, has not changed in 11 years of MSG data. */
     fseek(fp, 386982, SEEK_SET);

     /* Read the image description data */
     if (seviri_15HEADER_ImageDescription_read     (fp, &d->header.ImageDescription,      aux)) {E_L_R();}
     /* Read the per-channel calibration data */
     if (seviri_15HEADER_RadiometricProcessing_read(fp, &d->header.RadiometricProcessing, aux)) {E_L_R();}

     fclose(fp);

     return 0;
}

/*******************************************************************************
 * Allocate the space in memory to be used for the SEVIRI image. Space is
 * allocated for the HRV data is required, although it is unsupported.
 *
 * nbands:	Number of bands to be read (NOT number of bands in file)
 * band_ids:	Array of band ids (channel numbers)
 * d:		Main SEVIRI data structure
 *
 * returns:	Zero if successful, nonzero if error
 ******************************************************************************/
int alloc_imagearr(uint nbands, const uint *band_ids, struct seviri_data *d)
{
     int i,proc_hrv = 0,virb;
     long length_vir,length_hrv;
     virb = nbands;

     for (i = 0; i < nbands; ++i) {
          d->image.band_ids[i]=band_ids[i];
          if (band_ids[i]==12) {
               virb -= 1;
               proc_hrv = 1;
          }
     }

     d->image.n_bands    = nbands;
     d->image.fill_value = FILL_VALUE_US;

     length_vir = d->header.ImageDescription.ReferenceGridVIS_IR.NumberOfLines *
          d->header.ImageDescription.ReferenceGridVIS_IR.NumberOfColumns;
     length_hrv = d->header.ImageDescription.ReferenceGridHRV.NumberOfLines    *
          d->header.ImageDescription.ReferenceGridHRV.NumberOfColumns;

     if (virb<=0) return -1;

     d->image.data_vir = malloc(virb * sizeof(ushort *));
     for (i = 0; i < virb; ++i) {
          d->image.data_vir[i] = malloc(length_vir * sizeof(float));
          snu_init_array_us(d->image.data_vir[i], length_vir, d->image.fill_value);
     }
     if (proc_hrv==1) d->image.data_hrv = malloc(length_hrv * sizeof(float));

     return 0;
}



/*******************************************************************************
 * Main function for reading the HRIT data
 * Assembles the filenames, reads ancillary data, reads bands, tidies up.
 *
 * indir:	Directory containing the HRIT data
 * timeslot:	Timeslot to be read. Format: YYYYMMDDHHMM
 * sat:		Satellite number - can be 1, 2, 3 or 4
 * d:		Main SEVIRI data structure
 * nbands:	Number of bands to be read (NOT number of bands in file)
 * band_ids:	Array of band ids (channel numbers)
 * bounds:	Boundaries of the data to be read
 * line0:	First line to read
 * line1:	Last line to read
 * column0:	First column to read
 * column1:	Last column to read
 * lat0:	Initial latitude
 * lat1:	Final latitude
 * lon0:	Initial longitude
 * lon1:	Final longitude
 *
 * returns:	Zero if successful, nonzero if error
 ******************************************************************************/
int seviri_read_hrit(const char *indir, const char *timeslot, int sat,
     struct seviri_data *d, uint n_bands, const uint *band_ids,
     enum seviri_bounds bounds, uint line0, uint line1, uint column0,
     uint column1, double lat0, double lat1, double lon0, double lon1)
{
     long int out,i,j;
     char *proname;
     char *epiname;
     char ***bnames;

     struct seviri_auxillary_io_data aux;
     struct seviri_dimension_data *dimens;

     /* Get the names of the prologue, epilogue and data files. */
     /* NOTE: Only supports full disk scanning. RSS unavailable! */
     out = assemble_proname(&proname, indir, timeslot, sat);
     if (out != 0) {
          fprintf(stderr, "ERROR: assemble_proname()\n");
          return -1;
     }
     out = assemble_epiname(&epiname,indir,timeslot, sat);
     if (out != 0) {
          fprintf(stderr, "ERROR: assemble_epiname()\n");
          return -1;
     }
     out = assemble_fnames(&bnames,indir,timeslot, n_bands, band_ids,sat);
     if (out != 0) {
          fprintf(stderr, "ERROR: assemble_fnames()\n");
          return -1;
     }

     /* Set up the aux data struct and check endianness */
     seviri_auxillary_alloc(&aux);
     aux.operation  = 0;
     aux.swap_bytes = snu_is_little_endian();

     /* Read the prologue file */
     read_hrit_prologue(proname,d,&aux);
     /* Read the epilogue file */
     read_hrit_epilogue(epiname,d,&aux);

     /* Put image info in the correct place. This is done for consistency with
        the .nat reader. HRIT files contain different data, so need to move
        things around. */
     sprintf(d->marf_header.secondary.NumberLinesVISIR.Value,"%i",   IMAGE_SIZE_VIR_LINES);
     sprintf(d->marf_header.secondary.NumberColumnsVISIR.Value,"%i", IMAGE_SIZE_VIR_COLUMNS);
     sprintf(d->marf_header.secondary.NumberLinesHRV.Value,"%i",     IMAGE_SIZE_HRV_LINES);
     sprintf(d->marf_header.secondary.NumberColumnsHRV.Value,"%i",   IMAGE_SIZE_HRV_COLUMNS);

     sprintf(d->marf_header.secondary.SouthLineSelectedRectangle.Value,  "%i",1);
     sprintf(d->marf_header.secondary.NorthLineSelectedRectangle.Value,  "%i",IMAGE_SIZE_VIR_LINES);
     sprintf(d->marf_header.secondary.EastColumnSelectedRectangle.Value, "%i",1);
     sprintf(d->marf_header.secondary.WestColumnSelectedRectangle.Value, "%i",IMAGE_SIZE_VIR_COLUMNS);


     /* Allocate and fill in the seviri_dimension_data struct. */

     d->image.dimens = malloc(sizeof(struct seviri_dimension_data));
     dimens = (struct seviri_dimension_data *) d->image.dimens;

     if (seviri_get_dimension_data(dimens, &d->marf_header, bounds, line0, line1,
                                   column0, column1, lat0, lat1, lon0, lon1)) {
          fprintf(stderr, "ERROR: seviri_get_dimension_data()\n");
          return -1;
     }

     /* Put image info in the correct place. This is done for consistency with
        the .nat reader. HRIT files contain different data, so need to move
        things around. */
     d->image.i_line     = dimens->i_line_requested_VIR;
     d->image.i_column   = dimens->i_column_requested_VIR;

     d->image.n_lines    = dimens->n_lines_requested_VIR;
     d->image.n_columns  = dimens->n_columns_requested_VIR;

     d->image.packet_header = NULL;
     d->image.LineSideInfo  = NULL;

     out = alloc_imagearr(n_bands, band_ids,d);

     /* Loop over each band and each segment. Note: VIR only, no HRV */
     for (i = 0; i < n_bands; i++) {
          for (j = 0; j < 8; j++)
               read_data_oneseg(bnames[i][j], j, i+1, d, dimens);
     }

     /* Tidy up */
     seviri_auxillary_free(&aux);
     free(proname);
     free(epiname);
     for (i = 0; i < n_bands; i++) {
          for (j = 0; j < 8 ; j++)
               free(bnames[i][j]);
          free(bnames[i]);
     }
     free(bnames);

     return 0;
}
