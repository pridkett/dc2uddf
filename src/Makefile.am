CFLAGS=-Wall
bin_PROGRAMS=dc2uddf
dc2uddf_CFLAGS=$(XML_CFLAGS) $(DIVECOMPUTER_CFLAGS) $(GLIB_CFLAGS)
dc2uddf_LDADD=$(XML_LIBS) $(DIVECOMPUTER_LIBS) $(GLIB_LIBS)
dc2uddf_SOURCES=dc2uddf.c utils.c dif/dif.c dif/uddf.c
