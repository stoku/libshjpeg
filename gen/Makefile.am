
if MAKE_JPEG_WRAPPER
.PHONY : all clean
all: temp/jpeglib.in

jpegapi.h: jpeglibheads.c
	$(CC) -E -I$(libjpeg_inc_path)/. jpeglibheads.c > jpegapi.h

temp/jpeglib.in: jpegapi.h
	./genapi

clean:
	rm temp/*
	rm jpegapi.h
endif
