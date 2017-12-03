SEVIRI_tool.o: SEVIRI_tool.c SEVIRI_tool.h seviri_util.h external.h \
 preproc.h read_write.h read_write_hrit.h read_write_nat.h
SEVIRI_tool_funcs.o: SEVIRI_tool_funcs.c SEVIRI_tool.h seviri_util.h \
 external.h preproc.h read_write.h read_write_hrit.h read_write_nat.h
SEVIRI_tool_prog.o: SEVIRI_tool_prog.c SEVIRI_tool.h seviri_util.h \
 external.h preproc.h read_write.h read_write_hrit.h read_write_nat.h
example_c.o: example_c.c seviri_util.h external.h preproc.h read_write.h \
 read_write_hrit.h read_write_nat.h
hrit_anc_funcs.o: hrit_anc_funcs.c external.h hrit_anc_funcs.h \
 read_write.h internal.h misc_util.h nav_util.h
internal.o: internal.c external.h internal.h misc_util.h nav_util.h \
 read_write.h
misc_util.o: misc_util.c external.h internal.h misc_util.h nav_util.h \
 read_write.h
nav_util.o: nav_util.c external.h internal.h misc_util.h nav_util.h \
 read_write.h
preproc.o: preproc.c external.h hrit_anc_funcs.h read_write.h internal.h \
 misc_util.h nav_util.h preproc.h read_write_hrit.h read_write_nat.h
read_write.o: read_write.c external.h internal.h misc_util.h nav_util.h \
 read_write.h
read_write_hrit.o: read_write_hrit.c external.h hrit_anc_funcs.h \
 read_write.h internal.h misc_util.h nav_util.h read_write_hrit.h
read_write_nat.o: read_write_nat.c external.h hrit_anc_funcs.h \
 read_write.h internal.h misc_util.h nav_util.h read_write_nat.h
seviri_util_dlm.o: seviri_util_dlm.c seviri_util.h external.h preproc.h \
 read_write.h read_write_hrit.h read_write_nat.h seviri_util_dlm.h
seviri_util_py.o: seviri_util_py.c seviri_util.h external.h preproc.h \
 read_write.h read_write_hrit.h read_write_nat.h
