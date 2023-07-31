/*
MIT License

Copyright (C) 2023 MingHui

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "BMPRGBA.h"

//--------------------------------------------------------------------
// Get the information of the BMP image
//--------------------------------------------------------------------
void BMPRGBA_BmpGetInf(BMPRGBA_BmpInf *bmpinf,FILE *bmpfile)
{
    char bmp_head_data[54]; //Data of the BMP file head
    fpos_t bmp_head_pos = 0; //Position of the BMP file head
    fsetpos(bmpfile,&bmp_head_pos);
    fread(bmp_head_data,54,1,bmpfile);
    bmpinf->BmpWidth = (bmp_head_data[21] << 24 & 0xff000000) + (bmp_head_data[20] << 16 & 0xff0000) + (bmp_head_data[19] << 8 & 0xff00) + (bmp_head_data[18] & 0xff);
    bmpinf->BmpHeight = (bmp_head_data[25] << 24 & 0xff000000) + (bmp_head_data[24] << 16 & 0xff0000) + (bmp_head_data[23] << 8 & 0xff00) + (bmp_head_data[22] & 0xff);
    bmpinf->BmpBit = bmp_head_data[28] & 0xff;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Read the data of the bmp image,and convert to RGBA
//--------------------------------------------------------------------
void BMPRGBA_BmpToRGBA(char *dest,BMPRGBA_BmpInf *bmpinf,FILE *bmpfile)
{
    int bmp_w = bmpinf->BmpWidth; //Width of the BMP image
    int bmp_h = bmpinf->BmpHeight; //Height of the BMP image
    int bmp_b = bmpinf->BmpBit; //Bit of the BMP image
    fpos_t bmp_file_pos; //Position of the BMP file
    int bmp_line_end = ((bmp_w * bmp_b + 31) / 32 * 4) - bmp_w * (bmp_b / 8); //Size of the BMP line end
    size_t bmp_size = (bmp_w * bmp_h * (bmp_b / 8)) + bmp_h * bmp_line_end; //Size of the BMP pixel data
    ind_t bmp_pxi = 0; //Index of the BMP pixel
    int bmp_pxx = 0; //Position x of the BMP pixel
    ind_t dest_i = 0; //Index of the Dest array
    char col_c1; //Color char 1
    char col_c2; //Color char 2
    char col_c3; //Color char 3
    int col_16; //16 bit color value
    char *bmpfile_cache; //BMP data cache
    int bmp_pals; //Size of the BMP palette
    char int_char[4]; //Char of int
    char *bmp_pal_data; //BMP palette data
    int bmp_pali; //Index of the BMP palette
    if(bmp_b == 8)
    {
        bmp_file_pos = 10;
        fsetpos(bmpfile,&bmp_file_pos);
        fread(int_char,4,1,bmpfile);
        bmp_pals = (int_char[3] << 24 & 0xff000000) + (int_char[2] << 16 & 0xff0000) + (int_char[1] << 8 & 0xff00) + (int_char[0] & 0xff) - 54;
        bmp_pal_data = malloc(bmp_pals);
        bmp_file_pos = 54;
        fsetpos(bmpfile,&bmp_file_pos);
        fread(bmp_pal_data,bmp_pals,1,bmpfile);
        bmpfile_cache = malloc(bmp_size);
        bmp_file_pos = bmp_pals + 54;
        fsetpos(bmpfile,&bmp_file_pos);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        for(bmp_pxi;bmp_pxi<bmp_size;bmp_pxi++)
        {
            bmp_pali = (bmpfile_cache[bmp_pxi] & 0xff) * 4;
            dest[dest_i] = bmp_pal_data[bmp_pali+2];
            dest[dest_i+1] = bmp_pal_data[bmp_pali+1];
            dest[dest_i+2] = bmp_pal_data[bmp_pali];
            dest[dest_i+3] = bmp_pal_data[bmp_pali+3];
            dest_i += 4;
            bmp_pxx ++;
            if(bmp_pxx == bmp_w)
            {
                bmp_pxx = 0;
                bmp_pxi += bmp_line_end;
            }
        }
        free(bmp_pal_data);
        free(bmpfile_cache);
    }
    if(bmp_b == 16)
    {
        bmpfile_cache = malloc(bmp_size);
        bmp_file_pos = 54;
        fsetpos(bmpfile,&bmp_file_pos);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        for(bmp_pxi;bmp_pxi<bmp_size;bmp_pxi+=2)
        {
            col_c1 = bmpfile_cache[bmp_pxi];
            col_c2 = bmpfile_cache[bmp_pxi+1];
            col_16 = (col_c2 * 0x100 & 0xffff) + (col_c1 & 0xff);
            dest[dest_i] = (col_16 >> 10 & 0x1f) * 8;
            dest[dest_i+1] = (col_16 >> 5 & 0x1f) * 8;
            dest[dest_i+2] = (col_16 & 0x1f) * 8;
            dest[dest_i+3] = (col_16 >> 15 & 0x1) * 255;
            dest_i += 4;
            bmp_pxx ++;
            if(bmp_pxx == bmp_w)
            {
                bmp_pxx = 0;
                bmp_pxi += bmp_line_end;
            }
        }
        free(bmpfile_cache);
    }
    if(bmp_b == 24)
    {
        bmpfile_cache = malloc(bmp_size);
        bmp_file_pos = 54;
        fsetpos(bmpfile,&bmp_file_pos);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        for(bmp_pxi;bmp_pxi<bmp_size;bmp_pxi+=3)
        {
            col_c1 = bmpfile_cache[bmp_pxi];
            col_c2 = bmpfile_cache[bmp_pxi+1];
            col_c3 = bmpfile_cache[bmp_pxi+2];
            if(col_c1 != 0x01 || col_c2 != 0x01 || col_c3 != 0x01)
            {
                dest[dest_i] = col_c3;
                dest[dest_i+1] = col_c2;
                dest[dest_i+2] = col_c1;
                dest[dest_i+3] = 0xff;
            }
            else
            {
                dest[dest_i] = 0x00;
                dest[dest_i+1] = 0x00;
                dest[dest_i+2] = 0x00;
                dest[dest_i+3] = 0x00;
            }
            dest_i += 4;
            bmp_pxx ++;
            if(bmp_pxx == bmp_w)
            {
                bmp_pxx = 0;
                bmp_pxi += bmp_line_end;
            }
        }
        free(bmpfile_cache);
    }
    if(bmp_b == 32)
    {
        bmpfile_cache = malloc(bmp_size);
        bmp_file_pos = 54;
        fsetpos(bmpfile,&bmp_file_pos);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        for(bmp_pxi;bmp_pxi<bmp_size;bmp_pxi+=4)
        {
            dest[dest_i] = bmpfile_cache[bmp_pxi+2];
            dest[dest_i+1] = bmpfile_cache[bmp_pxi+1];
            dest[dest_i+2] = bmpfile_cache[bmp_pxi];
            dest[dest_i+3] = bmpfile_cache[bmp_pxi+3];
            dest_i += 4;
        }
        free(bmpfile_cache);
    }
}
//--------------------------------------------------------------------