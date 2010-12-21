EXTRA_DIST = jpeglibheads.c

if MAKE_JPEG_WRAPPER
if CUSTOM_JPEG_LIB
INCPATH = -I$(libjpeg_inc_path)
endif
.PHONY : all clean
all: temp/jpegapi.in

jpegapi.h: jpeglibheads.c
	$(CC) -E $(INCPATH)/. jpeglibheads.c > jpegapi.h

temp/jpegapi.in: jpegapi.h
	./genapi

clean:
	rm temp/*
	rm jpegapi.h
endif
