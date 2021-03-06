/*
 * Copyright 2010 IGEL Co.,Ltd.
 * Copyright 2008,2009 Renesas Solutions Co.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>

unsigned int height, width;
J_COLOR_SPACE mode = JCS_RGB;

#if JPEG_LIB_VERSION == 62
void my_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  if (num_bytes > 0) {
    while (num_bytes > (long) cinfo->src->bytes_in_buffer) {
      num_bytes -= (long) cinfo->src->bytes_in_buffer;
      (void) cinfo->src->fill_input_buffer(cinfo);
      /* note we assume that fill_input_buffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    cinfo->src->next_input_byte += (size_t) num_bytes;
    cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
  }
}
#endif

int
decompress_jpeg_file(char *input, unsigned char **buffer, int verbose, int stdio_src) {
	FILE *infile = fopen(input, "rb");
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW scan_line[1];
#if JPEG_LIB_VERSION >= 80
	unsigned char *membuf = NULL;
#endif

	if (!infile) {
		fprintf(stderr, "Cannot open input file %s\n", input);
		return -1;
	}

	cinfo.err = jpeg_std_error(&jerr);
	if (verbose)
		jerr.trace_level = 1;

	jpeg_create_decompress(&cinfo);
	if (stdio_src) {
		jpeg_stdio_src(&cinfo, infile);
	}
#if JPEG_LIB_VERSION >= 80
	else {
		struct stat st;
		stat(input, &st);
		membuf = malloc(st.st_size);
		if (st.st_size != fread(membuf, 1, st.st_size, infile) ) {
			fprintf(stderr, "Cannot read entire input file %s", input);
			return -1;
		}
		fclose(infile);
		jpeg_mem_src(&cinfo, membuf, st.st_size);
	}
#endif

/*Workaround for libjpeg62 to be able to process EXIF data through
  libshjpeg*/

#if JPEG_LIB_VERSION == 62
	cinfo.src->skip_input_data = my_skip_input_data;
#endif

	jpeg_read_header(&cinfo, TRUE);
	jpeg_calc_output_dimensions(&cinfo);

	height = cinfo.output_height;
	width = cinfo.output_width;
        cinfo.out_color_space = mode;

	jpeg_start_decompress(&cinfo);

	*buffer = malloc (cinfo.output_height * cinfo.output_width *
		cinfo.num_components);
	while (cinfo.output_scanline < cinfo.image_height) {
		scan_line[0] = &((*buffer)[cinfo.output_width * cinfo.output_scanline *
			cinfo.out_color_components]);
		jpeg_read_scanlines(&cinfo, scan_line, 1);
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	if (stdio_src)
		fclose(infile);
#if JPEG_LIB_VERSION >= 80
	else
		free(membuf);
#endif
	return 0;
}
void
dump_ppm(char *file, unsigned char *buffer) {
	FILE *dumpfile = fopen (file, "wb");
	if (!dumpfile) {
		fprintf(stderr, "Cannot open dump file %s\n", file);
		return;
	}
	if (mode == JCS_RGB) {
		fprintf(dumpfile, "P6 %d %d %d\n", width, height, 255);
	}
	fwrite(buffer, 1, width*height*3, dumpfile);
	fclose(dumpfile);
}

int
compress_jpeg_file(char *output, unsigned char *buffer, int verbose, int stdio_src) {
	FILE *outfile;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW scan_line[1];
#if JPEG_LIB_VERSION >= 80
	unsigned long buflen = 0;
	unsigned char *membuf = NULL;
#endif
	outfile = fopen (output, "wb");
	if (!outfile) {
		fprintf(stderr, "Cannot open output file %s\n", output);
		return -1;
	}

	cinfo.err = jpeg_std_error(&jerr);
	if (verbose)
		jerr.trace_level = 1;
	jpeg_create_compress(&cinfo);
	if (stdio_src) {
		jpeg_stdio_dest(&cinfo, outfile);
	}
#if JPEG_LIB_VERSION >= 80
	else {
		jpeg_mem_dest(&cinfo, &membuf, &buflen);
	}
#endif

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = mode;

	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	while(cinfo.next_scanline < cinfo.image_height) {
		scan_line[0] = &(buffer[width * cinfo.next_scanline *
			cinfo.input_components]);
		jpeg_write_scanlines(&cinfo, scan_line, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
#if JPEG_LIB_VERSION >= 80
	if (!stdio_src) {
		fwrite(membuf, 1, buflen, outfile);
	}
#endif
	fclose(outfile);
	return 0;
}
void
print_usage(char *argv0) {
    fprintf(stderr,
	    "Usage: %s [OPTION] <jpegfile> [<output>]\n", argv0);
    fprintf(stderr,
	    "- Decode given JPEG file, and then re-encode.\n"
	    "- Default re-encoed JPEG filename is with '.out' as a suffix.\n"
	    "\n"
	    "Options:\n"
	    "  -h, --help                  this message.\n"
	    "  -v, --verbose               libjpeg verbose output.\n"
	    "  -q, --quiet	           no messages from this program.\n"
	    "  -d[<ppm>], --dump[=<ppm>]   dump decoded image in PPM/YCbCr (default: test.[ppm/ycbcr]).\n"
	    "  -r<times>, --repeat=<times> reconvert the data n times to look for cumulative effects\n"
	    "  -t<file>, --tmpfile=<file>  temporary file to use for reconversions\n"
	    "  -y, --yuv                   decode to JCS_YCbCr format\n");
#if JPEG_LIB_VERSION >= 80
    fprintf(stderr,
	    "  -m, --mem		   use jpeg_mem_* as data source and destination\n"
	    "  -s, --stdio                 use jpeg_stdio_* as data src/dest (default)\n");
#endif
}

int
main(int argc, char *argv[])
{
    char		  *input, *output;
    char		  *dumpfn = NULL;
    char		  filename[200];
    int			   verbose = 0;
    int			   dump = 0;
    int			   quiet = 0;
    int			   repeat = 0;
    unsigned char	   *buffer = NULL;
    char		   *argv0 = NULL;
    char		   *pingpong = "pingpong.jpg";
    int		   repcount;
    int		   stdio_src = 1;

    argv0 = argv[0];

    /* parse arguments */
    while (1) {
	int c, option_index = 0;
	static struct option long_options[] = {
	    {"help", 0, 0, 'h'},
	    {"verbose", 0, 0, 'v'},
	    {"quiet", 0, 0, 'q'},
	    {"dump", 2, 0, 'd'},
	    {"repeat", 1, 0, 'r'},
	    {"mem", 0, 0, 'm'},
	    {"stdio", 0, 0, 's'},
	    {"tmpfile", 1, 0, 't'},
	    {"yuv", 0, 0, 'y'},
	    {0, 0, 0, 0}
	};

	if ((c = getopt_long(argc, argv, "hvd::qr:msty",
			     long_options, &option_index)) == -1)
	    break;

	switch(c) {
	case 'h':	// help
	    print_usage(argv0);
	    return 0;

	case 'v':
	    verbose = 1;
	    break;

	case 'd':
	    dump = 1;
	    if (optarg)
		dumpfn = optarg;
	    break;

	case 'q':
	    quiet = 1;
	    break;

	case 't':
	    pingpong = optarg;
	    break;

	case 'r':
	    repeat = atoi(optarg);
	    break;

	case 's':
	    stdio_src = 1;
	    break;

#if JPEG_LIB_VERSION >= 80
	case 'm':
	    stdio_src = 0;
	    break;
#endif

	case 'y':
	    mode = JCS_YCbCr;
	    break;

	default:
	    fprintf(stderr, "unknown option 0%x.\n", c);
	    print_usage(argv0);
	    return 1;
	}
    }


    if (optind >= argc) {
	print_usage(argv0);
	return 0;
    }
    /* check option */
    input = argv[optind++];
    if (argv[optind])
	    output = argv[optind];
    else {
	    snprintf(filename, sizeof(filename), "%s.out", input);
	    output = filename;
    }
    if (!quiet) {
	printf("Input file = %s\n", input);
	printf("Output file = %s\n", output);
	printf("Temp file = %s\n", pingpong);
	printf("Mode = %s\n", stdio_src ? "STDIO" : "memory");
    }

    if (decompress_jpeg_file(input, &buffer, verbose, stdio_src) < 0)  {
	fprintf(stderr, "Error on file decode\n");
	return -1;
    }

    if (dump) {
	if (!dumpfn)  {
		dumpfn = malloc (11);
		snprintf(dumpfn, 11, "test.%s", mode == JCS_RGB ? "ppm" : "ycbcr");
		dump_ppm(dumpfn, buffer);
		free (dumpfn);
	}
	dump_ppm(dumpfn, buffer);
    }

    for (repcount=0; repcount < repeat; repcount++) {

	if (compress_jpeg_file(pingpong, buffer, 0, stdio_src) < 0)  {
		fprintf(stderr, "Error on file encode\n");
		return -1;
	}
	free (buffer);
	if (decompress_jpeg_file(pingpong, &buffer, 0, stdio_src) < 0)  {
		fprintf(stderr, "Error on file decode\n");
		return -1;
	}
    }

    if (compress_jpeg_file(output, buffer, verbose, stdio_src) < 0)  {
	fprintf(stderr, "Error on file encode\n");
	return -1;
    }
    free (buffer);

    return 0;
}
