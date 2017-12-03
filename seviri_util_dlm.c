/*******************************************************************************
 *
 *    Copyright (C) 2014-2017 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

/* ANSI */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>

/* IDL */
#include "export.h"

/* Local */
#include "seviri_util.h"
#include "seviri_util_dlm.h"


#ifdef __IDLPRE53__
    /* FOR IDL < 5.3 */
    /* Define the procedures */
    static IDL_SYSFUN_DEF seviri_util_procedures[] = {
        {(IDL_FUN_RET) seviri_preproc_dlm, "SEVIRI_PREPROC_DLM", 4, 6,
         IDL_SYSFUN_DEF_F_KEYWORDS},
    };
#else
    /* FOR IDL >= 5.3 */
    /* Define the procedures */
    static IDL_SYSFUN_DEF2 seviri_util_procedures[] = {
        {(IDL_FUN_RET) seviri_preproc_dlm, "SEVIRI_PREPROC_DLM", 4, 6,
         IDL_SYSFUN_DEF_F_KEYWORDS, 0},
    };
#endif


/* Startup call when DLM is loaded */
int IDL_Load(void)
{
     /* IDL version 5.3 and greater use IDL_SYSFUN_DEF2 while earlier versions
        use IDL_SYSFUN_DEF.  Note the addition of the final '0' in each line for
        IDL_SYSFUN_DEF2. */

     /* If IDL is pre-5.3 then change IDL_SysRtnAdd to IDL_AddSystemRoutine,
        (NB: the parameters stay the same) */

#ifdef __IDLPRE53__
     /* FOR IDL < 5.3 */
     /* Add procedures */
     if (! IDL_AddSystemRoutine(seviri_util_procedures, FALSE,
                                ARRLEN(seviri_util_procedures))) {
          return IDL_FALSE;
     }

#else
     /* FOR IDL >= 5.3 */
     /* Add procedures */
     if (! IDL_SysRtnAdd       (seviri_util_procedures, FALSE,
                                ARRLEN(seviri_util_procedures))) {
          return IDL_FALSE;
     }

#endif

     /* Register the error handler */
     IDL_ExitRegister(seviri_util_exit_handler);
     return(IDL_TRUE);
}


/* Called when IDL is shutdown */
void seviri_util_exit_handler(void)
{
/* Nothing special to do in this case */
}


static int bounds_string_to_enum(const char *s, enum seviri_bounds *bounds) {

     char temp[128];

     if (strcmp(s, "full_disk") == 0)
          *bounds = SEVIRI_BOUNDS_FULL_DISK;
     else if (strcmp(s, "actual_image") == 0)
          *bounds = SEVIRI_BOUNDS_ACTUAL_IMAGE;
     else if (strcmp(s, "line_column") == 0)
          *bounds = SEVIRI_BOUNDS_LINE_COLUMN;
     else if (strcmp(s, "lat_lon") == 0)
          *bounds = SEVIRI_BOUNDS_LAT_LON;
     else {
          snprintf(temp, 128, "ERROR: invalid bounds type: %s", s);
          fprintf(stderr, "%s\n", temp);
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, temp);
          return -1;
     }

     return 0;
};


static int unit_string_to_enum(const char *s, enum seviri_units *unit) {

     char temp[128];

     if (strcmp(s, "Radiance") == 0)
          *unit = SEVIRI_UNIT_RAD;
     else if (strcmp(s, "BRF") == 0)
          *unit = SEVIRI_UNIT_BRF;
     else if (strcmp(s, "BT") == 0)
          *unit = SEVIRI_UNIT_BT;
     else {
          snprintf(temp, 128, "ERROR: invalid unit type: %s", s);
          fprintf(stderr, "%s\n", temp);
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, temp);
          return -1;
     }

     return 0;
};


/* Wrapped Fortran routines below this point */

void IDL_CDECL seviri_preproc_dlm(int argc, IDL_VPTR argv[], char *argk)
{
     char *filename;

     int i;

     uint n_bands;

     uint band_ids[SEVIRI_N_BANDS];

     enum seviri_units band_units[SEVIRI_N_BANDS];

     enum seviri_bounds bounds;

     uint pixel_coords[4];

     double lat_lon_coords[4];


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     IDL_VPTR out_argv[1];

     static int pixel_coords_flag;
     static short pixel_coords_data[4];
     static IDL_KW_ARR_DESC pixel_coords_desc   = {(char *) pixel_coords_data,   4, 4, 0};

     static int lat_lon_coords_flag;
     static float lat_lon_coords_data[4];
     static IDL_KW_ARR_DESC lat_lon_coords_desc = {(char *) lat_lon_coords_data, 4, 4, 0};

     static IDL_KW_PAR kw_pars[] = {
          IDL_KW_FAST_SCAN,
          {"LAT_LON_COORDS", IDL_TYP_FLOAT, 1, IDL_KW_ARRAY, &lat_lon_coords_flag, IDL_CHARA(lat_lon_coords_desc)},
          {"PIXEL_COORDS",   IDL_TYP_INT,   1, IDL_KW_ARRAY, &pixel_coords_flag,   IDL_CHARA(pixel_coords_desc)},
          {NULL}
     };

     IDL_KWCleanup(IDL_KW_MARK);

     IDL_KWGetParams(argc, argv, argk, kw_pars, out_argv, 1);


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     IDL_ENSURE_STRING(argv[0])
     filename = IDL_VarGetString(argv[0]);

     IDL_ENSURE_ARRAY (argv[1]);
     n_bands = argv[1]->value.arr->n_elts;
     if (n_bands > SEVIRI_N_BANDS)
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: to many band Ids given, max = SEVIRI_N_BANDS");
     if (argv[1]->type != IDL_TYP_INT)
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: band Ids must be an array of ints");
     for (i = 0; i < n_bands; ++i)
          band_ids[i] = ((short *) argv[1]->value.arr->data)[i];

     IDL_ENSURE_ARRAY (argv[2]);
     if (n_bands != argv[2]->value.arr->n_elts)
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: number of band ids and number of units do not match");
     if (argv[2]->type != IDL_TYP_STRING)
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: band units must be an array of strings");
     for (i = 0; i < n_bands; ++i)
          unit_string_to_enum(((IDL_STRING *) argv[2]->value.arr->data)[i].s, &band_units[i]);

     IDL_ENSURE_STRING(argv[3])
     bounds_string_to_enum(IDL_VarGetString(argv[3]), &bounds);


     if (pixel_coords_flag && lat_lon_coords_flag)
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: cannot use both the \"pixel_coords\" and \"lat_lon_coords\" keywords");

     if (bounds == SEVIRI_BOUNDS_FULL_DISK ||
         bounds == SEVIRI_BOUNDS_ACTUAL_IMAGE) {
          if (pixel_coords_flag || lat_lon_coords_flag)
               IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: cannot use the \"pixel_coords\" or \"lat_lon_coords\" keywords with \"full_disk\" or \"actual_image\" bounds");
     }
     else if (bounds == SEVIRI_BOUNDS_LINE_COLUMN) {
          if (! pixel_coords_flag)
               IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: must use the \"pixel_coords\" keyword with \"line_column\" bounds");
          for (i = 0; i < 4; ++i)
               pixel_coords[i] = pixel_coords_data[i];
     }
     else if (bounds == SEVIRI_BOUNDS_LAT_LON) {
          if (! lat_lon_coords_flag)
               IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: must use the \"lat_lon_coords\" keyword with \"lat_lon\" bounds");
          for (i = 0; i < 4; ++i)
               lat_lon_coords[i] = lat_lon_coords_data[i];
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     struct seviri_preproc_data preproc;

     if (seviri_read_and_preproc(filename, &preproc, n_bands, band_ids, band_units, bounds, pixel_coords[0], pixel_coords[1], pixel_coords[2], pixel_coords[3], lat_lon_coords[0], lat_lon_coords[1], lat_lon_coords[2], lat_lon_coords[3], 0))
          IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "ERROR: seviri_read_and_preproc()");


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     void *s;

     static IDL_MEMINT one = 1;

     static IDL_MEMINT filename_length;
     filename_length = 1024;
/*
     filename_length = strlen(filename) + 1;
*/
     static IDL_MEMINT dims_filename[2];
     dims_filename[0] = 1;
     dims_filename[1] = filename_length;

     static IDL_MEMINT dims_data_2[2];
     dims_data_2[0] = preproc.n_lines;
     dims_data_2[1] = preproc.n_columns;

     static IDL_MEMINT dims_data_3[3];
     dims_data_3[0] = preproc.n_bands;
     dims_data_3[1] = preproc.n_lines;
     dims_data_3[2] = preproc.n_columns;

     static IDL_STRUCT_TAG_DEF s_tags[] = {
          {"FILENAME",   dims_filename, (void *) IDL_TYP_STRING},
          {"I_LINE",     NULL,          (void *) IDL_TYP_INT},
          {"I_COLUMN",   NULL,          (void *) IDL_TYP_INT},
          {"N_LINES",    NULL,          (void *) IDL_TYP_INT},
          {"N_COLUMNS",  NULL,          (void *) IDL_TYP_INT},
          {"FILL_VALUE", NULL,          (void *) IDL_TYP_DOUBLE},
          {"TIME",       dims_data_2,   (void *) IDL_TYP_FLOAT},
/*
          {"LAT",        dims_data_2,   (void *) IDL_TYP_FLOAT},
          {"LON",        dims_data_2,   (void *) IDL_TYP_FLOAT},
          {"SZA",        dims_data_2,   (void *) IDL_TYP_FLOAT},
          {"SAA",        dims_data_2,   (void *) IDL_TYP_FLOAT},
          {"VZA",        dims_data_2,   (void *) IDL_TYP_FLOAT},
          {"VAA",        dims_data_2,   (void *) IDL_TYP_FLOAT},
          {"DATA",       dims_data_3,   (void *) IDL_TYP_FLOAT},
*/
          {NULL}
     };

     s = IDL_MakeStruct("seviri_preproc_data", s_tags);

     typedef struct data_struct {
          IDL_STRING filename[1024];
          IDL_INT i_line;
          IDL_INT i_column;
          IDL_INT n_lines;
          IDL_INT n_columns;
          float fill_value;
          IDL_VPTR *time;
/*
          float *lat;
          float *lon;
          float *sza;
          float *saa;
          float *vza;
          float *vaa;
          float *data;
*/
     } DATA_STRUCT;

     IDL_VPTR v;

     static DATA_STRUCT s_data;

     s_data.n_lines   = 2;
     s_data.n_columns = 4;

     s_data.time = malloc(preproc.n_bands * preproc.n_lines * preproc.n_columns * sizeof(float));

     v = IDL_ImportArray(1, &one, IDL_TYP_STRUCT, (UCHAR *) &s_data, 0, s);

     IDL_VarCopy(v, argv[4]);


     /*-------------------------------------------------------------------------
      * Cleanup any temporaries due to the keyword and deallocate the remain
      * temporary arrays.
      *-----------------------------------------------------------------------*/
     IDL_KWCleanup(IDL_KW_CLEAN);


     return;
}
