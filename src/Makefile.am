CFLAGS=-Wall
TESTS=check_dif
bin_PROGRAMS=dc2uddf
check_PROGRAMS=check_dif

dc2uddf_CFLAGS=$(XML_CFLAGS) $(DIVECOMPUTER_CFLAGS) $(GLIB_CFLAGS) -g
dc2uddf_LDADD=$(XML_LIBS) $(DIVECOMPUTER_LIBS) $(GLIB_LIBS)
dc2uddf_SOURCES=dc2uddf.c utils.c dif/dif.c dif/uddf.c dif/algos.c

check_dif_SOURCES=dif/dif.c dif/uddf.c dif/algos.c tests/check_dif.c
check_dif_CFLAGS=$(CHECK_CFLAGS) $(GLIB_CFLAGS) $(XML_CFLAGS)
check_dif_LDADD=$(XML_LIBS) $(GLIB_LIBS) $(CHECK_LIBS)
