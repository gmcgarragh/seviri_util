SEVIRI_util.o: SEVIRI_util.c SEVIRI_util.h seviri_native_util.h preproc.h \
 read_write.h
SEVIRI_util_funcs.o: SEVIRI_util_funcs.c SEVIRI_util.h \
 seviri_native_util.h preproc.h read_write.h
SEVIRI_util_prog.o: SEVIRI_util_prog.c SEVIRI_util.h seviri_native_util.h \
 preproc.h read_write.h
example_c_hrit.o: example_c_hrit.c seviri_native_util.h preproc.h \
 read_write.h
example_c_nat.o: example_c_nat.c seviri_native_util.h preproc.h \
 read_write.h
hrit_anc_funcs.o: hrit_anc_funcs.c internal.h seviri_native_util.h \
 preproc.h read_write.h misc_util.h nav_util.h hrit_funcs.h \
 hrit_anc_funcs.h
hrit_funcs.o: hrit_funcs.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h hrit_funcs.h hrit_anc_funcs.h
internal.o: internal.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
misc_util.o: misc_util.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
nav_util.o: nav_util.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
preproc.o: preproc.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h hrit_funcs.h
read_write.o: read_write.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
