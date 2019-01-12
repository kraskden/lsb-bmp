#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "bmp.h"

#define BUFF_SIZE 512

typedef enum {
    err_nop, err_data_file, err_cont_file, err_filetype, err_big_size
} stego_err;

bool is_encrypt(int argc, char *argv[])
{
    return (argc == 4) && !strcmp(argv[1], "-e");
}

bool is_decrypt(int argc, char *argv[])
{
    return (argc == 3) && !strcmp(argv[1], "-d");
}

uint32_t get_file_size(FILE* f)
{
    long pos = ftell(f);
    fseek(f, 0, SEEK_END);
    long res =  ftell(f);
    fseek(f, pos, SEEK_SET);
    return res;
}

bool get_bmp_header(FILE* f, uint32_t* bmp_size, uint32_t* bmp_offset)
{
    fseek(f, 0, SEEK_SET);
    BmpFileInfo res;
    if (fread(&res, sizeof (res), 1, f)) {
        *bmp_size = res.size;
        *bmp_offset = res.offBits;
        return true;
    } else {
        return false;
    }
}

void delim_num(uint8_t num, uint8_t bit_count, uint8_t* bit_arr)
{
    uint8_t mask = (1u << bit_count) - 1u;
    for (uint8_t i = 0; i < 8/bit_count; ++i) {
        bit_arr[i] = (num & mask) >> i*bit_count;
        mask <<= bit_count;
    }
}

uint8_t form_num(uint8_t bit_count, uint8_t *bit_arr)
{
    uint8_t res = 0;
    for (int8_t i = 8/bit_count - 1; i >= 0; --i) {
        res <<= bit_count;
        res |= bit_arr[i];
    }
    return res;
}

void encrypt(char *in_path, char *data_path, char *out_path)
{
    FILE *in, *data, *out;
    uint32_t bmp_size, bmp_offset;
    if (!(in = fopen(in_path, "rb")) || !(data = fopen(data_path, "rb")) || !(out = fopen(out_path, "w+"))) {
        fprintf(stderr, "File open error\n");
        return;
    }
    if (!get_bmp_header(in, &bmp_size, &bmp_offset)) {
        fprintf(stderr, "Invalid file format. Use BMP only files");
        goto fail;
    }
    uint32_t file_size = get_file_size(data);
    // modification!!
    if (file_size > bmp_size / 8) {
        fprintf(stderr, "To many data... Max size: %d bytes", bmp_size / 8);
        goto fail;
    }
    // Write headers.
    fseek(in, 0, SEEK_SET);
    uint8_t *headers_buff = (uint8_t*)malloc(bmp_offset * sizeof (*headers_buff));
    fread(headers_buff, sizeof (*headers_buff), bmp_offset, in);
    fwrite(headers_buff, sizeof (*headers_buff), bmp_offset, out);
    free(headers_buff);
    // Write file size
    uint8_t size_parts[3];
    size_parts[0] = (file_size & 0xFF);
    size_parts[1]= (file_size & 0xFF00) >> 2;
    size_parts[2] = (file_size & 0xFF0000) >> 4;
    uint8_t bits_arr[8];
    uint8_t buff[BUFF_SIZE];
    fread(buff, 1, 24, in);
    for (int i = 0; i < 24; ++i) {
        if (i % 8 == 0) {
            delim_num(size_parts[i/8], 1, bits_arr);
        }
        buff[i] = (buff[i] & 0b11111110) | bits_arr[i % 8];
    }
    fwrite(buff, 1, 24, out);
    // Write file data
    uint64_t container_read, data_read = BUFF_SIZE;
    uint8_t data_buff[BUFF_SIZE];
    uint64_t data_index = BUFF_SIZE;
    int bit_index = 0;
    while ((container_read = fread(buff, 1, BUFF_SIZE, in))) {
        for (uint64_t i = 0; i < container_read; ++i) {
            if (data_index == data_read) {
                data_read = fread(data_buff, 1, BUFF_SIZE, data);
                // Let set first bit arrr
                if (data_read) {
                    delim_num(*data_buff, 1, bits_arr);
                }
                data_index = 0;
            }
            if (data_read) {
                buff[i] = (buff[i] & 0b11111110) | bits_arr[bit_index++];
            }
            if (bit_index == 8) {
                bit_index = 0;
                if (++data_index != data_read) {
                    delim_num(data_buff[data_index], 1, bits_arr);
                }
            }
        }
        fwrite(buff, 1, container_read, out);
    }
fail:
    fclose(in);
    fclose(out);
    fclose(data);
}

void decrypt(char *in_path, char *out_path)
{
}

int main(int argc, char *argv[])
{
    encrypt("test.bmp", "data.bin", "out.bmp");
    /*
    if (is_encrypt(argc, argv)) {
        encrypt(argv[2], argv[3], "out.bmp");
    } else if (is_decrypt(argc, argv)) {
        decrypt(argv[2], "out.bin");
    } else {
        fprintf(stderr, "Undefined par's\n");
        return 1;
    }
   */
}
