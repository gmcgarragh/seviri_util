example_c.o: example_c.c seviri_native_util.h preproc.h read_write.h
internal.o: internal.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
misc_util.o: misc_util.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
nav_util.o: nav_util.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
preproc.o: preproc.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
read_write.o: read_write.c internal.h seviri_native_util.h preproc.h \
 read_write.h misc_util.h nav_util.h
