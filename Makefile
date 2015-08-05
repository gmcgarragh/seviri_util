#*******************************************************************************
#
# Copyright (C) 2014-2015 Greg McGarragh <mcgarragh@atm.ox.ac.uk>
#
# This source code is licensed under the GNU General Public License (GPL),
# Version 3.  See the file COPYING for more details.
#
#*******************************************************************************
.SUFFIXES: .c .f90

OBJECTS = internal.o \
          misc_util.o \
          nav_util.o \
          preproc.o \
          read_write.o \
	  hrit_funcs.o \
	  hrit_anc_funcs.o

include make.inc

all: libseviri_native_util.a \
     example_c_hrit \
     example_c_nat \
     $(OPTIONAL_PROGRAMS)

libseviri_native_util.a: $(OBJECTS)
	ar -rs libseviri_native_util.a $(OBJECTS)

seviri_native_util.o: seviri_native_util.f90

SEVIRI_util: SEVIRI_util.o SEVIRI_util_funcs.o SEVIRI_util_prog.o libseviri_native_util.a
	$(CC) $(CCFLAGS) -o SEVIRI_util SEVIRI_util_funcs.o SEVIRI_util_prog.o SEVIRI_util.o \
        libseviri_native_util.a $(INCDIRS) $(LIBDIRS) $(LINKS)

example_c_nat: example_c_nat.c libseviri_native_util.a
	$(CC) $(CCFLAGS) -o example_c_nat example_c_nat.c libseviri_native_util.a -lm

example_c_hrit: example_c_hrit.c libseviri_native_util.a
	$(CC) $(CCFLAGS) -o example_c_hrit example_c_hrit.c libseviri_native_util.a -lm

example_f90_hrit: example_f90_hrit.f90 libseviri_native_util.a
	$(F90) $(F90FLAGS) -o example_f90_hrit example_f90_hrit.f90 libseviri_native_util.a -lm

example_f90_nat: example_f90_nat.f90 libseviri_native_util.a
	$(F90) $(F90FLAGS) -o example_f90_nat example_f90_nat.f90 libseviri_native_util.a -lm

README: readme_source.txt
	fold --spaces --width=80 readme_source.txt > README
	sed -i 's/[ \t]*$$//' README

clean:
	rm -f *.a *.o *.mod example_c_hrit example_c_nat example_f90_hrit example_f90_nat $(OPTIONAL_PROGRAMS)

.c.o:
	$(CC) $(CCFLAGS) $(INCDIRS) -c -o $*.o $<

%.o : %.mod

.f90.o:
	$(F90) $(F90FLAGS) $(INCDIRS) -c -o $*.o $<

include dep.inc
