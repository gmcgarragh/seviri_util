/******************************************************************************%
**
**    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#include "external.h"
#include "hrit_anc_funcs.h"
#include "internal.h"
#include "read_write.h"
#include "read_write_nat.h"


/*******************************************************************************
 * SIZE constants useful for skipping around.
 ******************************************************************************/
/*
#define UMARF_HEADER_SIZE	5114
*/
#define PACKET_HEADER_SIZE	38
#define LINE_SIDE_INFO_SIZE	27
/*
#define _15HEADER_SIZE		445248
#define _15TRAILER_SIZE		380325
*/


/*******************************************************************************
 * Read a VIS/IR line record structure - the actual image data.
 *
 * fp		: Pointer to the image data file set to the beginning of the
 *              : line record structure.
 * image	: The output seviri_image_data struct with the image data
 * marf_header	: The seviri_marf_header_data struct for the current image data
 *                file.
 * n_bands	: Described in the seviri_read_nat() header
 * band_ids	: 	''
 * bounds	: 	''
 * line0	: 	''
 * line1	: 	''
 * column0	: 	''
 * column1	: 	''
 * lat0		: 	''
 * lat1		: 	''
 * lon0		: 	''
 * lon1		: 	''
 * aux		: Seviri_auxillary_io_data struct containing information related
 *                to the read operation
 *
 * returns	: Non-zero on error
 ******************************************************************************/
static int seviri_image_read(FILE *fp, struct seviri_image_data *image,
                             const struct seviri_marf_header_data *marf_header,
                             uint n_bands, const uint *band_ids,
                             enum seviri_bounds bounds,
                             uint line0, uint line1, uint column0, uint column1,
                             double lat0, double lat1, double lon0, double lon1,
                             struct seviri_auxillary_io_data *aux)
{
     uchar *data10;

     const uchar shifts[] = {6, 4, 2, 0};

     ushort temp;

     const ushort masks[] = {0xFFC0, 0x3FF0, 0x0FFC, 0x03FF};

     uint i;
     uint ii;
     uint iii;
     uint j;
     uint jj;
     uint k;

     uint length;

     uint i_band;

     uint i_image;

     uint n_bands_VIR;
     uint n_bands_HRV;

     uint n_bytes_VIR_line;
     uint n_bytes_HRV_line;

     uint n_bytes_line_group;

     long file_start;
     long file_offset;
     long file_offset2;

     struct seviri_dimension_data *dimens;

     int i_bands_infile[12];


     /*-------------------------------------------------------------------------
      * Check if the requested band IDs are valid.
      *-----------------------------------------------------------------------*/
     image->n_bands = n_bands;

     for (i = 0; i < n_bands; ++i) {
          if (band_ids[i] < 1 || band_ids[i] > SEVIRI_N_BANDS) {
               fprintf(stderr, "ERROR: Invalid SEVIRI band Id at band list "
                               "element %d: %d\n", i, band_ids[i]);
               return -1;
          }
          image->band_ids[i] = band_ids[i];
     }

     /* Check that the caller is not expecting HRV data. */
     for (i = 0; i < n_bands; ++i) {
          if (band_ids[i] == 12) {
               fprintf(stderr, "ERROR: Reading of the HRV channel is not yet "
                               "fully supported\n");
               return -1;
          }
     }


     /*-------------------------------------------------------------------------
      * Count the number VIR and HRV bands in the file.
      *-----------------------------------------------------------------------*/
     n_bands_VIR = 0;
     for (i = 0; i < 11; ++i) {
          if (marf_header->secondary.SelectedBandIDs.Value[i] == 'X') {
               n_bands_VIR++;
          }
     }

     n_bands_HRV = 0;
     if (marf_header->secondary.SelectedBandIDs.Value[11] == 'X')
          n_bands_HRV = 1;


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     ii = 0;
     for (i = 0; i < n_bands; ++i) {
          if (marf_header->secondary.SelectedBandIDs.Value[band_ids[i] - 1] == 'X') {
               iii = -1;
               for (ii = 0; ii < band_ids[i]; ++ii) {
                    if (marf_header->secondary.SelectedBandIDs.Value[ii] == 'X')
                         iii++;
               }
               i_bands_infile[i] = iii;
          }
          else
              i_bands_infile[i] = -1;
     }


     /*-------------------------------------------------------------------------
      * Allocate and fill in the seviri_dimension_data struct.
      *-----------------------------------------------------------------------*/
     dimens = (struct seviri_dimension_data *) &image->dimens;

     if (seviri_get_dimension_data(dimens, marf_header, bounds, line0, line1,
                                   column0, column1, lat0, lat1, lon0, lon1)) {
          fprintf(stderr, "ERROR: seviri_get_dimension_data()\n");
          return -1;
     }


     /*-------------------------------------------------------------------------
      * Quantities useful for moving around in the file.
      *-----------------------------------------------------------------------*/
     n_bytes_VIR_line = PACKET_HEADER_SIZE + LINE_SIDE_INFO_SIZE +
                        dimens->n_columns_selected_VIR / 4 * 5;
     n_bytes_HRV_line = PACKET_HEADER_SIZE + LINE_SIDE_INFO_SIZE +
                        dimens->n_columns_selected_HRV / 4 * 5 / 2;

     n_bytes_line_group = n_bands_VIR * n_bytes_VIR_line +
                          n_bands_HRV * 3 * n_bytes_HRV_line;


     /*-------------------------------------------------------------------------
      * Image offsets and dimensions and the fill_value for the caller.
      *-----------------------------------------------------------------------*/
     image->i_line     = dimens->i_line_requested_VIR;
     image->i_column   = dimens->i_column_requested_VIR;

     image->n_lines    = dimens->n_lines_requested_VIR;
     image->n_columns  = dimens->n_columns_requested_VIR;

     image->fill_value = FILL_VALUE_US;


     /*-------------------------------------------------------------------------
      * Allocate memory to hold structure fields.
      *-----------------------------------------------------------------------*/
     image->packet_header = malloc(image->n_bands *
                                   sizeof(struct seviri_packet_header_data *));
     for (i = 0; i < image->n_bands; ++i)
          image->packet_header[i] = malloc(dimens->n_lines_requested_VIR *
                                           sizeof(struct seviri_packet_header_data));

     image->LineSideInfo = malloc(image->n_bands *
                                  sizeof(struct seviri_LineSideInfo_data *));
     for (i = 0; i < image->n_bands; ++i)
          image->LineSideInfo[i]  = malloc(dimens->n_lines_requested_VIR *
                                           sizeof(struct seviri_LineSideInfo_data ));

     length = dimens->n_lines_requested_VIR * dimens->n_columns_requested_VIR;

     image->data_vir = malloc(image->n_bands * sizeof(ushort *));
     for (i = 0; i < image->n_bands; ++i) {
          image->data_vir[i] = malloc(length * sizeof(float));
          snu_init_array_us(image->data_vir[i], length, image->fill_value);
     }


     /*-------------------------------------------------------------------------
      * Read the image data.
      *-----------------------------------------------------------------------*/
     data10 = malloc(dimens->n_columns_selected_VIR / 4 * 5 * sizeof(uchar));

     file_start  = ftell(fp);

     file_offset = file_start + dimens->i_line_to_read_VIR * n_bytes_line_group;

     for (i = 0; i < dimens->n_lines_to_read_VIR; ++i) {
          ii = dimens->i_line_in_output_VIR + i;

          for (i_band = 0; i_band < image->n_bands; ++i_band) {
               if (i_bands_infile[i_band] < 0)
                    continue;

               file_offset2 = file_offset + i_bands_infile[i_band] *
                    n_bytes_VIR_line + dimens->i_column_to_read_VIR / 4 * 5;

               fseek(fp, file_offset2, SEEK_SET);

               if (seviri_packet_header_read(fp, &image->packet_header[i_band][i], aux)) {
                    fprintf(stderr, "ERROR: seviri_packet_header_read()\n");
                    return -1;
               }

               if (seviri_LineSideInfo_read(fp, &image->LineSideInfo[i_band][i], aux)) {
                    fprintf(stderr, "ERROR: seviri_LineSideInfo_read()\n");
                    return -1;
               }

               if (fread(data10, sizeof(char), dimens->n_columns_to_read_VIR / 4 * 5, fp) <
                         dimens->n_columns_to_read_VIR / 4 * 5) E_L_R();

               i_image = ii * dimens->n_columns_requested_VIR + dimens->i_column_in_output_VIR;

               for (j = 0, jj = 0; j < dimens->n_columns_to_read_VIR; ) {
                    for (k = 0; k < 4; ++k) {
                         temp = *((ushort *) (data10 + jj));

                         SWAP_2(temp, temp);

                         image->data_vir[i_band][i_image + j] = (temp & masks[k]) >> shifts[k];

                         j++, jj++;
                    }

                    jj++;
               }
          }

          file_offset += n_bytes_line_group;
     }

     file_offset = file_start + dimens->n_lines_selected_VIR * n_bytes_line_group;

     fseek(fp, file_offset, SEEK_SET);


     free(data10);


     return 0;
}



/*******************************************************************************
 * Write a VIS/IR line record structure - the actual image data.
 *
 * fp		: File pointer to where the line record structure is to be
 *                written.
 * image	: The input seviri_image_data struct
 * aux		: Seviri_auxillary_io_data struct containing information related
 *                to the read operation
 *
 * returns	: Non-zero on error
 *
 ******************************************************************************/
static int seviri_image_write(FILE *fp, const struct seviri_image_data *image,
                              struct seviri_auxillary_io_data *aux)
{
     uchar *data10;

     const uchar shifts[] = {6, 4, 2, 0};

     ushort temp;

     uint i;
     uint ii;
     uint j;
     uint jj;
     uint k;

     uint i_band;

     uint i_image;

     const struct seviri_dimension_data *dimens;


     /*-------------------------------------------------------------------------
      * For convenience.
      *-----------------------------------------------------------------------*/
     dimens = (struct seviri_dimension_data *) &image->dimens;


     /*-------------------------------------------------------------------------
      * Write the image data.
      *-----------------------------------------------------------------------*/
     data10 = malloc(dimens->n_columns_selected_VIR / 4 * 5 * sizeof(uchar));

     for (i = 0; i < dimens->n_lines_to_read_VIR; ++i) {
          ii = dimens->i_line_in_output_VIR + i;

          for (i_band = 0; i_band < image->n_bands; ++i_band) {

               if (seviri_packet_header_read(fp, &image->packet_header[i_band][i], aux)) {
                    fprintf(stderr, "ERROR: seviri_packet_header_read()\n");
                    return -1;
               }

               if (seviri_LineSideInfo_read(fp, &image->LineSideInfo[i_band][i], aux)) {
                    fprintf(stderr, "ERROR: seviri_LineSideInfo_read()\n");
                    return -1;
               }

               snu_init_array_uc(data10, dimens->n_columns_to_read_VIR / 4 * 5, 0);

               i_image = ii * dimens->n_columns_requested_VIR + dimens->i_column_in_output_VIR;

               for (j = 0, jj = 0; j < dimens->n_columns_to_read_VIR; ) {
                    for (k = 0; k < 4; ++k) {
                         temp = image->data_vir[i_band][i_image + j] << shifts[k];

                         SWAP_2(temp, temp);

                         *((ushort *) (data10 + jj)) = *((ushort *) (data10 + jj)) | temp;

                         j++, jj++;
                    }

                    jj++;
               }

               if (fwrite(data10, sizeof(char), dimens->n_columns_to_read_VIR / 4 * 5, fp) <
                          dimens->n_columns_to_read_VIR / 4 * 5) E_L_R();
          }
     }


     free(data10);


     return 0;
}



/*******************************************************************************
 * Free memory allocated by seviri_image_read() to hold seviri_image_data struct
 * fields.
 *
 * image	: The input seviri_image_data struct
 *
 * returns	: Non-zero on error
 ******************************************************************************/
static int seviri_image_free(struct seviri_image_data *d)
{
     uint i;

     for (i = 0; i < d->n_bands; ++i)
          free(d->packet_header[i]);
     free(d->packet_header);

     for (i = 0; i < d->n_bands; ++i)
          free(d->LineSideInfo[i]);
     free(d->LineSideInfo);

     for (i = 0; i < d->n_bands; ++i)
          free(d->data_vir[i]);
     free(d->data_vir);
/*
     for (i = 0; i < d->n_bands; ++i)
          free(d->data_hrv[i]);
     free(d->data_hrv);
*/
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
 * line0	: 	''
 * line1	: 	''
 * column0	: 	''
 * column1	: 	''
 * lat0		: 	''
 * lat1		: 	''
 * lon0		: 	''
 * lon1		: 	''
 * aux		: Seviri_auxillary_io_data struct containing information related
 *                to the read operation
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_get_dimens_nat(const char *filename, uint *i_line, uint *i_column,
                          uint *n_lines, uint *n_columns, enum seviri_bounds bounds,
                          uint line0, uint line1, uint column0, uint column1,
                          double lat0, double lat1, double lon0, double lon1)
{
     FILE *fp;

     struct seviri_auxillary_io_data aux;

     struct seviri_dimension_data dimens;

     struct seviri_marf_header_data marf_header;

     aux.operation  = 0;
     aux.swap_bytes = snu_is_little_endian();

     seviri_auxillary_alloc(&aux);

     if ((fp = fopen(filename, "r")) == NULL) {
          fprintf(stderr, "ERROR: Problem opening file for reading: %s ... %s\n",
                  filename, strerror(errno));
          return -1;
     }

     if (seviri_marf_header_read(fp, &marf_header, &aux)) {
          fprintf(stderr, "ERROR: seviri_marf_header_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     if (seviri_get_dimension_data(&dimens, &marf_header, bounds, line0, line1,
                                   column0, column1, lat0, lat1, lon0, lon1)) {
          fprintf(stderr, "ERROR: seviri_get_dimension_data()\n");
          fclose(fp);
          return -1;
     }

     fclose(fp);

     seviri_auxillary_free(&aux);

     *i_line    = dimens.i_line_requested_VIR;
     *i_column  = dimens.i_column_requested_VIR;
     *n_lines   = dimens.n_lines_requested_VIR;
     *n_columns = dimens.n_columns_requested_VIR;

     return 0;
}



/*******************************************************************************
 * The main read function.
 *
 * filename	: Native SEVIRI level 1.5 filename
 * d		: The output seviri_data struct with the U-MARF header, level
 *                1.5 header and trailer, and the image data.
 * n_bands	: The desired number of bands to read
 * band_ids	: Array of band Ids to read of length n_bands
 * bounds	: Type of image sub-setting desired.  Valid choices are:
 *	SEVIRI_BOUNDS_FULL_DISK		: Produce a full disk image even though
 *					  the actual image may be smaller.
 *					  fill_value is used for the rest of the
 *					  image.
 *	SEVIRI_BOUNDS_ACTUAL_IMAGE	: Read the actual sub-image
 *	SEVIRI_BOUNDS_LINE_COLUMN	: Produce a sub-image based on pixel
 *					  coordinates.  fill_value is used for
 *					  when the desired image is bigger than
 *					  the actual image.
 *	SEVIRI_BOUNDS_LAT_LON		: Produce a sub-image based on lat/lon
 *					  coordinates.  fill_value is used when
 *					  the desired image is bigger than the
 *                                        actual image.
 *
 * The following are used with bounds = SEVIRI_BOUNDS_LINE_COLUMN
 * line0	: Starting line within the full disk of the desired sub-image
 * line1	: Ending line within the full disk of the desired sub-image
 * column0	: Starting column within the full disk of the desired sub-image
 * column1	: Ending column within the full disk of the desired sub-image
 *
 * The following are used with bounds = SEVIRI_BOUNDS_LAT_LON
 * lat0		: Starting latitude of the desired sub-image
 * lat1		: Ending latitude of the desired sub-image
 * lon0		: Starting longitude of the desired sub-image
 * lon1		: Ending longitude of the desired sub-image
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_read_nat(const char *filename, struct seviri_data *d,
                    uint n_bands, const uint *band_ids,
                    enum seviri_bounds bounds,
                    uint line0, uint line1, uint column0, uint column1,
                    double lat0, double lat1, double lon0, double lon1)
{
     FILE *fp;

     struct seviri_auxillary_io_data aux;

     aux.operation  = 0;
     aux.swap_bytes = snu_is_little_endian();

     seviri_auxillary_alloc(&aux);

     if ((fp = fopen(filename, "r")) == NULL) {
          fprintf(stderr, "ERROR: Problem opening file for reading: %s ... %s\n",
                  filename, strerror(errno));
          return -1;
     }

     if (seviri_marf_header_read(fp, &d->marf_header, &aux)) {
          fprintf(stderr, "ERROR: seviri_marf_header_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     if (seviri_packet_header_read(fp, &d->packet_header1, &aux)) {
          fprintf(stderr, "ERROR: seviri_packet_header_read()\n");
          return -1;
     }

     if (seviri_15HEADER_read(fp, &d->header, &aux)) {
          fprintf(stderr, "ERROR: seviri_15HEADER_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     if (seviri_image_read(fp, &d->image, &d->marf_header, n_bands, band_ids,
                           bounds, line0, line1, column0, column1, lat0, lat1,
                           lon0, lon1, &aux)) {
          fprintf(stderr, "ERROR: seviri_image_read(), filename = %s\n",
                 filename);
          fclose(fp);
          return -1;
     }

     if (seviri_packet_header_read(fp, &d->packet_header2, &aux)) {
          fprintf(stderr, "ERROR: seviri_packet_header_read()\n");
          return -1;
     }

     if (seviri_15TRAILER_read(fp, &d->trailer, &aux)) {
          fprintf(stderr, "ERROR: seviri_15TRAILER_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     fclose(fp);

     seviri_auxillary_free(&aux);

     return 0;
}



/*******************************************************************************
 * The main write function.
 *
 * filename	: Native SEVIRI level 1.5 filename
 * d		: The input seviri_data struct with the U-MARF header, level 1.5
 *                header and trailer, and the image data.
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_write_nat(const char *filename, const struct seviri_data *d)
{
     FILE *fp;

     struct seviri_auxillary_io_data aux;

     aux.operation  = 1;
     aux.swap_bytes = snu_is_little_endian();

     seviri_auxillary_alloc(&aux);

     if ((fp = fopen(filename, "w")) == NULL) {
          fprintf(stderr, "ERROR: Problem opening file for reading: %s ... %s\n",
                  filename, strerror(errno));
          return -1;
     }

     if (seviri_marf_header_read(fp, (struct seviri_marf_header_data *)
                                 &d->marf_header, &aux)) {
          fprintf(stderr, "ERROR: seviri_marf_header_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     if (seviri_packet_header_read(fp, (struct seviri_packet_header_data *)
                                   &d->packet_header1, &aux)) {
          fprintf(stderr, "ERROR: seviri_packet_header_read()\n");
          return -1;
     }

     if (seviri_15HEADER_read(fp, (struct seviri_15HEADER_data *)
                              &d->header, &aux)) {
          fprintf(stderr, "ERROR: seviri_15HEADER_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     if (seviri_image_write(fp, &d->image, &aux)) {
          fprintf(stderr, "ERROR: seviri_image_write(), filename = %s\n",
                 filename);
          fclose(fp);
          return -1;
     }

     if (seviri_packet_header_read(fp, (struct seviri_packet_header_data *)
                                   &d->packet_header2, &aux)) {
          fprintf(stderr, "ERROR: seviri_packet_header_read()\n");
          return -1;
     }

     if (seviri_15TRAILER_read(fp, (struct seviri_15TRAILER_data *)
                               &d->trailer, &aux)) {
          fprintf(stderr, "ERROR: seviri_15TRAILER_read(), filename = %s\n",
                  filename);
          fclose(fp);
          return -1;
     }

     fclose(fp);

     seviri_auxillary_free(&aux);

     return 0;
}
