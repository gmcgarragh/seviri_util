; Example program calling seviri_util to read a native SEVIRI level 1.5 image
; file and preprocess it to obtain several fields.

; Create a seviri_preproc structure with 4 bands: 0.635 and 1.64 reflectance and
; 8.70 and 10.80 brightness temperature a sub-image defined by pixel coordinates.
seviri_preproc_dlm, $
     '/home/gregm/data/seviri/MSG1-SEVI-MSG15-0201-NA-20060303121241.nat', $
     [1, 3, 7, 9], $
     ['BRF', 'BRF', 'BT', 'BT'], $
     'line_column', $
     pixel_coords = [1899, 2199, 1700, 2299], $
     preproc

print, preproc.n_lines
print, preproc.n_columns

; Print the values for the central pixel.
;i_line   = util.n_lines / 2
;i_column = util.n_columns / 2

;print "i_line:                       %4d"   % i_line
;print "i_column:                     %4d"   % i_column
;print ""
;print "Julian Day Number:            % .8e" % util.time[i_line, i_column]
;print "latitude:                     % .8e" % util.lat [i_line, i_column]
;print "longitude:                    % .8e" % util.lon [i_line, i_column]
;print "solar zenith angle:           % .8e" % util.sza [i_line, i_column]
;print "solar azimuth angle:          % .8e" % util.saa [i_line, i_column]
;print "viewing zenith angle:         % .8e" % util.vza [i_line, i_column]
;print "viewing azimuth angle:        % .8e" % util.vaa [i_line, i_column]
;print "0.635 refectance:             % .8e" % util.data[0, i_line, i_column]
;print "1.64  refectance:             % .8e" % util.data[1, i_line, i_column]
;print "8.70  brightness temperature: % .8e" % util.data[2, i_line, i_column]
;print "10.80 brightness temprature:  % .8e" % util.data[3, i_line, i_column]

end
