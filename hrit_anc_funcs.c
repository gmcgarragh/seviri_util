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


/*******************************************************************************
 * Check if input band is HRV. If yes, 24 segments for HRIT image. Otherwise 8
 *
 * band:     Band number (1 -> 12)
 *
 * returns:     Number of segments for the given band
 ******************************************************************************/
int ishrv(int band)
{
     int segs=8;
     if (band==12) segs=24;
     return segs;
}

/*******************************************************************************
 * Helper function to build file names based upon the band
 *
 * cnum:     Channel number (1 -> 12). Note: 0 and 13 can be used for EPI+PRO
 *
 * returns:     Character array (len = 6) containing channel name string
 ******************************************************************************/
char* chan_name(int cnum)
{
     switch (cnum)
     {
          case 0:          return "______";      break;
          case 1:          return "VIS006";      break;
          case 2:          return "VIS008";      break;
          case 3:          return "IR_016";      break;
          case 4:          return "IR_039";      break;
          case 5:          return "WV_062";      break;
          case 6:          return "WV_073";      break;
          case 7:          return "IR_087";      break;
          case 8:          return "IR_097";      break;
          case 9:          return "IR_108";      break;
          case 10:     return "IR_120";      break;
          case 11:     return "IR_134";      break;
          case 12:     return "HRV___";      break;
          case 13:     return "______";      break;
          default:     return "______";      break;
     }
}

/*******************************************************************************
 *
 ******************************************************************************/
char *extract_path_sat_id_timeslot(const char *filename, int *sat_id, char *timeslot)
{
     int startfnam;
     char *ptr;
     char *indir;
     ptr=strstr(filename,"H-000-MSG");
     if (ptr==NULL){printf("Incorrectly formatted HRIT! Quitting\n");return NULL;}
     startfnam=ptr-filename;
     indir=(char*)malloc(sizeof(char)*startfnam+1);
     strncpy(indir,filename,startfnam);
     indir[startfnam] = '\0';
     int gopos=startfnam+46;
     strncpy(timeslot,filename+gopos,12);
     timeslot[12]='\0';
     *sat_id=atoi(&filename[startfnam+9]);

     return indir;
}

/*******************************************************************************
 * Builds the filenames for a given channel
 *
 * NOTE: Only supports full disk scanning. RSS and SRSS not supported
 *
 * fnam:     Output char array for the filenames. Format:
 *              Character array ([nbands][nsegs][fnames])
 * indir:     Directory that holds the HRIT file segments
 * timeslot:     Timeslot to be read. Format: YYYYMMDDHHMM
 * nbands:     Number of bands to be read (NOT number of bands in file)
 * bids:     Array of band ids (channel numbers)
 * sat:          Satellite number, can be 1, 2, 3 or 4
 *
 * returns:     Zero if successful
 ******************************************************************************/
int assemble_fnames(char ****fnam, const char *indir, const char *timeslot, int nbands, const uint *bids, int sat)
{
     int c,totsegs=0, nsegs[nbands];
     for (c=0;c<nbands;c++){ if (bids[c]==12) nsegs[c]=24; else nsegs[c]=8; totsegs+=nsegs[c];}
     char*** arr = malloc(sizeof(char *) * nbands);


     for (c=0;c<nbands;c++)
     {
          arr[c]=malloc(sizeof(char *) * nsegs[c]);
          uint cnum=bids[c];
          int i;
          char *band;
          band=chan_name(cnum);
          if (cnum>0 && cnum<=12)
          {
/*     Build the actual filenames*/
               for (i = 0; i < nsegs[c]; ++i)
               {
/*     HRIT filename is 61 so use that plus indir len*/
                    arr[c][i] = malloc(61+strlen(indir)+1);
                    sprintf(arr[c][i], "%sH-000-MSG%d__-MSG%d________-%s___-%.6d___-%s-__", indir, sat, sat, band, i+1, timeslot);
               }
          }
     }
     *fnam=arr;

     return 0;

}

/*******************************************************************************
 * Builds the epilogue filename for a timeslot
 *
 * NOTE: Only supports full disk scanning. RSS and SRSS not supported
 *
 * enam:     Output char array for the epilogue filename.
 * indir:     Directory that holds the HRIT file segments
 * timeslot:     Timeslot to be read. Format: YYYYMMDDHHMM
 * sat:          Satellite number, can be 1, 2, 3 or 4
 *
 * returns:     Zero if successful
 ******************************************************************************/
int assemble_epiname(char **enam, const char *indir, const char *timeslot, int sat)
{
/*     HRIT filename is 61 so use that plus indir len*/
     char* arr = malloc(61+strlen(indir)+1);
     char *band;
     band=chan_name(0);
     sprintf(arr, "%sH-000-MSG%d__-MSG%d________-%s___-EPI______-%s-__", indir, sat, sat, band, timeslot);

     *enam=arr;
/*     Return the completed array of filenames.*/
     return 0;

}

/*******************************************************************************
 * Builds the prologue filename for a timeslot
 *
 * NOTE: Only supports full disk scanning. RSS and SRSS not supported
 *
 * pnam:     Output char array for the epilogue filename.
 * indir:     Directory that holds the HRIT file segments
 * timeslot:     Timeslot to be read. Format: YYYYMMDDHHMM
 * sat:          Satellite number, can be 1, 2, 3 or 4
 *
 * returns:     Zero if successful
 ******************************************************************************/
int assemble_proname(char **pnam, const char *indir, const char *timeslot, int sat)
{
/*     HRIT filename is 61 so use that plus indir len*/
     char* arr = malloc(61+strlen(indir)+1);
     char *band;
     band=chan_name(0);
     sprintf(arr, "%sH-000-MSG%d__-MSG%d________-%s___-PRO______-%s-__", indir, sat, sat, band, timeslot);

     *pnam=arr;
/*     Return the completed array of filenames.*/
     return 0;

}

/*******************************************************************************
 * Reads one HRIT segment into the image memory space
 *
 * NOTE: Only supports full disk scanning. RSS and SRSS not supported
 * NOTE: Does support HRV reading, but this is untested
 *
 * fname:     The name of the file to be read
 * segnum:     The segment number to be read (1->8 / 1->24 for VIR / HRV)
 * cnum:     The channel number (1 -> 11 for VIR, 12 for HRV)
 * d:          Main SEVIRI data structure
 * dims:     Container for the image boundaries.
 *
 * returns:     Zero if successful
 ******************************************************************************/
int read_data_oneseg(char* fname,int segnum,int cnum,struct seviri_data *d,struct seviri_dimension_data *dims)
{

/*      Set up the various data that is required*/
     uchar *data10;
     const uchar shifts[] = {6, 4, 2, 0};
     ushort temp;
     const ushort masks[] = {0xFFC0, 0x3FF0, 0x0FFC, 0x03FF};
     int x,out,j,jj,k;


/*      Required to align with NAT format reader*/
/*      4 must be subtracted form the column as we read 4 pixels simultaneously*/
     int first_line=dims->i_line_requested_VIR;
     int first_col=dims->i_column_requested_VIR;

        int last_line= first_line+dims->n_lines_requested_VIR-1;
        int last_col= first_col+dims->n_columns_requested_VIR-1;

     long int out_d_col,out_d_line;
     FILE *fp;
     fp=fopen(fname,"rb");

/*     Skip header section, don't bother to read*/
     fseek(fp,6198,SEEK_SET);

     if (cnum>0 && cnum<12)     {
          int ncols= dims->n_columns_selected_VIR;

/*           Each segment if 464 lines, so skip to correct part of image based on segnum*/
          int offset=segnum*464;
          ushort tmpline[ncols];

          data10 = malloc(ncols / 4 * 5 * sizeof(uchar));
/*           Loop over all lines in segment*/
          for (x=offset;x<offset+464;x++)
          {
               out_d_line=(x-first_line)*(last_col-first_col+1);
/*                If we're outside the requested image boundary: move file pointed and skip*/
               if (x<first_line || x>last_line){fseek(fp,ncols/4*5,SEEK_CUR);continue;}

/*                OTherwise read the data and store in the image memory space*/
               out=fread(data10, sizeof(char), ncols / 4 * 5, fp);
               out=out;

               for (j = 0, jj = 0; j < ncols; ) {
                    for (k = 0; k < 4; ++k) {
                         temp = *((ushort *) (data10 + jj));
                         SWAP_2(temp, temp);
                         tmpline[j] = (temp & masks[k]) >> shifts[k];

                         if (j>=first_col && j<=last_col){
                              out_d_col=j-first_col;
                              d->image.data_vir[cnum-1][out_d_line+out_d_col]=tmpline[j];
                         }
                         j++, jj++;
                    }
                    jj++;
               }
          }
          free(data10);
     }

/*      Same as above, but for HRV*/
     if (cnum==12)     {

/*           Fudge the HRV line numbers*/
          int nlines=11136;
          int ncols=11136;
          int offset=nlines-(segnum*464)-464;
          for (x=offset+464;x>offset;x--)     {
               data10 = malloc(ncols / 4 * 5 * sizeof(uchar));
               out=fread(data10, sizeof(char), ncols / 4 * 5, fp);
               for (j = ncols, jj = 0; j > 0;) {
                    for (k = 0; k < 4; ++k) {
                         temp = *((ushort *) (data10 + jj));
                         SWAP_2(temp, temp);
                         d->image.data_hrv[(x*ncols*2)+j] = (temp & masks[k]) >> shifts[k];
                         j--, jj++;
                    }
                    jj++;
               }
          }
          free(data10);
     }
     if (cnum<=0 || cnum>12) return -1;
     fclose(fp);
     return 0;
}
