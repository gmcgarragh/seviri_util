/******************************************************************************%
**
**    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
**
**    This source code is licensed under the GNU General Public License (GPL),
**    Version 3.  See the file COPYING for more details.
**
*******************************************************************************/

#ifndef READ_WRITE_H
#define READ_WRITE_H

#include "external.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


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
 * The U-MARF header
 ******************************************************************************/
struct seviri_marf_l15_ph_data_data {
     char Name[30];
     char Value[50];
};


struct seviri_marf_l15_ph_data_id_data {
     char Name[30];
     char Size[16];
     char Address[16];
};


struct seviri_marf_l15_main_product_header_data {
     struct seviri_marf_l15_ph_data_data FormatName;
     struct seviri_marf_l15_ph_data_data FormatDocumentName;
     struct seviri_marf_l15_ph_data_data DocumentMajorVersion;
     struct seviri_marf_l15_ph_data_data DocumentMinorVersion;
     struct seviri_marf_l15_ph_data_data CreationDateTime;
     struct seviri_marf_l15_ph_data_data CreatingCenter;
     struct seviri_marf_l15_ph_data_id_data DataSetIdentification[27];
     struct seviri_marf_l15_ph_data_data TotalFileSize;
     struct seviri_marf_l15_ph_data_data GORT;
     struct seviri_marf_l15_ph_data_data ASTI;
     struct seviri_marf_l15_ph_data_data LLOS;
     struct seviri_marf_l15_ph_data_data SNIT;
     struct seviri_marf_l15_ph_data_data AIID;
     struct seviri_marf_l15_ph_data_data SSBT;
     struct seviri_marf_l15_ph_data_data SSST;
     struct seviri_marf_l15_ph_data_data RRCC;
     struct seviri_marf_l15_ph_data_data RRBT;
     struct seviri_marf_l15_ph_data_data RRST;
     struct seviri_marf_l15_ph_data_data PPRC;
     struct seviri_marf_l15_ph_data_data PPDT;
     struct seviri_marf_l15_ph_data_data GPLV;
     struct seviri_marf_l15_ph_data_data APNM;
     struct seviri_marf_l15_ph_data_data AARF;
     struct seviri_marf_l15_ph_data_data UUDT;
     struct seviri_marf_l15_ph_data_data QQOV;
     struct seviri_marf_l15_ph_data_data UDSP;
};


struct seviri_marf_l15_secondary_product_header_data {
     struct seviri_marf_l15_ph_data_data ABID;
     struct seviri_marf_l15_ph_data_data SMOD;
     struct seviri_marf_l15_ph_data_data APXS;
     struct seviri_marf_l15_ph_data_data AVPA;
     struct seviri_marf_l15_ph_data_data LSCD;
     struct seviri_marf_l15_ph_data_data LMPA;
     struct seviri_marf_l15_ph_data_data QDLC;
     struct seviri_marf_l15_ph_data_data QDLP;
     struct seviri_marf_l15_ph_data_data QQAI;
     struct seviri_marf_l15_ph_data_data SelectedBandIDs;
     struct seviri_marf_l15_ph_data_data SouthLineSelectedRectangle;
     struct seviri_marf_l15_ph_data_data NorthLineSelectedRectangle;
     struct seviri_marf_l15_ph_data_data EastColumnSelectedRectangle;
     struct seviri_marf_l15_ph_data_data WestColumnSelectedRectangle;
     struct seviri_marf_l15_ph_data_data NumberLinesVISIR;
     struct seviri_marf_l15_ph_data_data NumberColumnsVISIR;
     struct seviri_marf_l15_ph_data_data NumberLinesHRV;
     struct seviri_marf_l15_ph_data_data NumberColumnsHRV;
};


struct seviri_marf_header_data {
     struct seviri_marf_l15_main_product_header_data main;
     struct seviri_marf_l15_secondary_product_header_data secondary;
};



/*******************************************************************************
 *
 ******************************************************************************/
enum GP_SC_ID {
     GP_SC_ID_NOSPACECRAFT,
     GP_SC_ID_METEOSAT_3,
     GP_SC_ID_METEOSAT_4,
     GP_SC_ID_METEOSAT_5,
     GP_SC_ID_METEOSAT_6,
     GP_SC_ID_MTP_1,
     GP_SC_ID_MTP_2,
     GP_SC_ID_MSG_1,
     GP_SC_ID_MSG_2,
     GP_SC_ID_MSG_3,
     GP_SC_ID_MSG_4,
     GP_SC_ID_METOP_1,
     GP_SC_ID_METOP_2,
     GP_SC_ID_METOP_3,
     GP_SC_ID_NOAA_12,
     GP_SC_ID_NOAA_13,
     GP_SC_ID_NOAA_14,
     GP_SC_ID_NOAA_15,
     GP_SC_ID_NOAA_16,
     GP_SC_ID_NOAA_17,
     GP_SC_ID_GOES_7,
     GP_SC_ID_GOES_8,
     GP_SC_ID_GOES_9,
     GP_SC_ID_GOES_10,
     GP_SC_ID_GOES_11,
     GP_SC_ID_GOES_12,
     GP_SC_ID_GOMS_1,
     GP_SC_ID_GOMS_2,
     GP_SC_ID_GOMS_3,
     GP_SC_ID_GMS_4,
     GP_SC_ID_GMS_5,
     GP_SC_ID_GMS_6,
     GP_SC_ID_MTSAT_1,
     GP_SC_ID_MTSAT_1R,
     GP_SC_ID_MTSAT_2,
     GP_SC_ID_UNKNOWNSPACECRAFT,

     N_GP_SC_IDS
};


/* gp_sc_name is a char [12] */


struct seviri_TIME_CDS_data {
     short day;
     int msec;
     short usec;
};


struct seviri_TIME_CDS_SHORT_data {
     short day;
     int msec;
};


struct seviri_TIME_CDS_EXPANDED_data {
     short day;
     int msec;
     short usec;
     short nsec;
};


/*******************************************************************************
 * Native MSG level 1.5 header
 ******************************************************************************/
struct seviri_15HEADER_SatelliteStatus_ORBITCOEF_data {
     struct seviri_TIME_CDS_SHORT_data StartTime;
     struct seviri_TIME_CDS_SHORT_data EndTime;
     double X[8];
     double Y[8];
     double Z[8];
     double VX[8];
     double VY[8];
     double VZ[8];
};


struct seviri_15HEADER_SatelliteStatus_data {
     /* SatelliteDefinition */
     short SatelliteId;
     float NominalLongitude;
     uchar SatelliteStatus;

     /* SatelliteOperations */
     uchar LastManoeuvreFlag;
     struct seviri_TIME_CDS_SHORT_data LastManoeuvreStartTime;
     struct seviri_TIME_CDS_SHORT_data LastManoeuvreEndTime;
     uchar LastManoeuvreType;
     uchar NextManoeuvreFlag;
     struct seviri_TIME_CDS_SHORT_data NextManoeuvreStartTime;
     struct seviri_TIME_CDS_SHORT_data NextManoeuvreEndTime;
     uchar NextManoeuvreType;

     /* Orbit */
     struct seviri_TIME_CDS_SHORT_data PeriodStartTime;
     struct seviri_TIME_CDS_SHORT_data PeriodEndTime;
     struct seviri_15HEADER_SatelliteStatus_ORBITCOEF_data OrbitPolynomial[100];

     uchar Attitude[20420];
     double SpinRateatRCStart;
     uchar UTCCorrelation[59];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15HEADER_ImageAcquisition_data {
     /* PlannedAcquisitionTime */
     struct seviri_TIME_CDS_EXPANDED_data TrueRepeatCycleStart;
     struct seviri_TIME_CDS_EXPANDED_data PlannedForwardScanEnd;
     struct seviri_TIME_CDS_EXPANDED_data PlannedRepeatCycleEnd;

     /* RadiometerStatus */
     uchar ChannelStatus[12];
     uchar DetectorStatus[42];

     /* RadiometerSettings */
     ushort MDUSamplingDelays[42];

     /* HRVFrameOffsets */
     ushort MDUNomHRVDelay1;
     ushort MDUNomHRVDelay2;
     uchar Spare[2];
     ushort MDUNomHRVBreakline;

     uchar DHSSSynchSelection;
     ushort MDUOutGain[42];
     uchar MDUCourseGain[42];
     ushort MDUFineGain[42];
     ushort MDUNumericalOffset[42];
     ushort PUGain[42];
     ushort PUOffset[27];
     ushort PUBias[15];

     /* Operation Parameters */
     ushort L0_LineCounter;
     ushort K1_RetraceLines;
     ushort K2_PauseDeciseconds;
     ushort K3_RetraceLines;
     ushort K4_PauseDeciseconds;
     ushort K5_RetraceLines;
     uchar X_DeepSpaceWindowPos;

     ushort RefocusingLines;
     uchar RefocusingDirection;
     ushort RefocusingPosition;
     uchar ScanRefPosFlag;
     ushort ScanRefPosNumber;
     float ScanRefPotVal;
     ushort ScanFirstLine;
     ushort ScanLastLine;
     ushort RetraceStartLine;

     /* RadiometerOperations */
     uchar LastGainChangeFlag;
     struct seviri_TIME_CDS_SHORT_data LastGainChangeTime;

     /* Decontamination */
     uchar  DecontaminationNow;
     struct seviri_TIME_CDS_SHORT_data DecontaminationStart;
     struct seviri_TIME_CDS_SHORT_data DecontaminationEnd;

     uchar BBCalScheduled;
     uchar BBCalibrationType;
     ushort BBFirstLine;
     ushort BBLastLine;
     ushort ColdFocalPlaneOpTemp;
     ushort WarmFocalPlaneOpTemp;
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
enum seviri_StarId {
     seviri_StarId_Rigel = 1,
     seviri_StarId_Procyon,
     seviri_StarId_Betelgeuse,
     seviri_StarId_Alphard,
     seviri_StarId_Altair,
     seviri_StarId_Bellatrix,
     seviri_StarId_Alnilam,
     seviri_StarId_Alnitak,
     seviri_StarId_Yed_Prior,
     seviri_StarId_Mintaka,
     seviri_StarId_53KapOri,
     seviri_StarId_Zubeneschamali,
     seviri_StarId_Sadalsuud,
     seviri_StarId_67BetEri,
     seviri_StarId_Porrima,
     seviri_StarId_Sadalmelik,
     seviri_StarId_Menkar,
     seviri_StarId_Cebelrai,
     seviri_StarId_Unukalhai,
     seviri_StarId_Enif,

     N_StarIds
};


struct seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_data {
     uchar dummy[134915];
};


struct seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_data {
     struct seviri_TIME_CDS_SHORT_data StartTime;
     struct seviri_TIME_CDS_SHORT_data EndTime;
     double AlphaCoef[8];
     double BetaCoef[8];
};


struct seviri_15HEADER_CelestialEvents_STARCOEF_data {
     ushort StarId;
     struct seviri_TIME_CDS_SHORT_data StartTime;
     struct seviri_TIME_CDS_SHORT_data EndTime;
     double AlphaCoef[8];
     double BetaCoef[8];
};


struct seviri_15HEADER_CelestialEvents_data {
     /* CelestialBodiesPosition */
     struct seviri_TIME_CDS_SHORT_data PeriodStartTime;
     struct seviri_TIME_CDS_SHORT_data PeriodEndTime;
     struct seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_data
          RelatedOrbitFileTime;
     struct seviri_15HEADER_CelestialEvents_TIME_GENERALIZED_data
          RelatedAttitudeFileTime;
     struct seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_data
          EarthEphemeris[100];
     struct seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_data
          MoonEphemeris[100];
     struct seviri_15HEADER_CelestialEvents_EARTHMOONSUNCOEF_data
          SunEphemeris[100];
     struct seviri_15HEADER_CelestialEvents_STARCOEF_data
          StarEphemeris[100];

     /* RelationToImage */
     uchar TypeofEclipse;
     struct seviri_TIME_CDS_SHORT_data EclipseStartTime;
     struct seviri_TIME_CDS_SHORT_data EclipseEndTime;
     uchar VisibleBodiesInImage;
     uchar BodiesClosetoFOV;
     uchar ImpactOnImageQuality;
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15HEADER_ImageDescription_ReferenceGridVIS_IR_data {
     int NumberOfLines;
     int NumberOfColumns;
     float LineDirGridStep;
     float ColumnDirGridStep;
     uchar GridOrigin;
};


struct seviri_15HEADER_ImageDescription_ReferenceGridHRV_data {
     int NumberOfLines;
     int NumberOfColumns;
     float LineDirGridStep;
     float ColumnDirGridStep;
     uchar GridOrigin;
};


struct seviri_15HEADER_ImageDescription_PlannedCoverageVIS_IR_data {
     int SouthernLinePlanned;
     int NorthernLinePlanned;
     int EasternColumnPlanned;
     int WesternColumnPlanned;
};


struct seviri_15HEADER_ImageDescription_PlannedCoverageHRV_data {
     int LowerSouthLinePlanned;
     int LowerNorthLinePlanned;
     int LowerEastColumnPlanned;
     int LowerWestColumnPlanned;
     int UpperSouthLinePlanned;
     int UpperNorthLinePlanned;
     int UpperEastColumnPlanned;
     int UpperWestColumnPlanned;
};


struct seviri_15HEADER_ImageDescription_data {
     /* ProjectionDescription */
     uchar TypeOfProjection;
     float LongitudeOfSSP;

     struct seviri_15HEADER_ImageDescription_ReferenceGridVIS_IR_data
          ReferenceGridVIS_IR;
     struct seviri_15HEADER_ImageDescription_ReferenceGridHRV_data
          ReferenceGridHRV;
     struct seviri_15HEADER_ImageDescription_PlannedCoverageVIS_IR_data
          PlannedCoverageVIS_IR;
     struct seviri_15HEADER_ImageDescription_PlannedCoverageHRV_data
          PlannedCoverageHRV;

     /* Level_1_5_ImageProduction */
     uchar ImageProcDirection;
     uchar PixelGenDirection;
     uchar PlannedChanProcessing[12];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15HEADER_RadiometricProcessing_Level1_5ImageCalibration_data {
     double Cal_Slope;
     double Cal_Offset;
};


struct seviri_15HEADER_RadiometricProcessing_data {
     /* RPSummary */
     uchar RadianceLinearization[12];
     uchar DetectorEqualization[12];
     uchar OnBoardCalibrationResult[12];
     uchar MPEFCalFeedback[12];
     uchar MTFAdaptation[12];
     uchar StraylightCorrectionFlag[12];

     /* Level1_5ImageCalibration */
     struct seviri_15HEADER_RadiometricProcessing_Level1_5ImageCalibration_data
          Level1_5ImageCalibration[12];

     uchar dummy[20551];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15HEADER_GeometricProcessing_data {
     /* OptAxisDistances */
     float E_WFocalPlane[42];
     float N_SFocalPlane[42];

     /* EarthModel */
     uchar TypeOfEarthModel;
     double EquatorialRadius;
     double NorthPolarRadius;
     double SouthPolarRadius;

     float AtmosphericModel[12][360];
     uchar ResamplingFunctions[12];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15HEADER_IMPFConfiguration_data {
     uchar dummy[19786];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15HEADER_data {
     uchar _15HeaderVersion;

     struct seviri_15HEADER_SatelliteStatus_data       SatelliteStatus;
     struct seviri_15HEADER_ImageAcquisition_data      ImageAcquisition;
     struct seviri_15HEADER_CelestialEvents_data       CelestialEvents;
     struct seviri_15HEADER_ImageDescription_data      ImageDescription;
     struct seviri_15HEADER_RadiometricProcessing_data RadiometricProcessing;
     struct seviri_15HEADER_GeometricProcessing_data   GeometricProcessing;
     struct seviri_15HEADER_IMPFConfiguration_data     IMPFConfiguration;
};


/*******************************************************************************
 * Native MSG level 1.5 trailer
 ******************************************************************************/
struct seviri_15TRAILER_ImageProductionStats_L15ImageValidity_data {
     uchar NominalImage;
     uchar NonNominalBecauseIncomplete;
     uchar NonNominalRadiometricQuality;
     uchar NonNominalGeometricQuality;
     uchar NonNominalTimeliness;
     uchar IncompleteL15;
};


struct seviri_15TRAILER_ImageProductionStats_data {

     short SatelliteID;

     /* ActualScanningSummary */
     uchar NominalImageScanning;
     uchar ReducedScan;
     struct seviri_TIME_CDS_SHORT_data ActScanForwardStart;
     struct seviri_TIME_CDS_SHORT_data ActScanForwardEnd;

     /* RadiometerBehaviour */
     uchar NominalBehaviour;
     uchar RadScanIrregularity;
     uchar RadStoppage;
     uchar RepeatCycleNotCompleted;
     uchar GainChangeTookPlace;
     uchar DecontaminationTookPlace;
     uchar NoBBCalibrationAchieved;
     uchar IncorrectTemperature;
     uchar InvalidBBData;
     uchar InvalidAuxOrHKTMData;
     uchar RefocusingMechanismActuated;
     uchar MirrorBackToReferencePos;

     /* ReceptionSummaryStats */
     uint PlannedNumberOfL10Lines[12];
     uint NumberOfMissingL10Lines[12];
     uint NumberOfCorruptedL10Lines[12];
     uint NumberOfReplacedL10Lines[12];

     struct seviri_15TRAILER_ImageProductionStats_L15ImageValidity_data
          L15ImageValidity[12];

     /* ActualL15CoverageVIS_IR */
     int SouthernLineActual;
     int NorthernLineActual;
     int EasternColumnActual;
     int WesternColumnActual;

     /* Actual L15CoverageHRV */
     int LowerSouthLineActual;
     int LowerNorthLineActual;
     int LowerEastColumnActual;
     int LowerWestColumnActual;
     int UpperSouthLineActual;
     int UpperNorthLineActual;
     int UpperEastColumnActual;
     int UpperWestColumnActual;
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15TRAILER_NavigationExtractionResults_HORIZONOBSERVATION_data {
     uchar HorizonId;
     double Alpha;
     double AlphaConfidence;
     double Beta;
     double BetaConfidence;
     struct seviri_TIME_CDS_data ObservationTime;
     double SpinRate;
     double AlphaDeviation;
     double BetaDeviation;
};


struct seviri_15TRAILER_NavigationExtractionResults_STAROBSERVATION_data {
     ushort StarId;
     double Alpha;
     double AlphaConfidence;
     double Beta;
     double BetaConfidence;
     struct seviri_TIME_CDS_data ObservationTime;
     double SpinRate;
     double AlphaDeviation;
     double BetaDeviation;
};


struct seviri_15TRAILER_NavigationExtractionResults_LANDMARKOBSERVATION_data {
     ushort LandmarkId;
     double LandmarkLongitude;
     double LandmarkLatitude;
     double Alpha;
     double AlphaConfidence;
     double Beta;
     double BetaConfidence;
     struct seviri_TIME_CDS_data ObservationTime;
     double SpinRate;
     double AlphaDeviation;
     double BetaDeviation;
};


struct seviri_15TRAILER_NavigationExtractionResults_data {
     struct seviri_15TRAILER_NavigationExtractionResults_HORIZONOBSERVATION_data
          ExtractedHorizons[4];
     struct seviri_15TRAILER_NavigationExtractionResults_STAROBSERVATION_data
          ExtractedStars[20];
     struct seviri_15TRAILER_NavigationExtractionResults_LANDMARKOBSERVATION_data
          ExtractedLandmarks[50];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15TRAILER_RadiometricQuality_L10RadQuality_data {
     short FullImageMinimumCount;
     short FullImageMaximumCount;
     short EarthDiskMinimumCount;
     short EarthDiskMaximumCount;
     short MoonMinimumCount;
     short MoonMaximumCount;
     float FullImageMeanCount;
     float FullImageStandardDeviation;
     float EarthDiskMeanCount;
     float EarthDiskStandardDeviation;
     float MoonMeanCount;
     float MoonStandardDeviation;
     float SpaceMeanCount;
     float SpaceStandardDeviation;
     float SESpaceCornerMeanCount;
     float SESpaceCornerStandardDeviation;
     float SWSpaceCornerMeanCount;
     float SWSpaceCornerStandardDeviation;
     float NESpaceCornerMeanCount;
     float NESpaceCornerStandardDeviation;
     float NWSpaceCornerMeanCount;
     float NWSpaceCornerStandardDeviation;
     float FourSpaceCornersMeanCount;
     float FourSpaceCornersStandardDev;
     uint FullImageHistogram[256];
     uint EarthDiskHistogram[256];
     uint ImageCenterSquareHistogram[256];
     uint SESpaceCornerHistogram[128];
     uint SWSpaceCornerHistogram[128];
     uint NESpaceCornerHistogram[128];
     uint NWSpaceCornerHistogram[128];
     float FullImageEntropy[3];
     float EarthDiskEntropy[3];
     float ImageCenterSquareEntropy[3];
     float SESpaceCornerEntropy[3];
     float SWSpaceCornerEntropy[3];
     float NESpaceCornerEntropy[3];
     float NWSpaceCornerEntropy[3];
     float FourSpaceCornersEntropy[3];
     float ImageCenterSquarePSD_EW[128];
     float FullImagePSD_EW[128];
     float ImageCenterSquarePSD_NS[128];
     float FullImagePSD_NS[128];
};


struct seviri_15TRAILER_RadiometricQuality_L15RadQuality_data {
     int FullImageMinimumCount;
     int FullImageMaximumCount;
     int EarthDiskMinimumCount;
     int EarthDiskMaximumCount;
     float FullImageMeanCount;
     float FullImageStandardDeviation;
     float EarthDiskMeanCount;
     float EarthDiskStandardDeviation;
     float SpaceMeanCount;
     float SpaceStandardDeviation;
     uint FullImageHistogram[256];
     uint EarthDiskHistogram[256];
     uint ImageCenterSquareHistogram[256];
     float FullImageEntropy[3];
     float EarthDiskEntropy[3];
     float ImageCenterSquareEntropy[3];
     float ImageCenterSquarePSD_EW[128];
     float FullImagePSD_EW[128];
     float ImageCenterSquarePSD_NS[128];
     float FullImagePSD_NS[128];
     float SESpaceCornerL15_RMS;
     float SESpaceCornerL15_Mean;
     float SWSpaceCornerL15_RMS;
     float SWSpaceCornerL15_Mean;
     float NESpaceCornerL15_RMS;
     float NESpaceCornerL15_Mean;
     float NWSpaceCornerL15_RMS;
     float NWSpaceCornerL15_Mean;
};


struct seviri_15TRAILER_RadiometricQuality_data {
     struct seviri_15TRAILER_RadiometricQuality_L10RadQuality_data
          L10RadQuality[42];
     struct seviri_15TRAILER_RadiometricQuality_L15RadQuality_data
          L15RadQuality[42];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15TRAILER_GeometricQuality_Accuracy_data {
     uchar QualityInfoValidity;
     float EastWestAccuracyRMS;
     float NorthSouthAccuracyRMS;
     float MagnitudeRMS;
     float EastWestUncertaintyRMS;
     float NorthSouthUncertaintyRMS;
     float MagnitudeUncertaintyRMS;
     float EastWestMaxDeviation;
     float NorthSouthMaxDeviation;
     float MagnitudeMaxDeviation;
     float EastWestUncertaintyMax;
     float NorthSouthUncertaintyMax;
     float MagnitudeUncertaintyMax;
};


struct seviri_15TRAILER_GeometricQuality_MisregistrationResiduals_data {
     uchar QualityInfoValidity;
     float EastWestResidual;
     float NorthSouthResidual;
     float EastWestUncertainty;
     float NorthSouthUncertainty;
     float EastWestRMS;
     float NorthSouthRMS;
     float EastWestMagnitude;
     float NorthSouthMagnitude;
     float EastWestMagnitudeUncertainty;
     float NorthSouthMagnitudeUncertainty;
};


struct seviri_15TRAILER_GeometricQuality_GeometricQualityStatus_data {
     uchar QualityNominal;
     uchar NominalAbsolute;
     uchar NominalRelativeToPreviousImage;
     uchar NominalForREL500;
     uchar NominalForREL16;
     uchar NominalForResMisreg;
};


struct seviri_15TRAILER_GeometricQuality_data {
     struct seviri_15TRAILER_GeometricQuality_Accuracy_data
          AbsoluteAccuracy[12];
     struct seviri_15TRAILER_GeometricQuality_Accuracy_data
          RelativeAccuracy[12];
     struct seviri_15TRAILER_GeometricQuality_Accuracy_data
          _500PixelsRelativeAccuracy[12];
     struct seviri_15TRAILER_GeometricQuality_Accuracy_data
          _16PixelsRelativeAccuracy[12];
     struct seviri_15TRAILER_GeometricQuality_MisregistrationResiduals_data
          MisregistrationResiduals[12];
     struct seviri_15TRAILER_GeometricQuality_GeometricQualityStatus_data
          GeometricQualityStatus[12];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15TRAILER_TimelinessAndCompleteness_Completeness_data {
     ushort PlannedL15ImageLines;
     ushort GeneratedL15ImageLines;
     ushort ValidL15ImageLines;
     ushort DummyL15ImageLines;
     ushort CorruptedL15ImageLines;
};


struct seviri_15TRAILER_TimelinessAndCompleteness_data {
     float MaxDelay;
     float MinDelay;
     float MeanDelay;

     struct seviri_15TRAILER_TimelinessAndCompleteness_Completeness_data
          Completeness[12];
};


/*------------------------------------------------------------------------------
 *
 *----------------------------------------------------------------------------*/
struct seviri_15TRAILER_data {
     uchar L15TrailerVersion;
     struct seviri_15TRAILER_ImageProductionStats_data
          ImageProductionStats;
     struct seviri_15TRAILER_NavigationExtractionResults_data
          NavigationExtractionResults;
     struct seviri_15TRAILER_RadiometricQuality_data
          RadiometricQuality;
     struct seviri_15TRAILER_GeometricQuality_data
          GeometricQuality;
     struct seviri_15TRAILER_TimelinessAndCompleteness_data
          TimelinessAndCompleteness;

     uchar dummy[214];
};


/*******************************************************************************
 * Packet header
 ******************************************************************************/
struct seviri_packet_header_data {
     uchar HeaderVersionNo;
     uchar PacketType;
     uchar SubHeaderType;
     uchar SourceFacilityId;
     uchar SourceEnvId;
     uchar SourceInstanceId;
     int  SourceSUId;
     uchar SourceCPUId[4];
     uchar DestFacilityId;
     uchar DestEnvId;
     ushort SequenceCount;
     int  PacketLength;

     uchar SubHeaderVersionNo;
     uchar ChecksumFlag;
     uchar Acknowledgement[4];
     uchar ServiceType;
     uchar ServiceSubtype;
     uchar PacketTime[6];
     short SpacecraftId;
};


/*******************************************************************************
 * Line side information
 ******************************************************************************/
struct seviri_LineSideInfo_data {
     uchar _15LINEVersion;

     short SatelliteId;
     struct seviri_TIME_CDS_EXPANDED_data TrueRepeatCycleStart;
     int  LineNumberInGrid;
     uchar ChannelId;
     struct seviri_TIME_CDS_SHORT_data L10LineMeanAcquisitionTime;
     uchar LineValidity;
     uchar LineRadiometricQuality;
     uchar LineGeometricQuality;
};


/*******************************************************************************
 * Struct containing auxiliary variables related to configuring the read/write
 * functions.
 ******************************************************************************/
struct seviri_auxillary_io_data {
     int operation;

     int swap_bytes;

     ushort *temp_2;
     uint   *temp_4;
     ulong  *temp_8;
};


/*******************************************************************************
 * Struct containing offsets and dimensions that will passed from the read to
 * write functions.
 ******************************************************************************/
struct seviri_dimension_data {
     uint n_lines_selected_VIR;
     uint n_columns_selected_VIR;

     uint i0_line_selected_VIR;
     uint i1_line_selected_VIR;
     uint i0_column_selected_VIR;
     uint i1_column_selected_VIR;


     uint i_line_requested_VIR;
     uint i_column_requested_VIR;

     uint n_lines_requested_VIR;
     uint n_columns_requested_VIR;

     uint i_line_to_read_VIR;
     uint i_column_to_read_VIR;

     uint n_lines_to_read_VIR;
     uint n_columns_to_read_VIR;

     uint i_line_in_output_VIR;
     uint i_column_in_output_VIR;


     uint n_lines_selected_HRV;
     uint n_columns_selected_HRV;
};


/*******************************************************************************
 * Digital count image data
 ******************************************************************************/
struct seviri_image_data {
     uint i_line;	/* line in the full disk that the sub-image starts at */
     uint i_column;	/* column in the full disk that the sub-image starts at */
     uint n_lines;	/* number of lines in the sub-image */
     uint n_columns;	/* number of columns in the sub-image */

     uint n_bands;	/* number of bands read in */

     uint band_ids[SEVIRI_N_BANDS];
			/* array of bands read inn of length n_bands */

     ushort fill_value;	/* fill value of the image data */

     struct seviri_packet_header_data **packet_header;
			/* array of pointers to packet headers of length n_bands * n_lines */
     struct seviri_LineSideInfo_data **LineSideInfo;
			/* array of pointers to line side info's of length n_bands * n_lines */

     ushort **data_vir;	/* array of visible and infrared image arrays of length n_bands */
     ushort  *data_hrv;

     void *dimens;
};



/*******************************************************************************
 * The main SEVIRI data struct with arrays of digital counts
 ******************************************************************************/
struct seviri_native_data {
     struct seviri_marf_header_data marf_header;

     struct seviri_packet_header_data packet_header1;
     struct seviri_15HEADER_data header;

     struct seviri_image_data image;

     struct seviri_packet_header_data packet_header2;
     struct seviri_15TRAILER_data trailer;
};



int seviri_auxillary_alloc(struct seviri_auxillary_io_data *d);
int seviri_auxillary_free(struct seviri_auxillary_io_data *d);

int fxxxx_swap(void *ptr, size_t size, size_t nmemb, FILE *stream, struct
               seviri_auxillary_io_data *aux);

int seviri_l15_ph_data_read(FILE *fp,
                            struct seviri_marf_l15_ph_data_data *d,
                            struct seviri_auxillary_io_data *aux);
int seviri_l15_ph_data_id_read(FILE *fp,
                               struct seviri_marf_l15_ph_data_id_data *d,
                               struct seviri_auxillary_io_data *aux);

int seviri_marf_header_read(FILE *fp, struct seviri_marf_header_data *d,
                            struct seviri_auxillary_io_data *aux);

int seviri_TIME_CDS_read(FILE *fp,
                         struct seviri_TIME_CDS_data *d,
                         struct seviri_auxillary_io_data *aux);
int seviri_TIME_CDS_SHORT_read(FILE *fp,
                               struct seviri_TIME_CDS_SHORT_data *d,
                               struct seviri_auxillary_io_data *aux);
int seviri_TIME_CDS_EXPANDED_read(FILE *fp,
                                  struct seviri_TIME_CDS_EXPANDED_data *d,
                                  struct seviri_auxillary_io_data *aux);

int seviri_15HEADER_SatelliteStatus_read(
          FILE *fp,
          struct seviri_15HEADER_SatelliteStatus_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_ImageAcquisition_read(
          FILE *fp,
          struct seviri_15HEADER_ImageAcquisition_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_CelestialEvents_read(
          FILE *fp,
          struct seviri_15HEADER_CelestialEvents_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_ImageDescription_read(
          FILE *fp,
          struct seviri_15HEADER_ImageDescription_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_RadiometricProcessing_read(
          FILE *fp,
          struct seviri_15HEADER_RadiometricProcessing_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_GeometricProcessing_read(
          FILE *fp,
          struct seviri_15HEADER_GeometricProcessing_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_IMPFConfiguration_read(
          FILE *fp,
          struct seviri_15HEADER_IMPFConfiguration_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_15HEADER_read(
          FILE *fp,
          struct seviri_15HEADER_data *d,
          struct seviri_auxillary_io_data *aux);

int seviri_15TRAILER_read(
          FILE *fp,
          struct seviri_15TRAILER_data *d,
          struct seviri_auxillary_io_data *aux);

int seviri_packet_header_read(
          FILE *fp,
          struct seviri_packet_header_data *d,
          struct seviri_auxillary_io_data *aux);
int seviri_LineSideInfo_read(
          FILE *fp,
          struct seviri_LineSideInfo_data *d,
          struct seviri_auxillary_io_data *aux);

int seviri_get_dimension_data(
          struct seviri_dimension_data *d,
          const struct seviri_marf_header_data *marf_header,
          enum seviri_bounds bounds,
          uint line0, uint line1, uint column0, uint column1,
          double lat0, double lat1, double lon0, double lon1);

int seviri_native_free(struct seviri_native_data *d);


#ifdef __cplusplus
}
#endif

#endif /* READ_WRITE_H */
