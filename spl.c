/*
  Samsung SPL Driver Test
  Written by: M.EMMANUEL
  10/JUL/2023
  Based on information found on SPLIX CUPS drivers
*/

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 2480
#define HEIGHT 3508
#define DPI 300

void print_jcl_header(void) {
    printf("%c%%-12345X", 0x1B);
}

void print_jcl_end(void) {
    printf("%c%c%%-12345X", 0x09, 0x1B);
}

void print_pjl_header(void) {
    printf("@PJL DEFAULT SERVICEDATE=20060524\n");
    printf("@PJL SET PAPERTYPE = BOND\n");
    printf("@PJL SET DENSITY = 3\n");
    printf("@PJL SET ECONOMODE = ON\n");
    printf("@PJL SET POWERSAVE = ON\n");
    printf("@PJL SET POWERSAVETIME = 5\n");
    printf("@PJL SET RET = OFF\n");
    printf("@PJL SET JAMRECOVERY = OFF\n");
    printf("@PJL SET REPRINT = ON\n");
    printf("@PJL SET ALTITUDE = LOW\n");
    printf("@PJL ENTER LANGUAGE = QPDL\n");
}

void write_band_header(int band_number, int band_width, int band_height, int data_length) {

    unsigned char band_header[11];

    band_header[0] = 0x0C;
    band_header[1] = band_number;
    band_header[2] = (band_width >> 8) & 0xFF;
    band_header[3] = band_width & 0xFF;
    band_header[4] = (band_height >> 8) & 0xFF;
    band_header[5] = band_height & 0xFF;
    band_header[6] = 0x11;
    band_header[7] = (data_length >> 24) & 0xFF;
    band_header[8] = (data_length >> 16) & 0xFF;
    band_header[9] = (data_length >> 8) & 0xFF;
    band_header[10] = data_length & 0xFF;

    fwrite(band_header, sizeof(unsigned char), 11, stdout);
}

void write_uncompressed(const unsigned char *data, int len) {
    while (len > 0) {
        int chunk_len = len > 64 ? 64 : len;

        // Write the header. Remember that the length is stored as (length - 1).
        putchar(chunk_len - 1);

        // Write the chunk data.
        fwrite(data, 1, chunk_len, stdout);

        data += chunk_len;
        len -= chunk_len;
    }
}

unsigned char* print_big_x(void) {

    unsigned char* image = (unsigned char*)malloc(WIDTH * HEIGHT);
    if (image == NULL) {
        fprintf(stderr, "Failed to allocate memory for image\n");
        exit(1);
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            image[i * WIDTH + j] = ((j / 128 == i / 128) || (j / 128 == (HEIGHT - i) / 128)) ? 0xFF : 0x00;
        }
    }

    return image;
}

int main(void) {
    
    int band_width = 2480; // width in pixels
    int band_height = 128; // 128 lines per band

    // Determine number of bands
    int num_bands = 3508 / band_height; // height in pixels / band height

    unsigned char* image = print_big_x();
    
    print_jcl_header();
    print_pjl_header();

    for (int i = 0; i < num_bands; ++i) {
        // Calculate the length of data for this band
        int data_length = band_width * band_height;
        if (i == num_bands - 1) {
            // Last band might have less than band_height lines
            data_length = band_width * (3508 % band_height);
        }

        // Write band header
        write_band_header(i, band_width, band_height, data_length);

        // Write band data
        write_uncompressed(image + i*band_width*band_height, data_length);
    }

    print_jcl_end();
    return 0;
}
