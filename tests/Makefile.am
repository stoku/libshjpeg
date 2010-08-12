#
#
#
#
AM_CFLAGS = -Wall -Werror
AM_CPPFLAGS = -I$(top_srcdir)/include
if MAKE_JPEG_WRAPPER
AM_CPPFLAGS +=	-I$(LIBJPEG_INC_PATH)
endif

bin_PROGRAMS = shjpegtest v2mjpeg shjpegshow libjpegtest

shjpegtest_SOURCES = shjpegtest.c
shjpegtest_LDADD = ../src/libshjpeg.la

v2mjpeg_SOURCES = v2mjpeg.c
v2mjpeg_LDADD = ../src/libshjpeg.la

shjpegshow_SOURCES = shjpegshow.c
shjpegshow_CFLAGS = $(DIRECTFB_CFLAGS)
shjpegshow_LDADD = ../src/libshjpeg.la $(DIRECTFB_LIBS)

libjpegtest_SOURCES = libjpeg_test.c
libjpegtest_LDADD = ../src/libshjpeg.la
