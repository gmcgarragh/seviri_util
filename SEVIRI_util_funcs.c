/*******************************************************************************
*
*    Copyright (C) 2015-2018 Simon Proud <simon.proud@physics.ox.ac.uk>
*
*    This source code is licensed under the GNU General Public License (GPL),
*    Version 3.  See the file COPYING for more details.
*
*    This file contains helper code for the main SEVIRI utility.
*    These functions parse input, display options and perform most of
*    the set-up operations prior to actually parsing the SEVIRI data
*
*******************************************************************************/

#include "SEVIRI_util.h"

/* Sets up the band names. good for telling which band is which, easier than
   channel number */

const char *bnames[] = {"VIS006", "VIS008", "IR_016", "IR_039", "WV_062",
                        "WV_073", "IR_087", "IR_097", "IR_108", "IR_120",
                        "IR_134"};

/* Prints a message that shows how to use the utility. */
void show_usage()
{
     printf("To run this program please use:\n\t./SEVIRI_tool <filename>\n");
     printf("Where <filename> points to a driver file containing the following lines:\n");
     printf("\tLine 1,  Input data format: HRIT or NAT\n");
     printf("\tLine 2,  Input directory (if HRIT) or input file (if NAT)\n");
     printf("\tLine 3,  Timeslot (if HRIT, YYYYMMDDHHMM) or blank (if NAT)\n");
     printf("\tLine 4,  Satellite number (if HRIT, 1/2/3/4) or blank (if nat)\n");
     printf("\tLine 5,  Bands to read (HRIT and NAT) in format: 11010100100\n");
     printf("\t\t 1 = read this band, 0 = do not read this band\n");
     printf("\t\t max length: 11 chars. If shorter, defaults to 0 for missing bands\n");
     printf("\tLine 6,  Output data format: HDF, CDF or TIF\n");
     printf("\tLine 7,  Output data type: CNT, RAD or RBT (for count, radiance, refl/bt)\n");
     printf("\tLine 8,  Output directory\n");
     printf("\tLine 9,  Initial line\n");
     printf("\tLine 10, Final line\n");
     printf("\tLine 11, Initial column\n");
     printf("\tLine 12, Final column\n");
     printf("\t\t If final vals < initial vals: Program will quit\n");
     printf("\t\t If initial vals are < 0:      Program assumes inital vals = 0\n");
     printf("\t\t If final vals are > 3711:     Program assumes final vals = 3711\n");
     printf("Will now exit!\n");
}

/* Parses the input band character string (in format 010101010110) and converts
   it into a list of bands to process. */
static void parsebands(char *bands, struct bands_st *inbands)
{
     int len       = strlen(bands);
     int recval    = 11;
     int i         = 0;
     int act_bands = 0;
     if (len<11) recval=len;
     recval = recval;

     inbands->band_ids = malloc(sizeof(unsigned int));
     for (i=0;i<len;i++) {
          if (strcmp(&bands[i],"1")!=-1) {
               act_bands++;
               inbands->band_ids = (unsigned int*) realloc (inbands->band_ids, act_bands * sizeof(unsigned int));
               inbands->band_ids[act_bands-1]=i+1;
          }
     }
     inbands->nbands = act_bands;

     return;
}

/*******************************************************************************
 *    Utility function to assist with debugging.
 *    This prints all the driver information to the terminal
 *    Useful for checking that the driver file was parsed correctly
 *    Inputs:
 *          driver:     The driver structure
 *    Outputs:
 *          integer:     Zero if successful, otherwise 1
 ******************************************************************************/
int print_driver(struct driver_data driver)
{
     /* Sets up the band names. Good for telling which band is which, easier
        than channel number. */

     int i;
     char outstr[40];

     /* Print basic data about the driver */
     printf("\n******************************* DEBUG INFO *******************************\n");
     printf("Input data type:\t\t%i\n",driver.infrmt);
     printf("Input directory or file:\t%s\n",driver.infdir);
     if (driver.infrmt==SEVIRI_INFILE_HRIT)printf("SEVIRI timeslot:\t\t%s\n",driver.timeslot);
     if (driver.infrmt==SEVIRI_INFILE_HRIT)printf("MSG satellite number:\t\t%i\n",driver.satnum);
     printf("Number of bands to process:\t%i\n",driver.sev_bands.nbands);
     strcpy(outstr,"\0");

     /* Print info about which bands will be processed */
     printf("Will process bands and units:\n");
     for (i=0;i<driver.sev_bands.nbands;i++) {
          printf("\t\t\t\t%s\t%i\n",bnames[driver.sev_bands.band_ids[i]-1],driver.outtype[i]);
     }
     printf("Output format will be:\t\t%i\n",driver.outfrmt);
     printf("Output directory will be:\t%s\n",driver.outf);
     printf("SEVIRI image boundaries are:\t%i\n",driver.bounds);
     printf("Will process between lines:\t%i\t%i\n",driver.iline,driver.fline);
     printf("Will process between columns:\t%i\t%i\n",driver.icol,driver.fcol);
     strcpy(outstr,"\0");

     /* Print info about which ancilliary data is to be saved */
     if (driver.ancsave[0]==1)strcat(outstr,"time ");
     if (driver.ancsave[1]==1)strcat(outstr,"lat ");
     if (driver.ancsave[2]==1)strcat(outstr,"lon ");
     if (driver.ancsave[3]==1)strcat(outstr,"sza ");
     if (driver.ancsave[4]==1)strcat(outstr,"saa ");
     if (driver.ancsave[5]==1)strcat(outstr,"vza ");
     if (driver.ancsave[6]==1)strcat(outstr,"vaa ");
     printf("Will save this ancilliary data:\t%s\n",outstr);

     if (driver.compression==1 && driver.outfrmt==SEVIRI_OUTFILE_TIF)printf("The output file will be compressed with LZW\n");
     if (driver.compression!=1 && driver.outfrmt==SEVIRI_OUTFILE_TIF)printf("The output file will not be compressed\n");
     if (driver.compression==1 && driver.outfrmt==SEVIRI_OUTFILE_CDF)printf("The output file will be compressed with shuffle and deflate level 2\n");
     if (driver.compression!=1 && driver.outfrmt==SEVIRI_OUTFILE_CDF)printf("The output file will not be compressed\n");
     if (driver.compression==1 && driver.outfrmt==SEVIRI_OUTFILE_HDF)printf("The output file will be compressed with shuffle and deflate level 2\n");
     if (driver.compression!=1 && driver.outfrmt==SEVIRI_OUTFILE_HDF)printf("The output file will not be compressed\n");
     if (driver.do_calib==1)printf("The GSICS calibration coefficients will be applied.\n");
     if (driver.do_calib!=1)printf("The GSICS calibration coefficients will NOT be applied.\n");

     printf("**************************************************************************\n\n");
     return 0;
}

/* Sets lines and cols to zero in case of FULL/ACTUAL image reading */
static void setline(struct driver_data *driver)
{
     driver->iline=0;
     driver->icol=0;
     driver->fline=0;
     driver->fcol=0;
}

/*******************************************************************************
 *    Prints information on the preprocessing for one pixel.
 *    Useful to see if the data read was successful
 *    Inputs:
 *          preproc:     The main data structure
 *          i_line:     Which line of data do we examine?
 *          i_column:     Which column within the line do we examine?
 *    Outputs:
 *          integer:     Zero if successful, otherwise 1
 ******************************************************************************/
int print_preproc_out(struct driver_data driver, struct seviri_preproc_data preproc,
                      unsigned int i_line, unsigned int i_column)
{
     int i;

     unsigned int i_pixel;

     printf("****************************** PREPROC INFO ******************************\n");

     /* Print the values for the central pixel. */

     i_pixel  = i_line * preproc.n_columns + i_column;

     printf("i_line:                %d\n", i_line);
     printf("i_column:              %d\n", i_column);
     printf("i_pixel:               %d\n", i_pixel);
     printf("Julian Day Number:     %f\n", preproc.time[i_pixel]);
     printf("latitude:              %f\n", preproc.lat [i_pixel]);
     printf("longitude:             %f\n", preproc.lon [i_pixel]);
     printf("solar zenith angle:    %f\n", preproc.sza [i_pixel]);
     printf("solar azimuth angle:   %f\n", preproc.saa [i_pixel]);
     printf("viewing zenith angle:  %f\n", preproc.vza [i_pixel]);
     printf("viewing azimuth angle: %f\n", preproc.vaa [i_pixel]);

     printf("\n");

     /* Print the image values for each band that was processed. */

     for (i = 0; i < driver.sev_bands.nbands; ++i)
          printf("%s:                %08.4f\n",
                 bnames[driver.sev_bands.band_ids[i]-1], preproc.data[i][i_pixel]);

     printf("**************************************************************************\n\n");

     return 0;
}

/*******************************************************************************
 *    Frees the memory used by the driver script
 *    Inputs:
 *         driver:      The driver struct
 *    Outputs:
 *         integer:     Zero if success, otherwise -1
 ******************************************************************************/
int free_driver(struct driver_data *driver)
{
     free(driver->infdir);
     if (driver->infrmt==SEVIRI_INFILE_HRIT) free(driver->timeslot);
     free(driver->sev_bands.band_ids);
     free(driver->outtype);
     free(driver->outf);
     return 0;
}

/*******************************************************************************
 *    Parser for the driver file.
 *    Inputs:
 *         fname:     Driver filename
 *         driver:      Structure that will contain the parsed data
 *    Outputs:
 *         integer:     Zero if success, otherwise -1
 ******************************************************************************/
int parse_driver(char *fname,struct driver_data *driver)
{
     int i;
     int iline;
     size_t len=0;
     char *line = NULL;

     FILE *fp = fopen(fname,"r");
     if (fp == NULL) {printf("Unable to open the driver file: %s\n",fname);E_L_R();}

     /* Read the type of input file from line 1 of the driver */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading HRIT line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     if (!strcmp(line,"HRIT")) driver->infrmt = SEVIRI_INFILE_HRIT;
     else if (!strcmp(line,"NAT")) driver->infrmt = SEVIRI_INFILE_NAT;
     else {printf("Incorrect input type in driver file. Must be HRIT or NAT.\n");free(line);fclose(fp);E_L_R();}

     /* Read the filename / input file directory. */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading input file/dir line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     if (strlen(line)<4) {printf("Failure reading input file/dir line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     driver->infdir = (char*) malloc(sizeof(char)*(strlen(line)+1));
     strcpy(driver->infdir,line);
     if (driver->infrmt==SEVIRI_INFILE_HRIT) {
		 /* Read the timeslot (HRIT only) */
		 if (getline(&line,&len,fp)==-1) {printf("Failure reading input timeslot line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
		 if (strlen(line)<12 && driver->infrmt==SEVIRI_INFILE_HRIT) {printf("Failure reading input timeslot line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
		 line[strlen(line)-1]='\0';
		 driver->timeslot = (char*) malloc(sizeof(char)*13);
		 strcpy(driver->timeslot,line);

		 /* Read the satellite number (HRIT only) */
		 if (getline(&line,&len,fp)==-1) {printf("Failure reading input satellite number line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
		 if (strlen(line)<1 && driver->infrmt==SEVIRI_INFILE_HRIT) {printf("Failure reading input satellite number line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
         line[strlen(line)-1]='\0';
	     if (atoi(line)==1) driver->satnum  = SAT_MSG1+1;
	     else if (atoi(line)==2) driver->satnum = SAT_MSG2+1;
	     else if (atoi(line)==3) driver->satnum = SAT_MSG3+1;
	     else if (atoi(line)==4) driver->satnum = SAT_MSG4+1;
	     else {printf("Failure reading input satellite number line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     }

     /* Read the rss mode */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading RSS flag line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     if (strlen(line)<1 && driver->infrmt==SEVIRI_INFILE_HRIT) {printf("Failure reading RSS flag line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     if (atoi(line)==0) driver->rss = 0;
     else if (atoi(line)==1) driver->rss = 1;
     else {printf("Failure reading RSS flag line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}

     /* Read the iodc mode */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading IODC flag line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     if (strlen(line)<1 && driver->infrmt==SEVIRI_INFILE_HRIT) {printf("Failure reading IODC flag line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     if (atoi(line)==0) driver->iodc = 0;
     else if (atoi(line)==1) driver->iodc = 1;
     else {printf("Failure reading IODC flag line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}

     /* Read the bands to process */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading bands to process line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     parsebands(line,&driver->sev_bands);
     driver->outtype = (enum seviri_units*) malloc(sizeof(enum seviri_units)*driver->sev_bands.nbands);
     if (driver->outtype == NULL) {printf("Failure reading bands to process line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     if (driver->sev_bands.nbands<=0 || driver->sev_bands.nbands>11) {printf("Failure reading bands to process line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}

     /* Read the output file type */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading output file type line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     if (!strcmp(line,"HDF")) driver->outfrmt = SEVIRI_OUTFILE_HDF;
     else if (!strcmp(line,"CDF")) driver->outfrmt = SEVIRI_OUTFILE_CDF;
     else if (!strcmp(line,"TIF")) driver->outfrmt = SEVIRI_OUTFILE_TIF;
     else {printf("Failure reading output file type line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}

     /* Read the output units type */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading output units type line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     if (!strcmp(line,"CNT")) for (i=0;i<driver->sev_bands.nbands;i++) driver->outtype[i] = SEVIRI_UNIT_CNT;
     else if (!strcmp(line,"RAD")) for (i=0;i<driver->sev_bands.nbands;i++) driver->outtype[i] = SEVIRI_UNIT_RAD;
     else if (!strcmp(line,"RBT")) for (i=0;i<driver->sev_bands.nbands;i++) if (driver->sev_bands.band_ids[i]<=3) driver->outtype[i] = SEVIRI_UNIT_BRF; else driver->outtype[i] = SEVIRI_UNIT_BT;
     else {printf("Failure reading output units type line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}

     /* Read the output filename. */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading output filename line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     len = strlen(line);
     if (len<4) {printf("Failure reading utput filename line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     if (len > 0 && line[len-1] == '\n') line[len-1] = '\0';
     driver->outf = (char*) malloc(sizeof(char)*(len+5));
     strcpy(driver->outf,line);
     if (driver->outfrmt == SEVIRI_OUTFILE_HDF)strcat(driver->outf,".h5");
     if (driver->outfrmt == SEVIRI_OUTFILE_CDF)strcat(driver->outf,".nc");
     if (driver->outfrmt == SEVIRI_OUTFILE_TIF)strcat(driver->outf,".tiff");

     /* Read the initial line */
     if (getline(&line,&len,fp)==-1) {printf("Failure reading input initial line line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
     line[strlen(line)-1]='\0';
     iline = atoi(line);
     driver->bounds = SEVIRI_BOUNDS_LINE_COLUMN;
     if (iline==-100) { driver->bounds = SEVIRI_BOUNDS_FULL_DISK; setline(driver);}
     if (iline==-200) { driver->bounds = SEVIRI_BOUNDS_ACTUAL_IMAGE; setline(driver);}
     if (iline!=-100 && iline!=-200) {
          driver->iline = iline;
          /* Read the final line */
          if (getline(&line,&len,fp)==-1) {printf("Failure reading input final line line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
          line[strlen(line)-1]='\0';
          driver->fline = atoi(line);
          /* Read the initial column */
          if (getline(&line,&len,fp)==-1) {printf("Failure reading input initial column line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
          line[strlen(line)-1]='\0';
          driver->icol = atoi(line);
          /* Read the final column */
          if (getline(&line,&len,fp)==-1) {printf("Failure reading input final column line of driver file %s\n",fname);free(line);fclose(fp);E_L_R();}
          line[strlen(line)-1]='\0';
          driver->fcol = atoi(line);

          /* Check image boundaries to make sure the start and end line / col
             are sensible. */
          if (driver->iline<0) driver->iline=0;
          if (driver->icol<0) driver->icol=0;
          if (driver->fline>3711) driver->fline=3711;
          if (driver->fcol>3711) driver->fcol=3711;

          if (driver->iline>3711) {printf("The initial processing line cannot be greater than 3711. You used: %i\n",driver->iline);free(line);fclose(fp);E_L_R();}
          if (driver->icol>3711) {printf("The initial processing column cannot be greater than 3711. You used: %i\n",driver->icol);free(line);fclose(fp);E_L_R();}
          if (driver->fline<0) {printf("The final processing line cannot be less than 0. You used: %i\n",driver->fline);free(line);fclose(fp);E_L_R();}
          if (driver->fcol<0) {printf("The final processing column cannot be less than 0. You used: %i\n",driver->fcol);free(line);fclose(fp);E_L_R();}

          if (driver->icol>=driver->fcol) {printf("The initial processing column cannot be greater than the final processing column. You used: %i and %i\n",driver->icol,driver->fcol);free(line);fclose(fp);E_L_R();}
          if (driver->iline>=driver->fline) {printf("The initial processing line cannot be greater than the final processing line. You used: %i and %i\n",driver->iline,driver->fline);free(line);fclose(fp);E_L_R();}
          if (driver->fline>3711) driver->fline=3711;
          if (driver->fcol>3711) driver->fcol=3711;
     }

     /* Read the anciliary data line.*/
     /* ancsave contains: 0-time, 1-lat, 2-lon, 3-sza, 4-saa, 5-vza, 6-vaa */
     driver->compression=0;
     for (i=0;i<7;i++) driver->ancsave[i]=0;
     while (fgets(line, 10, fp)) {
          line[strlen(line)-1]='\0';
          if (strcmp(line,"time")==0)    driver->ancsave[0]=1;
          if (strcmp(line,"compress")==0)driver->compression=1;
          if (strcmp(line,"lat")==0)     driver->ancsave[1]=1;
          if (strcmp(line,"lon")==0)     driver->ancsave[2]=1;
          if (strcmp(line,"sza")==0)     driver->ancsave[3]=1;
          if (strcmp(line,"saa")==0)     driver->ancsave[4]=1;
          if (strcmp(line,"vza")==0)     driver->ancsave[5]=1;
          if (strcmp(line,"vaa")==0)     driver->ancsave[6]=1;
          if (strcmp(line,"calib")==0)
               driver->do_calib=1;
          else
               driver->do_calib=0;
     }
     fclose(fp);

     return 0;
}
