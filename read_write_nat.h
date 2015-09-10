/******************************************************************************%
 *
 *    Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
 *
 *    This source code is licensed under the GNU General Public License (GPL),
 *    Version 3.  See the file COPYING for more details.
 *
 ******************************************************************************/

#ifndef READ_WRITE_NAT_H
#define READ_WRITE_NAT_H

#include "external.h"
#include "read_write.h"

#ifdef __cplusplus
extern "C" {
#endif


int seviri_get_dimens_nat(const char *filename, uint *i_line, uint *i_column,
                          uint *n_lines, uint *n_columns, enum seviri_bounds bounds,
                          uint line0, uint line1, uint column0, uint column1,
                          double lat0, double lat1, double lon0, double lon1);
int seviri_read_nat(const char *filename, struct seviri_data *d,
                    uint n_bands, const uint *band_ids, enum seviri_bounds bounds,
                    uint line0, uint line1, uint column0, uint column1,
                    double lat0, double lat1, double lon0, double lon1);
int seviri_write_nat(const char *filename, const struct seviri_data *d);


#ifdef __cplusplus
}
#endif

#endif /* READ_WRITE_NAT_H */
