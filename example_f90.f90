! Example program calling seviri_util to read a native SEVIRI level 1.5 image
! file and preprocess it to obtain several fields.

program example_f90

     ! Use the seviri_util interface module.
     use seviri_util

     implicit none

     ! Filename of the file to be read.
     character(1024) :: filename
     character(128)  :: satposstr

     integer :: i_line
     integer :: i_column

     ! We want to read 4 bands: 0.635 and 1.64 reflectance and 8.70 and 10.80
     ! brightness temperature.
     integer, parameter :: n_bands = 4

     integer, parameter :: band_ids(n_bands) = (/1, 2,3,4/)

     integer, parameter :: band_units(n_bands) = (/SEVIRI_UNIT_BRF, &
                                                   SEVIRI_UNIT_BRF, &
                                                   SEVIRI_UNIT_BRF, &
                                                   SEVIRI_UNIT_BRF/)

     ! We want to read a sub-image defined by pixel coordinates.
     integer, parameter :: line0   = 1
     integer, parameter :: line1   = 3712
     integer, parameter :: column0 = 1
     integer, parameter :: column1 = 3712

     ! This struct will contain the image data and some metadata.
     type(seviri_preproc_t_f90) :: preproc

     ! Get the filename of the file to be read from the command line.
     if (command_argument_count() .lt. 1) then
        print *, "ERROR: Filename missing from command line."
        stop 1
     endif

     call get_command_argument(1,filename)

     ! Read and preprocess the data.  Note: the last four arguments are unused
     ! in this case. Please see the functionn header for a complete discussion
     ! of the arrguments.
     if (seviri_read_and_preproc_f90(trim(filename), preproc, n_bands, band_ids, &
                            band_units, SEVIRI_BOUNDS_LINE_COLUMN, line0, line1, &
                            column0, column1, 0.d0, 0.d0, 0.d0, 0.d0, .false.,   &
                            .false., satposstr, .false.) .ne. 0) then
        print *, 'ERROR: seviri_read_and_preproc_f90()'
        stop -1
     end if

     ! Print the values for the central pixel.
     i_line   = preproc%n_lines / 2 + 1
     i_column = preproc%n_columns / 2 + 1

     print '("i_line:                       ", I4)', i_line - 1
     print '("i_column:                     ", I4)', i_column - 1
     print '("Julian Day Number:            ", ES15.8)', preproc%time(i_column, i_line)
     print '("latitude:                     ", ES15.8)', preproc%lat (i_column, i_line)
     print '("longitude:                    ", ES15.8)', preproc%lon (i_column, i_line)
     print '("solar zenith angle:           ", ES15.8)', preproc%sza (i_column, i_line)
     print '("solar azimuth angle:          ", ES15.8)', preproc%saa (i_column, i_line)
     print '("viewing zenith angle:         ", ES15.8)', preproc%vza (i_column, i_line)
     print '("viewing azimuth angle:        ", ES15.8)', preproc%vaa (i_column, i_line)
     print '("0.635 reflectance:            ", ES15.8)', preproc%data(i_column, i_line, 1)
     print '("1.64  reflectance:            ", ES15.8)', preproc%data(i_column, i_line, 2)
     print '("8.70  brightness temperature: ", ES15.8)', preproc%data(i_column, i_line, 3)
     print '("10.80 brightness temperature: ", ES15.8)', preproc%data(i_column, i_line, 4)

     ! Free memory allocated by seviri_read_and_preproc().
     if (seviri_preproc_free_f90(preproc) .ne. 0) then
        print *, 'ERROR: seviri_preproc_free_f90()'
        stop -1
     end if

end program example_f90
