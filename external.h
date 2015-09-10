/******************************************************************************%
 *
 *    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef EXTERNAL_H
#define EXTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif


#define SEVIRI_N_BANDS 12


typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned long	ulong;


enum seviri_bounds {
     SEVIRI_BOUNDS_FULL_DISK,
     SEVIRI_BOUNDS_ACTUAL_IMAGE,
     SEVIRI_BOUNDS_LINE_COLUMN,
     SEVIRI_BOUNDS_LAT_LON,

     N_SEVIRI_BOUNDS
};


enum seviri_units {
     SEVIRI_UNIT_CNT,
     SEVIRI_UNIT_RAD,
     SEVIRI_UNIT_REF,
     SEVIRI_UNIT_BRF,
     SEVIRI_UNIT_BT,

     N_SEVIRI_UNITS
};


#ifdef __cplusplus
}
#endif

#endif /* EXTERNAL_H */
