#
#
#
#
AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I$(top_srcdir)/include
if CUSTOM_JPEG_LIB
AM_CPPFLAGS +=	-I$(libjpeg_inc_path)
endif

bin_PROGRAMS = shjpegtest v2mjpeg libjpegtest shjpeg_multithread
if MAKE_DIRECTFB_TEST
bin_PROGRAMS += shjpegshow

shjpegshow_SOURCES = shjpegshow.c
shjpegshow_CFLAGS = $(DIRECTFB_CFLAGS)
shjpegshow_LDADD = ../src/libshjpeg.la $(DIRECTFB_LIBS) $(UIOMUX_LIBS) $(SHVEU_LIBS)
endif

shjpegtest_SOURCES = shjpegtest.c
shjpegtest_LDADD = ../src/libshjpeg.la $(UIOMUX_LIBS) $(SHVEU_LIBS)

v2mjpeg_SOURCES = v2mjpeg.c
v2mjpeg_LDADD = ../src/libshjpeg.la $(UIOMUX_LIBS) $(SHVEU_LIBS)

libjpegtest_SOURCES = libjpeg_test.c
libjpegtest_LDADD = ../src/libshjpeg.la $(UIOMUX_LIBS) $(SHVEU_LIBS)

shjpeg_multithread_SOURCES = shjpeg_multithread.c
shjpeg_multithread_LDADD = ../src/libshjpeg.la $(UIOMUX_LIBS) $(SHVEU_LIBS)
shjpeg_multithread_CFLAGS = $(AM_CFLAGS) $(UIOMUX_CFLAGS)
