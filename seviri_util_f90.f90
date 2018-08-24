!*******************************************************************************
!
! Copyright (C) 2014-2018 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
! Copyright (C) 2018 Simon Proud <simon.proud@physics.ox.ac.uk>
!
! This source code is licensed under the GNU General Public License (GPL),
! Version 3.  See the file COPYING for more details.
!
!*******************************************************************************


!*******************************************************************************
! Module containing the Fortran interface to seviri_util.  Please see the
! comments in the C code for descriptions of the various routines including
! their arguments and return values.
!*******************************************************************************
module seviri_util

    use iso_c_binding

    implicit none

    private

    public :: seviri_preproc_t_f90, &
              seviri_get_dimens_nat_f90, &
              seviri_get_dimens_hrit_f90, &
              seviri_get_dimens_f90, &
              seviri_read_and_preproc_nat_f90, &
              seviri_read_and_preproc_hrit_f90, &
              seviri_read_and_preproc_f90, &
              seviri_preproc_free_f90


    integer, parameter, public :: SEVIRI_BOUNDS_FULL_DISK    = 0
    integer, parameter, public :: SEVIRI_BOUNDS_ACTUAL_IMAGE = 1
    integer, parameter, public :: SEVIRI_BOUNDS_LINE_COLUMN  = 2
    integer, parameter, public :: SEVIRI_BOUNDS_LAT_LON      = 3

    integer, parameter, public :: SEVIRI_UNIT_CNT            = 0
    integer, parameter, public :: SEVIRI_UNIT_RAD            = 1
    integer, parameter, public :: SEVIRI_UNIT_REF            = 2
    integer, parameter, public :: SEVIRI_UNIT_BRF            = 3
    integer, parameter, public :: SEVIRI_UNIT_BT             = 4


    type, bind(c) :: seviri_preproc_t
        integer(c_int) :: memory_alloc_d
        integer(c_int) :: n_bands
        integer(c_int) :: n_lines
        integer(c_int) :: n_columns
        real(c_float)  :: fill_value
        type(c_ptr)    :: time
        type(c_ptr)    :: lat
        type(c_ptr)    :: lon
        type(c_ptr)    :: sza
        type(c_ptr)    :: saa
        type(c_ptr)    :: vza
        type(c_ptr)    :: vaa
        type(c_ptr)    :: data
        type(c_ptr)    :: data2
    end type seviri_preproc_t

    type :: seviri_preproc_t_f90
        logical          :: memory_alloc_d
        integer          :: n_bands
        integer          :: n_lines
        integer          :: n_columns
        real(4)          :: fill_value
        real(8), pointer :: time(:, :)
        real(4), pointer :: lat(:, :)
        real(4), pointer :: lon(:, :)
        real(4), pointer :: sza(:, :)
        real(4), pointer :: saa(:, :)
        real(4), pointer :: vza(:, :)
        real(4), pointer :: vaa(:, :)
        real(4), pointer :: data(:, :, :)

        type(seviri_preproc_t) :: preproc
    end type seviri_preproc_t_f90


    interface
        integer(c_int) function seviri_get_dimens_nat(filename, &
            i_line, i_column, n_lines, n_columns, bounds, line0, line1, &
            column0, column1, lat0, lat1, lon0, lon1) &
            bind(C, name = 'seviri_get_dimens_nat')

            use iso_c_binding

            implicit none

            character(c_char), intent(in)        :: filename(*)
            integer(c_int),    intent(out)       :: i_line, i_column
            integer(c_int),    intent(out)       :: n_lines, n_columns
            integer(c_int),    intent(in), value :: bounds
            integer(c_int),    intent(in), value :: line0, line1, &
                                                 column0, column1
            real(c_double),    intent(in), value :: lat0, lat1, lon0, lon1
        end function seviri_get_dimens_nat
    end interface

    interface
        integer(c_int) function seviri_get_dimens_hrit(filename, timeslot, &
            satnum, i_line, i_column, n_lines, n_columns, bounds, line0, line1, &
            column0, column1, lat0, lat1, lon0, lon1) &
            bind(C, name = 'seviri_get_dimens_hrit')

            use iso_c_binding

            implicit none

            character(c_char), intent(in)        :: filename(*)
            character(c_char), intent(in)        :: timeslot(*)
            integer(c_int),    intent(in)        :: satnum
            integer(c_int),    intent(out)       :: i_line, i_column
            integer(c_int),    intent(out)       :: n_lines, n_columns
            integer(c_int),    intent(in), value :: bounds
            integer(c_int),    intent(in), value :: line0, line1, &
                                                    column0, column1
            real(c_double),    intent(in), value :: lat0, lat1, lon0, lon1
        end function seviri_get_dimens_hrit
    end interface

    interface
        integer(c_int) function seviri_get_dimens(filename, &
            i_line, i_column, n_lines, n_columns, bounds, line0, line1, &
            column0, column1, lat0, lat1, lon0, lon1) &
            bind(C, name = 'seviri_get_dimens')

            use iso_c_binding

            implicit none

            character(c_char), intent(in)        :: filename(*)
            integer(c_int),    intent(out)       :: i_line, i_column
            integer(c_int),    intent(out)       :: n_lines, n_columns
            integer(c_int),    intent(in), value :: bounds
            integer(c_int),    intent(in), value :: line0, line1, &
                                                    column0, column1
            real(c_double),    intent(in), value :: lat0, lat1, lon0, lon1
        end function seviri_get_dimens
    end interface

    interface
        integer(c_int) function seviri_read_and_preproc_nat(filename, preproc, &
            n_bands, band_ids, band_units, bounds, line0, line1, column0, column1, &
            lat0, lat1, lon0, lon1, do_calib, satposstr, do_not_alloc) bind(C, &
            name = 'seviri_read_and_preproc_nat')

            use iso_c_binding

            import seviri_preproc_t

            implicit none

            character(c_char),      intent(in)        :: filename(*)
            type(seviri_preproc_t), intent(out)       :: preproc
            integer(c_int),         intent(in), value :: n_bands
            integer(c_int),         intent(in)        :: band_ids(*)
            integer(c_int),         intent(in)        :: band_units(*)
            integer(c_int),         intent(in), value :: bounds
            integer(c_int),         intent(in), value :: line0, line1, &
                                                         column0, column1
            real(c_double),         intent(in), value :: lat0, lat1, lon0, lon1
            integer(c_int),         intent(in), value :: do_calib
            character(c_char),      intent(inout)     :: satposstr(128)
            integer(c_int),         intent(in), value :: do_not_alloc
        end function seviri_read_and_preproc_nat
    end interface

    interface
        integer(c_int) function seviri_read_and_preproc_hrit(filename, timeslot, &
            satnum, preproc, n_bands, band_ids, band_units, bounds, line0, line1, &
            column0, column1, lat0, lat1, lon0, lon1, rss, do_calib, satposstr, &
            do_not_alloc) bind(C, name = 'seviri_read_and_preproc_hrit')

            use iso_c_binding

            import seviri_preproc_t

            implicit none

            character(c_char),      intent(in)        :: filename(*)
            character(c_char),      intent(in)        :: timeslot(*)
            integer(c_int),         intent(in)        :: satnum
            type(seviri_preproc_t), intent(out)       :: preproc
            integer(c_int),         intent(in), value :: n_bands
            integer(c_int),         intent(in)        :: band_ids(*)
            integer(c_int),         intent(in)        :: band_units(*)
            integer(c_int),         intent(in), value :: bounds
            integer(c_int),         intent(in), value :: line0, line1, &
                                                         column0, column1
            real(c_double),         intent(in), value :: lat0, lat1, lon0, lon1
            integer(c_int),         intent(in), value :: rss
            integer(c_int),         intent(in), value :: do_calib
            character(c_char),      intent(inout)     :: satposstr(128)
            integer(c_int),         intent(in), value :: do_not_alloc
        end function seviri_read_and_preproc_hrit
    end interface

    interface
        integer(c_int) function seviri_read_and_preproc(filename, preproc, &
            n_bands, band_ids, band_units, bounds, line0, line1, column0, column1, &
            lat0, lat1, lon0, lon1, do_calib, satposstr, do_not_alloc) bind(C, &
            name = 'seviri_read_and_preproc')

            use iso_c_binding

            import seviri_preproc_t

            implicit none

            character(c_char),      intent(in)        :: filename(*)
            type(seviri_preproc_t), intent(out)       :: preproc
            integer(c_int),         intent(in), value :: n_bands
            integer(c_int),         intent(in)        :: band_ids(*)
            integer(c_int),         intent(in)        :: band_units(*)
            integer(c_int),         intent(in), value :: bounds
            integer(c_int),         intent(in), value :: line0, line1, &
                                                         column0, column1
            real(c_double),         intent(in), value :: lat0, lat1, lon0, lon1
            integer(c_int),         intent(in), value :: do_calib
            character(c_char),      intent(inout)     :: satposstr(128)
            integer(c_int),         intent(in), value :: do_not_alloc
        end function seviri_read_and_preproc
    end interface

    interface
        integer(c_int) function seviri_preproc_free(preproc) &
            bind(C, name = 'seviri_preproc_free')

            use iso_c_binding

            import seviri_preproc_t

            implicit none

            type(seviri_preproc_t), intent(inout) :: preproc
        end function seviri_preproc_free
    end interface

contains

integer function seviri_get_dimens_nat_f90(filename, i_line, i_column, &
    n_lines, n_columns, bounds, line0, line1, column0, column1, lat0, lat1, &
    lon0, lon1) result(status)

    implicit none

    character(*), intent(in)        :: filename
    integer,      intent(out)       :: i_line, i_column
    integer,      intent(out)       :: n_lines, n_columns
    integer,      intent(in), value :: bounds
    integer,      intent(in), value :: line0, line1, &
                                       column0, column1
    real(8),      intent(in), value :: lat0, lat1, lon0, lon1

    status = seviri_get_dimens_nat(trim(filename)//C_NULL_CHAR, &
        i_line, i_column, n_lines, n_columns, bounds, line0, line1, &
        column0, column1, lat0, lat1, lon0, lon1)
    if (status .ne. 0) then
        write(6, *) 'ERROR: seviri_get_dimens_nat()'
        return
    end if

end function seviri_get_dimens_nat_f90


integer function seviri_get_dimens_hrit_f90(filename, timeslot, satnum, &
    i_line, i_column, n_lines, n_columns, bounds, line0, line1, column0, &
    column1, lat0, lat1, lon0, lon1) result(status)

    implicit none

    character(*), intent(in)        :: filename
    character(*), intent(in)        :: timeslot
    integer,      intent(in)        :: satnum
    integer,      intent(out)       :: i_line, i_column
    integer,      intent(out)       :: n_lines, n_columns
    integer,      intent(in), value :: bounds
    integer,      intent(in), value :: line0, line1, &
                                       column0, column1
    real(8),      intent(in), value :: lat0, lat1, lon0, lon1

    status = seviri_get_dimens_hrit(trim(filename)//C_NULL_CHAR, &
        trim(timeslot)//C_NULL_CHAR, satnum, i_line, i_column, n_lines, &
        n_columns, bounds, line0, line1, column0, column1, lat0, lat1, &
        lon0, lon1)
    if (status .ne. 0) then
        write(6, *) 'ERROR: seviri_get_dimens_hrit()'
        return
    end if

end function seviri_get_dimens_hrit_f90


integer function seviri_get_dimens_f90(filename, i_line, i_column, &
    n_lines, n_columns, bounds, line0, line1, column0, column1, lat0, lat1, &
    lon0, lon1) result(status)

    implicit none

    character(*), intent(in)        :: filename
    integer,      intent(out)       :: i_line, i_column
    integer,      intent(out)       :: n_lines, n_columns
    integer,      intent(in), value :: bounds
    integer,      intent(in), value :: line0, line1, &
                                       column0, column1
    real(8),      intent(in), value :: lat0, lat1, lon0, lon1

    status = seviri_get_dimens(trim(filename)//C_NULL_CHAR, &
        i_line, i_column, n_lines, n_columns, bounds, line0, line1, &
        column0, column1, lat0, lat1, lon0, lon1)
    if (status .ne. 0) then
        write(6, *) 'ERROR: seviri_get_dimens()'
        return
    end if

end function seviri_get_dimens_f90


integer function seviri_read_and_preproc_nat_f90(filename, preproc_f90, n_bands, &
    band_ids, band_units, bounds, line0, line1, column0, column1, lat0, lat1, &
    lon0, lon1, do_calib_f90, satposstr, do_not_alloc_f90) result(status)

    implicit none

    character(*),               intent(in)        :: filename
    type(seviri_preproc_t_f90), intent(out)       :: preproc_f90
    integer,                    intent(in), value :: n_bands
    integer,                    intent(in)        :: band_ids(:)
    integer,                    intent(in)        :: band_units(:)
    integer,                    intent(in), value :: bounds
    integer,                    intent(in), value :: line0, line1, &
                                                     column0, column1
    real(8),                    intent(in), value :: lat0, lat1, lon0, lon1
    logical,                    intent(in), value :: do_calib_f90
    character(kind=c_char),     intent(inout)     :: satposstr(128)
    logical,                    intent(in), value :: do_not_alloc_f90

    integer                :: do_calib
    integer                :: do_not_alloc
    type(seviri_preproc_t) :: preproc
    integer                :: shape1(2)
    integer                :: shape2(3)

    do_calib = 0
    if (do_calib_f90) &
        do_calib = 1

    do_not_alloc = 0
    if (do_not_alloc_f90) &
        do_not_alloc = 1

    if (do_not_alloc_f90) then
        preproc%time  = c_loc(preproc_f90%time(1, 1))
        preproc%lat   = c_loc(preproc_f90%lat (1, 1))
        preproc%lon   = c_loc(preproc_f90%lon (1, 1))
        preproc%sza   = c_loc(preproc_f90%sza (1, 1))
        preproc%saa   = c_loc(preproc_f90%saa (1, 1))
        preproc%vza   = c_loc(preproc_f90%vza (1, 1))
        preproc%vaa   = c_loc(preproc_f90%vaa (1, 1))
        preproc%data2 = c_loc(preproc_f90%data(1, 1, 1))
    end if

    status = seviri_read_and_preproc_nat(trim(filename)//C_NULL_CHAR, preproc, &
        n_bands, band_ids, band_units, bounds, line0, line1, column0, column1, &
        lat0, lat1, lon0, lon1, do_calib, satposstr, do_not_alloc)
    if (status .ne. 0) then
        write(6, *) 'ERROR: seviri_read_and_preproc_nat()'
        return
    end if

    preproc_f90%memory_alloc_d = .false.
    if (preproc%memory_alloc_d .ne. 0) &
        preproc_f90%memory_alloc_d = .true.

    preproc_f90%n_bands    = preproc%n_bands
    preproc_f90%n_lines    = preproc%n_lines
    preproc_f90%n_columns  = preproc%n_columns
    preproc_f90%fill_value = preproc%fill_value

    shape1 = [preproc_f90%n_columns, preproc_f90%n_lines]

    if (.not. do_not_alloc_f90) then
        call c_f_pointer(preproc%time,  preproc_f90%time, shape1)
        call c_f_pointer(preproc%lat,   preproc_f90%lat,  shape1)
        call c_f_pointer(preproc%lon,   preproc_f90%lon,  shape1)
        call c_f_pointer(preproc%sza,   preproc_f90%sza,  shape1)
        call c_f_pointer(preproc%saa,   preproc_f90%saa,  shape1)
        call c_f_pointer(preproc%vza,   preproc_f90%vza,  shape1)
        call c_f_pointer(preproc%vaa,   preproc_f90%vaa,  shape1)

        shape2 = [preproc_f90%n_columns, preproc_f90%n_lines, n_bands]

        call c_f_pointer(preproc%data2, preproc_f90%data, shape2)
    end if

    preproc_f90%preproc = preproc

end function seviri_read_and_preproc_nat_f90


integer function seviri_read_and_preproc_hrit_f90(filename, timeslot, satnum, &
    preproc_f90, n_bands, band_ids, band_units, bounds, line0, line1, column0, &
    column1, lat0, lat1, lon0, lon1, rss_f90, do_calib_f90, satposstr, &
    do_not_alloc_f90) result(status)

    implicit none

    character(*),               intent(in)        :: filename
    character(*),               intent(in)        :: timeslot
    integer,                    intent(in)        :: satnum
    type(seviri_preproc_t_f90), intent(out)       :: preproc_f90
    integer,                    intent(in), value :: n_bands
    integer,                    intent(in)        :: band_ids(:)
    integer,                    intent(in)        :: band_units(:)
    integer,                    intent(in), value :: bounds
    integer,                    intent(in), value :: line0, line1, &
                                                     column0, column1
    real(8),                    intent(in), value :: lat0, lat1, lon0, lon1
    logical,                    intent(in), value :: rss_f90
    logical,                    intent(in), value :: do_calib_f90
    character(kind=c_char),     intent(inout)     :: satposstr(128)
    logical,                    intent(in), value :: do_not_alloc_f90

    integer                :: rss
    integer                :: do_calib
    integer                :: do_not_alloc
    type(seviri_preproc_t) :: preproc
    integer                :: shape1(2)
    integer                :: shape2(3)

    rss = 0
    if (rss_f90) &
        rss = 1

    do_calib = 0
    if (do_calib_f90) &
        do_calib = 1

    do_not_alloc = 0
    if (do_not_alloc_f90) &
        do_not_alloc = 1

    if (do_not_alloc_f90) then
        preproc%time  = c_loc(preproc_f90%time(1, 1))
        preproc%lat   = c_loc(preproc_f90%lat (1, 1))
        preproc%lon   = c_loc(preproc_f90%lon (1, 1))
        preproc%sza   = c_loc(preproc_f90%sza (1, 1))
        preproc%saa   = c_loc(preproc_f90%saa (1, 1))
        preproc%vza   = c_loc(preproc_f90%vza (1, 1))
        preproc%vaa   = c_loc(preproc_f90%vaa (1, 1))
        preproc%data2 = c_loc(preproc_f90%data(1, 1, 1))
    end if

    status = seviri_read_and_preproc_hrit(trim(filename)//C_NULL_CHAR, &
        trim(timeslot)//C_NULL_CHAR, satnum, preproc, n_bands, band_ids, &
        band_units, bounds, line0, line1, column0, column1, lat0, lat1, &
        lon0, lon1, rss, do_calib, satposstr, do_not_alloc)
    if (status .ne. 0) then
        write(6, *) 'ERROR: seviri_read_and_preproc_hrit()'
        return
    end if

    preproc_f90%memory_alloc_d = .false.
    if (preproc%memory_alloc_d .ne. 0) &
        preproc_f90%memory_alloc_d = .true.

    preproc_f90%n_bands    = preproc%n_bands
    preproc_f90%n_lines    = preproc%n_lines
    preproc_f90%n_columns  = preproc%n_columns
    preproc_f90%fill_value = preproc%fill_value

    shape1 = [preproc_f90%n_columns, preproc_f90%n_lines]

    if (.not. do_not_alloc_f90) then
        call c_f_pointer(preproc%time,  preproc_f90%time, shape1)
        call c_f_pointer(preproc%lat,   preproc_f90%lat,  shape1)
        call c_f_pointer(preproc%lon,   preproc_f90%lon,  shape1)
        call c_f_pointer(preproc%sza,   preproc_f90%sza,  shape1)
        call c_f_pointer(preproc%saa,   preproc_f90%saa,  shape1)
        call c_f_pointer(preproc%vza,   preproc_f90%vza,  shape1)
        call c_f_pointer(preproc%vaa,   preproc_f90%vaa,  shape1)

        shape2 = [preproc_f90%n_columns, preproc_f90%n_lines, n_bands]

        call c_f_pointer(preproc%data2, preproc_f90%data, shape2)
    end if

    preproc_f90%preproc = preproc

end function seviri_read_and_preproc_hrit_f90


integer function seviri_read_and_preproc_f90(filename, preproc_f90, n_bands, &
    band_ids, band_units, bounds, line0, line1, column0, column1, lat0, lat1, &
    lon0, lon1, do_calib_f90, satposstr, do_not_alloc_f90) result(status)

    implicit none

    character(*),               intent(in)        :: filename
    type(seviri_preproc_t_f90), intent(out)       :: preproc_f90
    integer,                    intent(in), value :: n_bands
    integer,                    intent(in)        :: band_ids(:)
    integer,                    intent(in)        :: band_units(:)
    integer,                    intent(in), value :: bounds
    integer,                    intent(in), value :: line0, line1, &
                                                     column0, column1
    real(8),                    intent(in), value :: lat0, lat1, lon0, lon1
    logical,                    intent(in), value :: do_calib_f90
    character(kind=c_char),     intent(inout)     :: satposstr(128)
    logical,                    intent(in), value :: do_not_alloc_f90

    integer                :: do_calib
    integer                :: do_not_alloc
    type(seviri_preproc_t) :: preproc
    integer                :: shape1(2)
    integer                :: shape2(3)

    do_calib = 0
    if (do_calib_f90) &
        do_calib = 1

    do_not_alloc = 0
    if (do_not_alloc_f90) &
        do_not_alloc = 1

    if (do_not_alloc_f90) then
        preproc%time  = c_loc(preproc_f90%time(1, 1))
        preproc%lat   = c_loc(preproc_f90%lat (1, 1))
        preproc%lon   = c_loc(preproc_f90%lon (1, 1))
        preproc%sza   = c_loc(preproc_f90%sza (1, 1))
        preproc%saa   = c_loc(preproc_f90%saa (1, 1))
        preproc%vza   = c_loc(preproc_f90%vza (1, 1))
        preproc%vaa   = c_loc(preproc_f90%vaa (1, 1))
        preproc%data2 = c_loc(preproc_f90%data(1, 1, 1))
    end if

    status = seviri_read_and_preproc(trim(filename)//C_NULL_CHAR, preproc, &
        n_bands, band_ids, band_units, bounds, line0, line1, column0, column1, &
        lat0, lat1, lon0, lon1, do_calib, satposstr, do_not_alloc)
    if (status .ne. 0) then
        write(6, *) 'ERROR: seviri_read_and_preproc()'
        return
    end if

    preproc_f90%memory_alloc_d = .false.
    if (preproc%memory_alloc_d .ne. 0) &
        preproc_f90%memory_alloc_d = .true.

    preproc_f90%n_bands    = preproc%n_bands
    preproc_f90%n_lines    = preproc%n_lines
    preproc_f90%n_columns  = preproc%n_columns
    preproc_f90%fill_value = preproc%fill_value

    shape1 = [preproc_f90%n_columns, preproc_f90%n_lines]

    if (.not. do_not_alloc_f90) then
        call c_f_pointer(preproc%time,  preproc_f90%time, shape1)
        call c_f_pointer(preproc%lat,   preproc_f90%lat,  shape1)
        call c_f_pointer(preproc%lon,   preproc_f90%lon,  shape1)
        call c_f_pointer(preproc%sza,   preproc_f90%sza,  shape1)
        call c_f_pointer(preproc%saa,   preproc_f90%saa,  shape1)
        call c_f_pointer(preproc%vza,   preproc_f90%vza,  shape1)
        call c_f_pointer(preproc%vaa,   preproc_f90%vaa,  shape1)

        shape2 = [preproc_f90%n_columns, preproc_f90%n_lines, n_bands]

        call c_f_pointer(preproc%data2, preproc_f90%data, shape2)
    end if

    preproc_f90%preproc = preproc

end function seviri_read_and_preproc_f90


integer function seviri_preproc_free_f90(preproc_f90) result(status)

    implicit none

    type(seviri_preproc_t_f90), intent(inout) :: preproc_f90

    status = seviri_preproc_free(preproc_f90%preproc)

end function seviri_preproc_free_f90

end module seviri_util
