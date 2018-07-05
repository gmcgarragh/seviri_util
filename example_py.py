#! /usr/bin/env python

# Example program calling seviri_util to read a native SEVIRI level 1.5 image
# file and preprocess it to obtain several fields.


# Used to to get the filename from argv.
import sys

# Include the seviri_util module.
import seviri_util

# Create a seviri_preproc object with 4 bands: 0.635 and 1.64 reflectance and
# 8.70 and 10.80 brightness temperature a sub-image defined by pixel coordinates.
try:
    util = seviri_util.seviri_preproc(sys.argv[1], [1, 3, 7, 9], ['BRF', 'BRF',
        'BT', 'BT'], 'line_column', pixel_coords = (1899, 2199, 1700, 2299),
        do_gsics = False)
except seviri_preproc.error:
    print('ERROR: seviri_preproc.init()', file=sys.stderr)
    exit()

# Print the values for the central pixel.
i_line   = util.n_lines / 2
i_column = util.n_columns / 2

print('i_line:                       %d'    % i_line)
print('i_column:                     %d'    % i_column)
print('Julian Day Number:            % .8e' % util.time[i_line, i_column])
print('latitude:                     % .8e' % util.lat[i_line, i_column])
print('longitude:                    % .8e' % util.lon [i_line, i_column])
print('solar zenith angle:           % .8e' % util.sza [i_line, i_column])
print('solar azimuth angle:          % .8e' % util.saa [i_line, i_column])
print('viewing zenith angle:         % .8e' % util.vza [i_line, i_column])
print('viewing azimuth angle:        % .8e' % util.vaa [i_line, i_column])
print('0.635 reflectance:            % .8e' % util.data[0, i_line, i_column])
print('1.64  reflectance:            % .8e' % util.data[1, i_line, i_column])
print('8.70  brightness temperature: % .8e' % util.data[2, i_line, i_column])
print('10.80 brightness temperature: % .8e' % util.data[3, i_line, i_column])
