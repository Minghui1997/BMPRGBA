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
// Get information of the BMP image
//--------------------------------------------------------------------
void BMPRGBA_BmpGetInf(BMPRGBA_BmpInf *bmpinf,FILE *bmpfile)
{
    char bmp_head_data[54] = ""; //Data of the BMP file head
    fseek(bmpfile,0,SEEK_SET);
    fread(bmp_head_data,54,1,bmpfile);
    bmpinf->BmpWidth = ((bmp_head_data[21] & 0x000000ff) * 0x1000000) + ((bmp_head_data[20] & 0x000000ff) * 0x10000) + ((bmp_head_data[19] & 0x000000ff) * 0x100) + (bmp_head_data[18] & 0x000000ff);
    bmpinf->BmpHeight = ((bmp_head_data[25] & 0x000000ff) * 0x1000000) + ((bmp_head_data[24] & 0x000000ff) * 0x10000) + ((bmp_head_data[23] & 0x000000ff) * 0x100) + (bmp_head_data[22] & 0x000000ff);
    bmpinf->BmpBit = bmp_head_data[28] & 0x000000ff;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Read data of the bmp image,and convert to RGBA
//--------------------------------------------------------------------
void BMPRGBA_BmpToRGBA(char *dest,BMPRGBA_BmpInf *bmpinf,FILE *bmpfile)
{
    unsigned int bmp_w = bmpinf->BmpWidth; //BMP image width
    unsigned int bmp_h = bmpinf->BmpHeight; //BMP image height
    int bmp_b = bmpinf->BmpBit; //BMP image bit
    int bmp_line_end = ((bmp_w * bmp_b + 31) / 32 * 4) - bmp_w * (bmp_b / 8); //Size of the BMP line end
    size_t bmp_size = (bmp_w * bmp_h * (bmp_b / 8)) + bmp_h * bmp_line_end; //BMP pixel data size
    ind_t bmp_pxi = 0; //BMP pixel index
    unsigned int bmp_pxx = 0; //BMP pixel the position x
    ind_t dest_i = 0; //Dest array index
    char *bmpfile_cache; //BMP data cache
    int bmp_pals; //BMP palette size
    char *bmp_pal_data; //BMP palette data
    int bmp_pali; //BMP palette index
    char data_4b[4] = ""; //4 byte data
    char col1; //Color 1
    char col2; //Color 2
    char col3; //Color 3
    int col16; //Color value of the 16 bit pixel
    if(bmp_b == 8)
    {
        fseek(bmpfile,10,SEEK_SET);
        fread(data_4b,4,1,bmpfile);
        bmp_pals = (((data_4b[3] & 0x000000ff) * 0x1000000) + ((data_4b[2] & 0x000000ff) * 0x10000) + ((data_4b[1] & 0x000000ff) * 0x100) + (data_4b[0] & 0x000000ff)) - 54;
        bmp_pal_data = malloc(bmp_pals);
        fseek(bmpfile,54,SEEK_SET);
        fread(bmp_pal_data,bmp_pals,1,bmpfile);
        bmpfile_cache = malloc(bmp_size);
        fseek(bmpfile,bmp_pals+54,SEEK_SET);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        while(bmp_pxi < bmp_size)
        {
            bmp_pali = bmpfile_cache[bmp_pxi] & 0x000000ff;
            dest[dest_i] = bmp_pal_data[bmp_pali+2];
            dest[dest_i+1] = bmp_pal_data[bmp_pali+1];
            dest[dest_i+2] = bmp_pal_data[bmp_pali];
            dest[dest_i+3] = bmp_pal_data[bmp_pali+3];
            bmp_pxx ++;
            bmp_pxi ++;
            dest_i += 4;
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
        fseek(bmpfile,54,SEEK_SET);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        while(bmp_pxi < bmp_size)
        {
            col1 = bmpfile_cache[bmp_pxi];
            col2 = bmpfile_cache[bmp_pxi+1];
            col16 = (col2 * 0x100 & 0x0000ffff) + (col1 & 0x000000ff);
            dest[dest_i] = (col16 >> 10 & 0x0000001f) * 8;
            dest[dest_i+1] = (col16 >> 5 & 0x0000001f) * 8;
            dest[dest_i+2] = (col16 & 0x0000001f) * 8;
            dest[dest_i+3] = (col16 >> 15 & 0x00000001) * 255;
            bmp_pxx ++;
            bmp_pxi += 2;
            dest_i += 4;
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
        fseek(bmpfile,54,SEEK_SET);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        while(bmp_pxi < bmp_size)
        {
            col1 = bmpfile_cache[bmp_pxi];
            col2 = bmpfile_cache[bmp_pxi+1];
            col3 = bmpfile_cache[bmp_pxi+2];
            if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
            {
                dest[dest_i] = col3;
                dest[dest_i+1] = col2;
                dest[dest_i+2] = col1;
                dest[dest_i+3] = 0xff;
            }
            else
            {
                dest[dest_i] = 0x00;
                dest[dest_i+1] = 0x00;
                dest[dest_i+2] = 0x00;
                dest[dest_i+3] = 0x00;
            }
            bmp_pxx ++;
            bmp_pxi += 3;
            dest_i += 4;
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
        fseek(bmpfile,54,SEEK_SET);
        fread(bmpfile_cache,bmp_size,1,bmpfile);
        while(bmp_pxi < bmp_size)
        {
            dest[dest_i] = bmpfile_cache[bmp_pxi+2];
            dest[dest_i+1] = bmpfile_cache[bmp_pxi+1];
            dest[dest_i+2] = bmpfile_cache[bmp_pxi];
            dest[dest_i+3] = bmpfile_cache[bmp_pxi+3];
            bmp_pxi += 4;
            dest_i += 4;
        }
        free(bmpfile_cache);
    }
}
//--------------------------------------------------------------------