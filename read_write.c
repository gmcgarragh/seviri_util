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
 * Error, location and return macros.
 ******************************************************************************/
#define E_L_R() do { \
     fprintf(stderr, "ERROR: file = %s, line = %d, function = %s()\n", \
             __FILE__, __LINE__, __func__); \
     return -1; \
} while (0)


#define E_L_R_MSG(MSG) do { \
     fprintf(stderr, "ERROR: file = %s, line = %d, function = %s(): %s\n", \
             __FILE__, __LINE__, __func__, MSG); \
     return -1; \
} while (0)





/*******************************************************************************
 * Byte swapping macros to convert Endianness.
 ******************************************************************************/
#define SWAP_2(x, y) do {	\
     y = (x << 8) | (x >> 8);	\
} while (0)


#define SWAP_4(x, y) do {					\
     y = ((x << 8 ) & 0xFF00FF00 ) | ((x >> 8 ) & 0x00FF00FF ); \
     y =  (y << 16)                |  (y >> 16);		\
} while (0)


#define SWAP_8(x, y) do {							\
     y = ((x << 8 ) & 0xFF00FF00FF00FF00U) | ((x >> 8 ) & 0x00FF00FF00FF00FFU);	\
     y = ((y << 16) & 0xFFFF0000FFFF0000U) | ((y >> 16) & 0x0000FFFF0000FFFFU);	\
     y =  (y << 32)                        |  (y >> 32);			\
} while (0)



/*******************************************************************************
 * Like fread() that also swaps bytes after reading as needed.
 ******************************************************************************/
int fread_swap(void *ptr, size_t size, size_t nmemb, FILE *stream,
                      struct seviri_auxillary_io_data *aux) {

     size_t i;
     size_t n;

     ushort *ptr_2;
     uint   *ptr_4;
     ulong  *ptr_8;

     n = fread(ptr, size, nmemb, stream);
     if (n < nmemb) {
          if (feof(stream))
               fprintf(stderr, "ERROR: End of file reached\n");
          else
               fprintf(stderr, "ERROR: Error reading file: %s\n",
                       strerror(errno));

          return -1;
     }

     if (aux->swap_bytes) {
          switch(size) {
               case 1:
                    break;
               case 2:
                    ptr_2 = (ushort *) ptr;
                    for (i = 0; i < nmemb; ++i)
                         SWAP_2(ptr_2[i], ptr_2[i]);
                    break;
               case 4:
                    ptr_4 = (uint *) ptr;
                    for (i = 0; i < nmemb; ++i)
                         SWAP_4(ptr_4[i], ptr_4[i]);
                    break;
               case 8:
                    ptr_8 = (ulong *) ptr;
                    for (i = 0; i < nmemb; ++i)
                         SWAP_8(ptr_8[i], ptr_8[i]);
                    break;
               default:
                    fprintf(stderr, "ERROR: Invalid element size: %ld\n", size);
                    return 0;
          }
     }

     return n;
}



/*******************************************************************************
 * Like fwrite() that also swaps bytes before writing as needed.
 ******************************************************************************/
int fwrite_swap(const void *ptr, size_t size, size_t nmemb, FILE *stream,
                       struct seviri_auxillary_io_data *aux) {

     size_t i;
     size_t n;

     ushort *ptr_2;
     uint   *ptr_4;
     ulong  *ptr_8;

     const void *ptr_temp;

     if (! aux->swap_bytes)
          ptr_temp = ptr;
     else {
          switch(size) {
               case 1:
                    ptr_temp = ptr;
                    break;
               case 2:
                    ptr_2 = (ushort *) ptr;
                    for (i = 0; i < nmemb; ++i)
                         SWAP_2(ptr_2[i], aux->temp_2[i]);
                    ptr_temp = aux->temp_2;
                    break;
               case 4:
                    ptr_4 = (uint *) ptr;
                    for (i = 0; i < nmemb; ++i)
                         SWAP_4(ptr_4[i], aux->temp_4[i]);
                    ptr_temp = aux->temp_4;
                    break;
               case 8:
                    ptr_8 = (ulong *) ptr;
                    for (i = 0; i < nmemb; ++i)
                         SWAP_8(ptr_8[i], aux->temp_8[i]);
                    ptr_temp = aux->temp_8;
                    break;
               default:
                    fprintf(stderr, "ERROR: Invalid element size: %ld\n", size);
                    return 0;
          }
     }

     n = fwrite(ptr_temp, size, nmemb, stream);
     if (n < nmemb) {
          if (feof(stream))
               fprintf(stderr, "ERROR: End of file reached\n");
          else
               fprintf(stderr, "ERROR: Error writing file: %s\n", strerror(errno));

          return -1;
     }

     return n;
}



/*******************************************************************************
 * High level function to handle the choice of operation.
 ******************************************************************************/
int fxxxx_swap(void *ptr, size_t size, size_t nmemb, FILE *stream, struct
                      seviri_auxillary_io_data *aux) {

     if (aux->operation == 0)
          return fread_swap(ptr, size, nmemb, stream, aux);
     else
          return fwrite_swap(ptr, size, nmemb, stream, aux);
}



/*******************************************************************************
 *
 ******************************************************************************/
int seviri_l15_ph_data_read(FILE *fp,
                                   struct seviri_marf_l15_ph_data_data *d,
                                   struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(d->Name,  1, 30, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(d->Value, 1, 50, fp, aux) < 0) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
int seviri_l15_ph_data_id_read(FILE *fp,
                                      struct seviri_marf_l15_ph_data_id_data *d,
                                      struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(d->Name,    1, 30, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(d->Size,    1, 16, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(d->Address, 1, 16, fp, aux) < 0) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_marf_l15_main_product_header_read(
          FILE *fp,
          struct seviri_marf_l15_main_product_header_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     if (seviri_l15_ph_data_read(fp, &d->FormatName,           aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->FormatDocumentName,   aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->DocumentMajorVersion, aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->DocumentMinorVersion, aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->CreationDateTime,     aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->CreatingCenter,       aux)) E_L_R();
     for (i = 0; i < 27; ++i)
          if (seviri_l15_ph_data_id_read(fp, &d->DataSetIdentification[i], aux))
                                                                     E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->TotalFileSize,        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->GORT,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->ASTI,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->LLOS,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->SNIT,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->AIID,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->SSBT,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->SSST,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->RRCC,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->RRBT,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->RRST,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->PPRC,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->PPDT,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->GPLV,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->APNM,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->AARF,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->UUDT,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->QQOV,                 aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->UDSP,                 aux)) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_marf_l15_secondary_product_header_read(
          FILE *fp,
          struct seviri_marf_l15_secondary_product_header_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (seviri_l15_ph_data_read(fp, &d->ABID,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->SMOD,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->APXS,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->AVPA,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->LSCD,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->LMPA,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->QDLC,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->QDLP,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->QQAI,                        aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->SelectedBandIDs,             aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->SouthLineSelectedRectangle,  aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->NorthLineSelectedRectangle,  aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->EastColumnSelectedRectangle, aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->WestColumnSelectedRectangle, aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->NumberLinesVISIR,            aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->NumberColumnsVISIR,          aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->NumberLinesHRV,              aux)) E_L_R();
     if (seviri_l15_ph_data_read(fp, &d->NumberColumnsHRV,            aux)) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_marf_header_read(FILE *fp, struct seviri_marf_header_data *d,
                                   struct seviri_auxillary_io_data *aux) {

     if (seviri_marf_l15_main_product_header_read(fp, &d->main, aux)) {
          fprintf(stderr, "ERROR: seviri_marf_l15_main_product_header_read()\n");
          return -1;
     }

     if (seviri_marf_l15_secondary_product_header_read(fp, &d->secondary, aux)) {
          fprintf(stderr, "ERROR: seviri_marf_l15_secondary_product_header_read()\n");
          return -1;
     }

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
int seviri_TIME_CDS_read(FILE *fp,
                                struct seviri_TIME_CDS_data *d,
                                struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->day,  sizeof(short), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->msec, sizeof(int),   1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->usec, sizeof(short), 1, fp, aux) < 0) E_L_R();

     return 0;
}



int seviri_TIME_CDS_SHORT_read(FILE *fp,
                                      struct seviri_TIME_CDS_SHORT_data *d,
                                      struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->day,  sizeof(short), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->msec, sizeof(int),   1, fp, aux) < 0) E_L_R();

     return 0;
}



int seviri_TIME_CDS_EXPANDED_read(FILE *fp,
                                         struct seviri_TIME_CDS_EXPANDED_data *d,
                                         struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->day,  sizeof(short), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->msec, sizeof(int),   1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->usec, sizeof(short), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->nsec, sizeof(short), 1, fp, aux) < 0) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_15HEADER_SatelliteStatus_ORBITCOEF_read(
          FILE *fp,
          struct seviri_15HEADER_SatelliteStatus_ORBITCOEF_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (seviri_TIME_CDS_SHORT_read(fp, &d->StartTime, aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->EndTime,   aux))     E_L_R();
     if (fxxxx_swap(d->X,  sizeof(double), 8, fp,      aux) < 0) E_L_R();
     if (fxxxx_swap(d->Y,  sizeof(double), 8, fp,      aux) < 0) E_L_R();
     if (fxxxx_swap(d->Z,  sizeof(double), 8, fp,      aux) < 0) E_L_R();
     if (fxxxx_swap(d->VX, sizeof(double), 8, fp,      aux) < 0) E_L_R();
     if (fxxxx_swap(d->VY, sizeof(double), 8, fp,      aux) < 0) E_L_R();
     if (fxxxx_swap(d->VZ, sizeof(double), 8, fp,      aux) < 0) E_L_R();

     return 0;
}



int seviri_15HEADER_SatelliteStatus_read(
          FILE *fp,
          struct seviri_15HEADER_SatelliteStatus_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     if (fxxxx_swap(&d->SatelliteId,       sizeof(short),  1,     fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NominalLongitude,  sizeof(float),  1,     fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SatelliteStatus,   sizeof(uchar),  1,     fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->LastManoeuvreFlag, sizeof(uchar),  1,     fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->LastManoeuvreStartTime,   aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->LastManoeuvreEndTime,     aux))     E_L_R();
     if (fxxxx_swap(&d->LastManoeuvreType, sizeof(uchar),  1,     fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NextManoeuvreFlag, sizeof(uchar),  1,     fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->NextManoeuvreStartTime,   aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->NextManoeuvreEndTime,     aux))     E_L_R();
     if (fxxxx_swap(&d->NextManoeuvreType, sizeof(uchar),  1,     fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->PeriodStartTime,          aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->PeriodEndTime,            aux))     E_L_R();
     for (i = 0; i < 100; ++i)
          if (seviri_15HEADER_SatelliteStatus_ORBITCOEF_read
                   (fp, &d->OrbitPolynomial[i],                       aux))     E_L_R();
     if (fxxxx_swap(&d->Attitude,          sizeof(uchar),  20420, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SpinRateatRCStart, sizeof(double), 1,     fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UTCCorrelation,    sizeof(uchar),  59,    fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_ImageAcquisition_read(
          FILE *fp,
          struct seviri_15HEADER_ImageAcquisition_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (seviri_TIME_CDS_EXPANDED_read(fp, &d->TrueRepeatCycleStart,   aux))     E_L_R();
     if (seviri_TIME_CDS_EXPANDED_read(fp, &d->PlannedForwardScanEnd,  aux))     E_L_R();
     if (seviri_TIME_CDS_EXPANDED_read(fp, &d->PlannedRepeatCycleEnd,  aux))     E_L_R();

     if (fxxxx_swap(&d->ChannelStatus,        sizeof(uchar),  12,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->DetectorStatus,       sizeof(uchar),  42,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->MDUSamplingDelays,    sizeof(ushort), 42,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->MDUNomHRVDelay1,      sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MDUNomHRVDelay2,      sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Spare,                sizeof(uchar),  2,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MDUNomHRVBreakline,   sizeof(ushort), 1,   fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->DHSSSynchSelection,   sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MDUOutGain,           sizeof(ushort), 42,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MDUCourseGain,        sizeof(uchar),  42,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MDUFineGain,          sizeof(ushort), 42,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MDUNumericalOffset,   sizeof(ushort), 42,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PUGain,               sizeof(ushort), 42,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PUOffset,             sizeof(ushort), 27,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PUBias,               sizeof(ushort), 15,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->L0_LineCounter,       sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->K1_RetraceLines,      sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->K2_PauseDeciseconds,  sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->K3_RetraceLines,      sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->K4_PauseDeciseconds,  sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->K5_RetraceLines,      sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->X_DeepSpaceWindowPos, sizeof(uchar),  1,   fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->RefocusingLines,      sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RefocusingDirection,  sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RefocusingPosition,   sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ScanRefPosFlag,       sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ScanRefPosNumber,     sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ScanRefPotVal,        sizeof(float),  1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ScanFirstLine,        sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ScanLastLine,         sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RetraceStartLine,     sizeof(ushort), 1,   fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->LastGainChangeFlag,   sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->LastGainChangeTime,        aux))     E_L_R();

     if (fxxxx_swap(&d->DecontaminationNow,   sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->DecontaminationStart,      aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->DecontaminationEnd,        aux))     E_L_R();

     if (fxxxx_swap(&d->BBCalScheduled,       sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BBCalibrationType,    sizeof(uchar),  1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BBFirstLine,          sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BBLastLine,           sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ColdFocalPlaneOpTemp, sizeof(ushort), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->WarmFocalPlaneOpTemp, sizeof(ushort), 1,   fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_read(
          FILE *fp,
          struct seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->dummy, sizeof(uchar), 134915, fp, aux) < 134915) E_L_R();

     return 0;
}



static int seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_read(
          FILE *fp,
          struct seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (seviri_TIME_CDS_SHORT_read(fp, &d->StartTime,    aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->EndTime,      aux))     E_L_R();
     if (fxxxx_swap(&d->AlphaCoef, sizeof(double), 8, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaCoef,  sizeof(double), 8, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15HEADER_CelestialEvents_STARCOEF_read(
          FILE *fp,
          struct seviri_15HEADER_CelestialEvents_STARCOEF_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->StarId,    sizeof(ushort), 1, fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->StartTime,    aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->EndTime,      aux))     E_L_R();
     if (fxxxx_swap(&d->AlphaCoef, sizeof(double), 8, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaCoef,  sizeof(double), 8, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15HEADER_CelestialEvents_read(
          FILE *fp,
          struct seviri_15HEADER_CelestialEvents_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     if (seviri_TIME_CDS_SHORT_read(fp, &d->PeriodStartTime,        aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->PeriodEndTime,          aux))     E_L_R();
     if (seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_read
              (fp, &d->RelatedOrbitFileTime,    aux))                         E_L_R();
     if (seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_read
              (fp, &d->RelatedAttitudeFileTime, aux))                         E_L_R();

     for (i = 0; i < 100; ++i)
          if (seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_read
              (fp, &d->EarthEphemeris[i],  aux))                              E_L_R();
     for (i = 0; i < 100; ++i)
          if (seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_read
              (fp, &d->MoonEphemeris [i],  aux))                              E_L_R();
     for (i = 0; i < 100; ++i)
          if (seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_read
              (fp, &d->SunEphemeris  [i],  aux))                              E_L_R();
     for (i = 0; i < 100; ++i)
          if (seviri_15HEADER_CelestialEvents_STARCOEF_read
              (fp, &d->StarEphemeris [i],  aux))                              E_L_R();

     if (fxxxx_swap(&d->TypeofEclipse,        sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->EclipseStartTime,       aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->EclipseEndTime,         aux))     E_L_R();
     if (fxxxx_swap(&d->VisibleBodiesInImage, sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BodiesClosetoFOV,     sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImpactOnImageQuality, sizeof(uchar), 1, fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_ImageDescription_ReferenceGridVIS_IR_read(
          FILE *fp,
          struct seviri_15HEADER_ImageDescription_ReferenceGridVIS_IR_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->NumberOfLines,     sizeof(int),   1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NumberOfColumns,   sizeof(int),   1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LineDirGridStep,   sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ColumnDirGridStep, sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->GridOrigin,        sizeof(uchar), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15HEADER_ImageDescription_ReferenceGridHRV_read(
          FILE *fp,
          struct seviri_15HEADER_ImageDescription_ReferenceGridHRV_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->NumberOfLines,     sizeof(int),   1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NumberOfColumns,   sizeof(int),   1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LineDirGridStep,   sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ColumnDirGridStep, sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->GridOrigin,        sizeof(uchar), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15HEADER_ImageDescription_PlannedCoverageVIS_IR_read(
          FILE *fp,
          struct seviri_15HEADER_ImageDescription_PlannedCoverageVIS_IR_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->SouthernLinePlanned,  sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthernLinePlanned,  sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EasternColumnPlanned, sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->WesternColumnPlanned, sizeof(int), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15HEADER_ImageDescription_PlannedCoverageHRV_read(
          FILE *fp,
          struct seviri_15HEADER_ImageDescription_PlannedCoverageHRV_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->LowerSouthLinePlanned,  sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LowerNorthLinePlanned,  sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LowerEastColumnPlanned, sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LowerWestColumnPlanned, sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperSouthLinePlanned,  sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperNorthLinePlanned,  sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperEastColumnPlanned, sizeof(int), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperWestColumnPlanned, sizeof(int), 1, fp, aux) < 0) E_L_R();

     return 0;
}



int seviri_15HEADER_ImageDescription_read(
          FILE *fp,
          struct seviri_15HEADER_ImageDescription_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->TypeOfProjection,      sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LongitudeOfSSP,        sizeof(float), 1,  fp, aux) < 0) E_L_R();

     if (seviri_15HEADER_ImageDescription_ReferenceGridVIS_IR_read
              (fp, &d->ReferenceGridVIS_IR,   aux))                             E_L_R();
     if (seviri_15HEADER_ImageDescription_ReferenceGridHRV_read
              (fp, &d->ReferenceGridHRV,      aux))                             E_L_R();
     if (seviri_15HEADER_ImageDescription_PlannedCoverageVIS_IR_read
              (fp, &d->PlannedCoverageVIS_IR, aux))                             E_L_R();
     if (seviri_15HEADER_ImageDescription_PlannedCoverageHRV_read
              (fp, &d->PlannedCoverageHRV,    aux))                             E_L_R();

     if (fxxxx_swap(&d->ImageProcDirection,    sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PixelGenDirection,     sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PlannedChanProcessing, sizeof(uchar), 12, fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_RadiometricProcessing_Level1_5ImageCalibration_read(
          FILE *fp,
          struct seviri_15HEADER_RadiometricProcessing_Level1_5ImageCalibration_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->Cal_Slope,  sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Cal_Offset, sizeof(double), 1, fp, aux) < 0) E_L_R();

     return 0;
}



int seviri_15HEADER_RadiometricProcessing_read(
          FILE *fp,
          struct seviri_15HEADER_RadiometricProcessing_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     if (fxxxx_swap(&d->RadianceLinearization,    sizeof(uchar), 12,    fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->DetectorEqualization,     sizeof(uchar), 12,    fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->OnBoardCalibrationResult, sizeof(uchar), 12,    fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MPEFCalFeedback,          sizeof(uchar), 12,    fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MTFAdaptation,            sizeof(uchar), 12,    fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->StraylightCorrectionFlag, sizeof(uchar), 12,    fp, aux) < 0) E_L_R();

     for (i = 0; i < 12; ++i)
          if (seviri_15HEADER_RadiometricProcessing_Level1_5ImageCalibration_read
                   (fp, &d->Level1_5ImageCalibration[i], aux)) E_L_R();

     if (fxxxx_swap(d->dummy,                     sizeof(uchar), 20551, fp, aux) < 20551) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_GeometricProcessing_read(
          FILE *fp,
          struct seviri_15HEADER_GeometricProcessing_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(d->E_WFocalPlane,        sizeof(float),  42,       fp, aux) < 0) E_L_R();
     if (fxxxx_swap(d->N_SFocalPlane,        sizeof(float),  42,       fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->TypeOfEarthModel,    sizeof(uchar),  1,        fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EquatorialRadius,    sizeof(double), 1,        fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthPolarRadius,    sizeof(double), 1,        fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SouthPolarRadius,    sizeof(double), 1,        fp, aux) < 0) E_L_R();

     if (fxxxx_swap(d->AtmosphericModel,     sizeof(float),  12 * 360, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ResamplingFunctions, sizeof(uchar),  12,       fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_IMPFConfiguration_read(
          FILE *fp,
          struct seviri_15HEADER_IMPFConfiguration_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(d->dummy, sizeof(uchar), 19786, fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15HEADER_read(
          FILE *fp,
          struct seviri_15HEADER_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->_15HeaderVersion, sizeof(uchar), 1, fp, aux) < 0)                E_L_R();

     if (seviri_15HEADER_SatelliteStatus_read      (fp, &d->SatelliteStatus,       aux)) E_L_R();
     if (seviri_15HEADER_ImageAcquisition_read     (fp, &d->ImageAcquisition,      aux)) E_L_R();
     if (seviri_15HEADER_CelestialEvents_read      (fp, &d->CelestialEvents,       aux)) E_L_R();
     if (seviri_15HEADER_ImageDescription_read     (fp, &d->ImageDescription,      aux)) E_L_R();
     if (seviri_15HEADER_RadiometricProcessing_read(fp, &d->RadiometricProcessing, aux)) E_L_R();
     if (seviri_15HEADER_GeometricProcessing_read  (fp, &d->GeometricProcessing,   aux)) E_L_R();
     if (seviri_15HEADER_IMPFConfiguration_read    (fp, &d->IMPFConfiguration,     aux)) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_15TRAILER_ImageProductionStats_L15ImageValidity_read(
          FILE *fp,
          struct seviri_15TRAILER_ImageProductionStats_L15ImageValidity_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->NominalImage,                 sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NonNominalBecauseIncomplete,  sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NonNominalRadiometricQuality, sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NonNominalGeometricQuality,   sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NonNominalTimeliness,         sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->IncompleteL15,                sizeof(uchar), 1,  fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_ImageProductionStats_read(
          FILE *fp,
          struct seviri_15TRAILER_ImageProductionStats_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     if (fxxxx_swap(&d->SatelliteID,                  sizeof(short), 1,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->NominalImageScanning,         sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ReducedScan,                  sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->ActScanForwardStart,             aux))     E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->ActScanForwardEnd,               aux))     E_L_R();

     if (fxxxx_swap(&d->NominalBehaviour,             sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RadScanIrregularity,          sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RadStoppage,                  sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RepeatCycleNotCompleted,      sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->GainChangeTookPlace,          sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->DecontaminationTookPlace,     sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NoBBCalibrationAchieved,      sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->IncorrectTemperature,         sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->InvalidBBData,                sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->InvalidAuxOrHKTMData,         sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->RefocusingMechanismActuated,  sizeof(uchar), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MirrorBackToReferencePos,     sizeof(uchar), 1,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->PlannedNumberOfL10Lines,      sizeof(uchar), 12, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NumberOfMissingL10Lines,      sizeof(uchar), 12, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NumberOfCorruptedL10Lines,    sizeof(uchar), 12, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NumberOfReplacedL10Lines,     sizeof(uchar), 12, fp, aux) < 0) E_L_R();

     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_ImageProductionStats_L15ImageValidity_read
              (fp, &d->L15ImageValidity[i], aux))                                      E_L_R();

     if (fxxxx_swap(&d->SouthernLineActual,           sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthernLineActual,           sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EasternColumnActual,          sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->WesternColumnActual,          sizeof(int),   1,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->LowerSouthLineActual,         sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LowerNorthLineActual,         sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LowerEastColumnActual,        sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LowerWestColumnActual,        sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperSouthLineActual,         sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperNorthLineActual,         sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperEastColumnActual,        sizeof(int),   1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->UpperWestColumnActual,        sizeof(int),   1,  fp, aux) < 0) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15TRAILER_NavigationExtractionResults_HORIZONOBSERVATION_read(
          FILE *fp,
          struct seviri_15TRAILER_NavigationExtractionResults_HORIZONOBSERVATION_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->HorizonId,                sizeof(uchar),  1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Alpha,                    sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->AlphaConfidence,          sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Beta,                     sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaConfidence,           sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_read(fp, &d->ObservationTime,                   aux))     E_L_R();
     if (fxxxx_swap(&d->SpinRate,                 sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->AlphaDeviation,           sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaDeviation,            sizeof(double), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_NavigationExtractionResults_STAROBSERVATION_read(
          FILE *fp,
          struct seviri_15TRAILER_NavigationExtractionResults_STAROBSERVATION_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->StarId,                   sizeof(uchar),  1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Alpha,                    sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->AlphaConfidence,          sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Beta,                     sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaConfidence,           sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_read(fp, &d->ObservationTime,                   aux))     E_L_R();
     if (fxxxx_swap(&d->SpinRate,                 sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->AlphaDeviation,           sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaDeviation,            sizeof(double), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_NavigationExtractionResults_LANDMARKOBSERVATION_read(
          FILE *fp,
          struct seviri_15TRAILER_NavigationExtractionResults_LANDMARKOBSERVATION_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->LandmarkId,               sizeof(uchar),  1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LandmarkLongitude,        sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LandmarkLatitude,         sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Alpha,                    sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->AlphaConfidence,          sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Beta,                     sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaConfidence,           sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (seviri_TIME_CDS_read(fp, &d->ObservationTime,                   aux))     E_L_R();
     if (fxxxx_swap(&d->SpinRate,                 sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->AlphaDeviation,           sizeof(double), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->BetaDeviation,            sizeof(double), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_NavigationExtractionResults_read(
          FILE *fp,
          struct seviri_15TRAILER_NavigationExtractionResults_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     for (i = 0; i < 4; ++i)
          if (seviri_15TRAILER_NavigationExtractionResults_HORIZONOBSERVATION_read
                   (fp, &d->ExtractedHorizons [i], aux)) E_L_R();
     for (i = 0; i < 20; ++i)
          if (seviri_15TRAILER_NavigationExtractionResults_STAROBSERVATION_read
                   (fp, &d->ExtractedStars    [i], aux)) E_L_R();
     for (i = 0; i < 50; ++i)
          if (seviri_15TRAILER_NavigationExtractionResults_LANDMARKOBSERVATION_read
                   (fp, &d->ExtractedLandmarks[i], aux)) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15TRAILER_RadiometricQuality_L10RadQuality_read(
          FILE *fp,
          struct seviri_15TRAILER_RadiometricQuality_L10RadQuality_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->FullImageMinimumCount,          sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageMaximumCount,          sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskMinimumCount,          sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskMaximumCount,          sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MoonMinimumCount,               sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MoonMaximumCount,               sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageMeanCount,             sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageStandardDeviation,     sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskMeanCount,             sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskStandardDeviation,     sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MoonMeanCount,                  sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MoonStandardDeviation,          sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SpaceMeanCount,                 sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SpaceStandardDeviation,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SESpaceCornerMeanCount,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SESpaceCornerStandardDeviation, sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SWSpaceCornerMeanCount,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SWSpaceCornerStandardDeviation, sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NESpaceCornerMeanCount,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NESpaceCornerStandardDeviation, sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NWSpaceCornerMeanCount,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NWSpaceCornerStandardDeviation, sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FourSpaceCornersMeanCount,      sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FourSpaceCornersStandardDev,    sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageHistogram,             sizeof(uint),  256, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskHistogram,             sizeof(uint),  256, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquareHistogram,     sizeof(uint),  256, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SESpaceCornerHistogram,         sizeof(uint),  128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SWSpaceCornerHistogram,         sizeof(uint),  128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NESpaceCornerHistogram,         sizeof(uint),  128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NWSpaceCornerHistogram,         sizeof(uint),  128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageEntropy,               sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskEntropy,               sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquareEntropy,       sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SESpaceCornerEntropy,           sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SWSpaceCornerEntropy,           sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NESpaceCornerEntropy,           sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NWSpaceCornerEntropy,           sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FourSpaceCornersEntropy,        sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquarePSD_EW,        sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImagePSD_EW,                sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquarePSD_NS,        sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImagePSD_NS,                sizeof(float), 128, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_RadiometricQuality_L15RadQuality_read(
          FILE *fp,
          struct seviri_15TRAILER_RadiometricQuality_L15RadQuality_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->FullImageMinimumCount,      sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageMaximumCount,      sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskMinimumCount,      sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskMaximumCount,      sizeof(short), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageMeanCount,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageStandardDeviation, sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskMeanCount,         sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskStandardDeviation, sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SpaceMeanCount,             sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SpaceStandardDeviation,     sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageHistogram,         sizeof(uint),  256, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskHistogram,         sizeof(uint),  256, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquareHistogram, sizeof(uint),  256, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImageEntropy,           sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EarthDiskEntropy,           sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquareEntropy,   sizeof(float), 3,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquarePSD_EW,    sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImagePSD_EW,            sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ImageCenterSquarePSD_NS,    sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->FullImagePSD_NS,            sizeof(float), 128, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SESpaceCornerL15_RMS,       sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SESpaceCornerL15_Mean,      sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SWSpaceCornerL15_RMS,       sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SWSpaceCornerL15_Mean,      sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NESpaceCornerL15_RMS,       sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NESpaceCornerL15_Mean,      sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NWSpaceCornerL15_RMS,       sizeof(float), 1,   fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NWSpaceCornerL15_Mean,      sizeof(float), 1,   fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_RadiometricQuality_read(
          FILE *fp,
          struct seviri_15TRAILER_RadiometricQuality_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     for (i = 0; i < 42; ++i)
          if (seviri_15TRAILER_RadiometricQuality_L10RadQuality_read
                   (fp, &d->L10RadQuality[i], aux)) E_L_R();
     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_RadiometricQuality_L15RadQuality_read
                   (fp, &d->L15RadQuality[i], aux)) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15TRAILER_GeometricQuality_Accuracy_read(
          FILE *fp,
          struct seviri_15TRAILER_GeometricQuality_Accuracy_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->QualityInfoValidity,      sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestAccuracyRMS,      sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthAccuracyRMS,    sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MagnitudeRMS,             sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestUncertaintyRMS,   sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthUncertaintyRMS, sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MagnitudeUncertaintyRMS,  sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestMaxDeviation,     sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthMaxDeviation,   sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MagnitudeMaxDeviation,    sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestUncertaintyMax,   sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthUncertaintyMax, sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MagnitudeUncertaintyMax,  sizeof(float), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_GeometricQuality_MisregistrationResiduals_read(
          FILE *fp,
          struct seviri_15TRAILER_GeometricQuality_MisregistrationResiduals_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->QualityInfoValidity,            sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestResidual,               sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthResidual,             sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestUncertainty,            sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthUncertainty,          sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestRMS,                    sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthRMS,                  sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestMagnitude,              sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthMagnitude,            sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->EastWestMagnitudeUncertainty,   sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NorthSouthMagnitudeUncertainty, sizeof(float), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_GeometricQuality_GeometricQualityStatus_read(
          FILE *fp,
          struct seviri_15TRAILER_GeometricQuality_GeometricQualityStatus_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->QualityNominal,                 sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NominalAbsolute,                sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NominalRelativeToPreviousImage, sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NominalForREL500,               sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NominalForREL16,                sizeof(uchar), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->NominalForResMisreg,            sizeof(uchar), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_GeometricQuality_read(
          FILE *fp,
          struct seviri_15TRAILER_GeometricQuality_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_GeometricQuality_Accuracy_read
                   (fp, &d->AbsoluteAccuracy[i],           aux)) E_L_R();
     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_GeometricQuality_Accuracy_read
                   (fp, &d->RelativeAccuracy[i],           aux)) E_L_R();
     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_GeometricQuality_Accuracy_read
                   (fp, &d->_500PixelsRelativeAccuracy[i], aux)) E_L_R();
     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_GeometricQuality_Accuracy_read
                   (fp, &d->_16PixelsRelativeAccuracy[i],  aux)) E_L_R();
     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_GeometricQuality_MisregistrationResiduals_read
                   (fp, &d->MisregistrationResiduals[i],   aux)) E_L_R();
     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_GeometricQuality_GeometricQualityStatus_read
                   (fp, &d->GeometricQualityStatus  [i],   aux)) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15TRAILER_TimelinessAndCompleteness_Completeness_read(
          FILE *fp,
          struct seviri_15TRAILER_TimelinessAndCompleteness_Completeness_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->PlannedL15ImageLines,    sizeof(ushort), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->GeneratedL15ImageLines,  sizeof(ushort), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ValidL15ImageLines,      sizeof(ushort), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->DummyL15ImageLines,      sizeof(ushort), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->CorruptedL15ImageLines,  sizeof(ushort), 1, fp, aux) < 0) E_L_R();

     return 0;
}



static int seviri_15TRAILER_TimelinessAndCompleteness_read(
          FILE *fp,
          struct seviri_15TRAILER_TimelinessAndCompleteness_data *d,
          struct seviri_auxillary_io_data *aux) {

     uint i;

     if (fxxxx_swap(&d->MaxDelay,  sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MinDelay,  sizeof(float), 1, fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->MeanDelay, sizeof(float), 1, fp, aux) < 0) E_L_R();

     for (i = 0; i < 12; ++i)
          if (seviri_15TRAILER_TimelinessAndCompleteness_Completeness_read
                   (fp, &d->Completeness[i], aux)) E_L_R();

     return 0;
}



/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
static int seviri_15TRAILER_read(
          FILE *fp,
          struct seviri_15TRAILER_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->L15TrailerVersion, sizeof(uchar), 1, fp, aux) < 0) E_L_R();

     if (seviri_15TRAILER_ImageProductionStats_read
              (fp, &d->ImageProductionStats,                     aux))     E_L_R();
     if (seviri_15TRAILER_NavigationExtractionResults_read
             (fp, &d->NavigationExtractionResults,               aux))     E_L_R();
     if (seviri_15TRAILER_RadiometricQuality_read
             (fp, &d->RadiometricQuality,                        aux))     E_L_R();
     if (seviri_15TRAILER_GeometricQuality_read
             (fp, &d->GeometricQuality,                          aux))     E_L_R();
     if (seviri_15TRAILER_TimelinessAndCompleteness_read
             (fp, &d->TimelinessAndCompleteness,                 aux))     E_L_R();

     if (fxxxx_swap(&d->dummy, sizeof(uchar), 214, fp,           aux) < 0) E_L_R();

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_packet_header_read(
          FILE *fp,
          struct seviri_packet_header_data *d,
          struct seviri_auxillary_io_data *aux) {

     aux->swap_bytes = 0;

     if (fxxxx_swap(&d->HeaderVersionNo,    sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PacketType,         sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SubHeaderType,      sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SourceFacilityId,   sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SourceEnvId,        sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SourceInstanceId,   sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SourceSUId,         sizeof(int),    1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SourceCPUId,        sizeof(uchar),  4,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->DestFacilityId,     sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->DestEnvId,          sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SequenceCount,      sizeof(ushort), 1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PacketLength,       sizeof(int),    1,  fp, aux) < 0) E_L_R();

     if (fxxxx_swap(&d->SubHeaderVersionNo, sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ChecksumFlag,       sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->Acknowledgement,    sizeof(uchar),  4,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ServiceType,        sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ServiceSubtype,     sizeof(uchar),  1,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->PacketTime,         sizeof(uchar),  6,  fp, aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SpacecraftId,       sizeof(short),  1,  fp, aux) < 0) E_L_R();

     aux->swap_bytes = 1;

     return 0;
}



/*******************************************************************************
 *
 ******************************************************************************/
static int seviri_LineSideInfo_read(
          FILE *fp,
          struct seviri_LineSideInfo_data *d,
          struct seviri_auxillary_io_data *aux) {

     if (fxxxx_swap(&d->_15LINEVersion,         sizeof(char),  1,  fp,  aux) < 0) E_L_R();
     if (fxxxx_swap(&d->SatelliteId,            sizeof(short), 1,  fp,  aux) < 0) E_L_R();
     if (seviri_TIME_CDS_EXPANDED_read(fp, &d->TrueRepeatCycleStart,    aux))     E_L_R();
     if (fxxxx_swap(&d->LineNumberInGrid,       sizeof(int),   1,  fp,  aux) < 0) E_L_R();
     if (fxxxx_swap(&d->ChannelId,              sizeof(char),  1,  fp,  aux) < 0) E_L_R();
     if (seviri_TIME_CDS_SHORT_read(fp, &d->L10LineMeanAcquisitionTime, aux))     E_L_R();
     if (fxxxx_swap(&d->LineValidity,           sizeof(char),  1,  fp,  aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LineRadiometricQuality, sizeof(char),  1,  fp,  aux) < 0) E_L_R();
     if (fxxxx_swap(&d->LineGeometricQuality,   sizeof(char),  1,  fp,  aux) < 0) E_L_R();

     return 0;
}



/*******************************************************************************
 * Fills a seviri_dimension_data struct given a choice of offset and dimension
 * parameters.
 *
 * d		: The output seviri_dimension_data struct
 * marf_header	: The seviri_marf_header_data struct for the current image data
 *                file.
 * bounds	: Described in the seviri_native_read() header.
 * line0	: 	''
 * line1	: 	''
 * column0	: 	''
 * column1	: 	''
 * lat0		: 	''
 * lat1		: 	''
 * lon0		: 	''
 * lon1		: 	''
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_get_dimension_data(
          struct seviri_dimension_data *d,
          const struct seviri_marf_header_data *marf_header,
          enum seviri_bounds bounds,
          uint line0, uint line1, uint column0, uint column1,
          double lat0, double lat1, double lon0, double lon1) {

     /*-------------------------------------------------------------------------
      * Get some fields from the U-MARF header file.
      *-----------------------------------------------------------------------*/
     sscanf(marf_header->secondary.NumberLinesVISIR.Value,   "%u",
            &d->n_lines_selected_VIR);
     sscanf(marf_header->secondary.NumberColumnsVISIR.Value, "%u",
            &d->n_columns_selected_VIR);

     if (VERBOSE) {
          printf("n_lines_selected_VIR     = %u\n", d->n_lines_selected_VIR);
          printf("n_columns_selected_VIR   = %u\n", d->n_columns_selected_VIR);
          printf("\n");
     }

     sscanf(marf_header->secondary.SouthLineSelectedRectangle.Value,  "%u",
            &d->i0_line_selected_VIR);
     sscanf(marf_header->secondary.NorthLineSelectedRectangle.Value,  "%u",
            &d->i1_line_selected_VIR);
     sscanf(marf_header->secondary.EastColumnSelectedRectangle.Value, "%u",
            &d->i0_column_selected_VIR);
     sscanf(marf_header->secondary.WestColumnSelectedRectangle.Value, "%u",
            &d->i1_column_selected_VIR);

     if (VERBOSE) {
          printf("i0_line_selected_VIR     = %u\n", d->i0_line_selected_VIR);
          printf("i1_line_selected_VIR     = %u\n", d->i1_line_selected_VIR);
          printf("i0_column_selected_VIR   = %u\n", d->i0_column_selected_VIR);
          printf("i1_column_selected_VIR   = %u\n", d->i1_column_selected_VIR);
          printf("\n");
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     d->i0_line_selected_VIR--;
     d->i0_column_selected_VIR--;
     d->i1_line_selected_VIR--;
     d->i1_column_selected_VIR--;


     /*-------------------------------------------------------------------------
      * Set various offsets and dimensions depending on the requested of
      * dimensioning.
      *-----------------------------------------------------------------------*/
     if (bounds == SEVIRI_BOUNDS_FULL_DISK) {
          d->i_line_requested_VIR    = 0;
          d->i_column_requested_VIR  = 0;

          d->n_lines_requested_VIR   = IMAGE_SIZE_VIR_LINES;
          d->n_columns_requested_VIR = IMAGE_SIZE_VIR_COLUMNS;

          d->i_line_to_read_VIR      = 0;
          d->i_column_to_read_VIR    = 0;

          d->n_lines_to_read_VIR     = d->n_lines_selected_VIR;
          d->n_columns_to_read_VIR   = d->n_columns_selected_VIR;

          d->i_line_in_output_VIR    = d->i0_line_selected_VIR;
          d->i_column_in_output_VIR  = d->i0_column_selected_VIR;
     }
     else
     if (bounds == SEVIRI_BOUNDS_ACTUAL_IMAGE) {
          d->i_line_requested_VIR    = d->i0_line_selected_VIR;
          d->i_column_requested_VIR  = d->i0_column_selected_VIR;

          d->n_lines_requested_VIR   = d->n_lines_selected_VIR;
          d->n_columns_requested_VIR = d->n_columns_selected_VIR;

          d->i_line_to_read_VIR      = 0;
          d->i_column_to_read_VIR    = 0;

          d->n_lines_to_read_VIR     = d->n_lines_selected_VIR;
          d->n_columns_to_read_VIR   = d->n_columns_selected_VIR;

          d->i_line_in_output_VIR    = 0;
          d->i_column_in_output_VIR  = 0;
     }
     else
     if (bounds == SEVIRI_BOUNDS_LINE_COLUMN || bounds == SEVIRI_BOUNDS_LAT_LON) {
          if (bounds == SEVIRI_BOUNDS_LINE_COLUMN) {
               if (column0 % 4 || (column1 + 1) % 4) {
                    fprintf(stderr, "ERROR: column bounds must be on boundaries "
                                    "of a multiple of 4\n");
                    return -1;
               }
          }
          else {
               if (snu_lat_lon_to_line_column(lat0, lon1, &line0, &column0, 0.,
                                              &nav_scaling_factors_vir)) {
                    fprintf(stderr, "ERROR: snu_lat_lon_to_line_column()\n");
                    return -1;
               }

               line0--;
               column0--;

               if ((column0 % 4))
                    column0 = column0 / 4 * 4;

               if (snu_lat_lon_to_line_column(lat1, lon0, &line1, &column1, 0.,
                                              &nav_scaling_factors_vir)) {
                    fprintf(stderr, "ERROR: snu_lat_lon_to_line_column()\n");
                    return -1;
               }

               line1--;
               column1--;

               if ((column1 + 1) % 4)
                    column1 = (column1 + 1) / 4 * 4 + 4 - 1;
          }

          if (VERBOSE) {
                    printf("line0                    = %u\n", line0);
                    printf("line1                    = %u\n", line1);
                    printf("column0                  = %u\n", column0);
                    printf("column1                  = %u\n", column1);
                    printf("\n");
          }


          /*--------------------------------------------------------------------
           * Check that the requested pixel coordinates are valid.
           *------------------------------------------------------------------*/
          if (line0 < d->i0_line_selected_VIR) {
               fprintf(stderr, "ERROR: requested start line (line0 = %u) is "
                               "less than that of the actual image: %u\n", line0,
                               d->i0_line_selected_VIR);
               return -1;
          }
/*
          if (line1 > d->i1_line_selected_VIR) {
*/
          if (line1 > d->i0_line_selected_VIR + d->n_lines_selected_VIR - 1) {
               fprintf(stderr, "ERROR: requested end   line (line1 = %u) is "
                               "greater than that of the actual image: %u\n", line1,
                               d->i1_line_selected_VIR);
               return -1;
          }

          if (column0 < d->i0_column_selected_VIR) {
               fprintf(stderr, "ERROR: requested start column (column0 = %u) is "
                               "less than that of the actual image: %u\n", column0,
                               d->i0_column_selected_VIR);
               return -1;
          }
/*
          if (column1 > d->i1_column_selected_VIR) {
*/
          if (column1 > d->i0_column_selected_VIR + d->n_columns_selected_VIR - 1) {
               fprintf(stderr, "ERROR: requested end   column (column1 = %u) is "
                               "greater than that of the actual image: %u\n", column1,
                               d->i1_column_selected_VIR);
               return -1;
          }


          /*--------------------------------------------------------------------
           *
           *------------------------------------------------------------------*/
          d->i_line_requested_VIR    = line0;
          d->i_column_requested_VIR  = column0;

          d->n_lines_requested_VIR   = line1   - line0   + 1;
          d->n_columns_requested_VIR = column1 - column0 + 1;


          d->n_lines_to_read_VIR     = d->n_lines_selected_VIR;
          d->n_columns_to_read_VIR   = d->n_columns_selected_VIR;

          d->i_line_in_output_VIR    = d->i0_line_selected_VIR;
          d->i_column_in_output_VIR  = d->i0_column_selected_VIR;


          d->i_line_to_read_VIR      = 0;
          d->i_column_to_read_VIR    = 0;

          if (d->i_line_requested_VIR   > d->i0_line_selected_VIR)
               d->i_line_to_read_VIR  = d->i_line_requested_VIR -
                                        d->i0_line_selected_VIR;
          if (d->i_column_requested_VIR > d->i0_column_selected_VIR)
               d->i_column_to_read_VIR = d->i_column_requested_VIR -
                                         d->i0_column_selected_VIR;

          d->n_lines_to_read_VIR     =
               MIN(d->n_lines_selected_VIR - d->i_line_to_read_VIR,
                   d->n_lines_requested_VIR);
          d->n_columns_to_read_VIR   =
               MIN(d->n_columns_selected_VIR - d->i_column_to_read_VIR,
                   d->n_columns_requested_VIR);

          d->i_line_in_output_VIR    = 0;
          d->i_column_in_output_VIR  = 0;
     }
     else {
          fprintf(stderr, "ERROR: invalid seviri_bounds type: %d\n", bounds);
          return -1;
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     if (VERBOSE) {
          printf("n_lines_selected_VIR     = %u\n", d->n_lines_selected_VIR);
          printf("n_columns_selected_VIR   = %u\n", d->n_columns_selected_VIR);
          printf("\n");

          printf("i_line_requested_VIR     = %u\n", d->i_line_requested_VIR);
          printf("i_column_requested_VIR   = %u\n", d->i_column_requested_VIR);
          printf("\n");

          printf("n_lines_requested_VIR    = %u\n", d->n_lines_requested_VIR);
          printf("n_columns_requested_VIR  = %u\n", d->n_columns_requested_VIR);
          printf("\n");

          printf("i_line_to_read_VIR       = %u\n", d->i_line_to_read_VIR);
          printf("i_column_to_read_VIR     = %u\n", d->i_column_to_read_VIR);
          printf("\n");

          printf("n_lines_to_read_VIR      = %u\n", d->n_lines_to_read_VIR);
          printf("n_columns_to_read_VIR    = %u\n", d->n_columns_to_read_VIR);
          printf("\n");

          printf("i_line_in_output_VIR     = %u\n", d->i_line_in_output_VIR);
          printf("i_column_in_output_VIR   = %u\n", d->i_column_in_output_VIR);
          printf("\n");
     }


     /*-------------------------------------------------------------------------
      *
      *-----------------------------------------------------------------------*/
     sscanf(marf_header->secondary.NumberLinesHRV.Value,     "%u",
            &d->n_lines_selected_HRV);
     sscanf(marf_header->secondary.NumberColumnsHRV.Value,   "%u",
            &d->n_columns_selected_HRV);


     return 0;
}



/*******************************************************************************
 * Read a VIS/IR line record structure - the actual image data.
 *
 * fp		: Pointer to the image data file set to the beginning of the
 *              : line record structure.
 * image	: The output seviri_image_data struct with the image data
 * marf_header	: The seviri_marf_header_data struct for the current image data
 *                file.
 * n_bands	: Described in the seviri_native_read() header
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
                             struct seviri_auxillary_io_data *aux) {

     uchar *data10;

     const uchar shifts[] = {6, 4, 2, 0};

     ushort temp;

     const ushort masks[] = {0xFFC0, 0x3FF0, 0x0FFC, 0x03FF};

     uint i;
     uint ii;
     uint j;
     uint jj;
     uint k;
     uint q;

     uint skipper;

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

     int bands_infile[12];
     for (i=0;i<12;i++)bands_infile[i]=0;

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
          if (marf_header->secondary.SelectedBandIDs.Value[i] == 'X'){
               n_bands_VIR++;
            bands_infile[i]=1;}
     }

     n_bands_HRV = 0;
     if (marf_header->secondary.SelectedBandIDs.Value[11] == 'X'){
          n_bands_HRV = 1;
          bands_infile[11]=1;}

     /*-------------------------------------------------------------------------
      * Allocate and fill in the seviri_dimension_data struct.
      *-----------------------------------------------------------------------*/
     image->dimens = malloc(sizeof(struct seviri_dimension_data));

     dimens = (struct seviri_dimension_data *) image->dimens;

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
            if (bands_infile[image->band_ids[i_band]-1]!=1) continue;
          skipper=0;
          for (q=0;q<image->band_ids[i_band]-1;q++) if (bands_infile[q]==1) skipper++;
            file_offset2 = file_offset + (skipper) * n_bytes_VIR_line + dimens->i_column_to_read_VIR / 4 * 5;

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
                              struct seviri_auxillary_io_data *aux) {

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
     dimens = (struct seviri_dimension_data *) image->dimens;


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
static int seviri_image_free(struct seviri_image_data *d) {

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
     free(d->dimens);

     return 0;
}



/*******************************************************************************
 * Allocate and free data required by the low level read and write functions.
 ******************************************************************************/
int seviri_auxillary_alloc(struct seviri_auxillary_io_data *d) {

     uint n = 134915;

     d->temp_2 = malloc(n * sizeof(ushort));
     d->temp_4 = malloc(n * sizeof(uint));
     d->temp_8 = malloc(n * sizeof(ulong));

     return 0;
}



int seviri_auxillary_free(struct seviri_auxillary_io_data *d) {

     free(d->temp_2);
     free(d->temp_4);
     free(d->temp_8);

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
 * bounds	: Described in the seviri_native_read() header.
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
int seviri_native_get_dimens(const char *filename, uint *i_line, uint *i_column,
                             uint *n_lines, uint *n_columns, enum seviri_bounds bounds,
                             uint line0, uint line1, uint column0, uint column1,
                             double lat0, double lat1, double lon0, double lon1) {

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
 * d		: The output seviri_native_data struct with the U-MARF header,
 *                level 1.5 header and trailer, and the image data.
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
int seviri_native_read(const char *filename, struct seviri_native_data *d,
                       uint n_bands, const uint *band_ids,
                       enum seviri_bounds bounds,
                       uint line0, uint line1, uint column0, uint column1,
                       double lat0, double lat1, double lon0, double lon1) {

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
 * d		: The input seviri_native_data struct with the U-MARF header,
 *                level 1.5 header and trailer, and the image data.
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_native_write(const char *filename, const struct seviri_native_data *d) {

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



/*******************************************************************************
 * Free memory allocated by seviri_native_read() to hold seviri_native_data
 * struct fields.
 *
 * image	: The input seviri_native_data struct
 *
 * returns	: Non-zero on error
 ******************************************************************************/
int seviri_native_free(struct seviri_native_data *d) {

     if (seviri_image_free(&d->image)) {
          fprintf(stderr, "ERROR: seviri_image_free()\n");
          return -1;
     }

     return 0;
}
