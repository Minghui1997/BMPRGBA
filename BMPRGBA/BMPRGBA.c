/*
MIT License

Copyright (C) 2024 Minghui

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

void BMPRGBA_BMP8LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile);
void BMPRGBA_BMP16LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile);
void BMPRGBA_BMP24LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile);
void BMPRGBA_BMP32LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile);

//-------------------------------------------------------------------------
// Get the BMP info
//-------------------------------------------------------------------------
void BMPRGBA_BMPGetInfo(BMPRGBA_BMPInfo* bmpinfo,FILE* bmpfile)
{
    char bmp_head[2] = ""; //BMP file head
    unsigned char bmp_info[12] = ""; //BMP info
    fpos_t bmpfile_pos = 0; //position of bmp file

    //init the BMP info struct
    bmpinfo->Width = 0;
    bmpinfo->Height = 0;
    bmpinfo->PixelBit = 0;

    //load the BMP file head
    fsetpos(bmpfile,&bmpfile_pos);
    fread(&bmp_head,2,1,bmpfile);

    if(bmp_head[0] == 'B' && bmp_head[1] == 'M') //check whether the BMP head is correct
    {
        //load the BMP info to the BMP info struct
        bmpfile_pos = 18;
        fsetpos(bmpfile,&bmpfile_pos);
        fread(&bmp_info,12,1,bmpfile);
        bmpinfo->Width = (bmp_info[3] << 24) | (bmp_info[2] << 16) | (bmp_info[1] << 8) | bmp_info[0];
        bmpinfo->Height = (bmp_info[7] << 24) | (bmp_info[6] << 16) | (bmp_info[5] << 8) | bmp_info[4];
        bmpinfo->PixelBit = (bmp_info[11] << 8) | bmp_info[10];
        bmpinfo->RGBAsize = (size_t)bmpinfo->Width * (size_t)bmpinfo->Height * 4;
    }
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Load the BMP image file and convert to RGBA32 format
//-------------------------------------------------------------------------
void BMPRGBA_BMPLoadToRGBA(unsigned char* pixeldata,int pixelorder,BMPRGBA_BMPInfo* bmpinfo,FILE* bmpfile)
{
    if(bmpinfo->Width > 0 && bmpinfo->Height > 0 && bmpinfo->PixelBit > 0) //check whether the BMP info is correct
    {
        if(bmpinfo->PixelBit == 8)
        {
            BMPRGBA_BMP8LoadToRGBA32(pixeldata,pixelorder,bmpinfo->Width,bmpinfo->Height,bmpfile);
        }
        if(bmpinfo->PixelBit == 16)
        {
            BMPRGBA_BMP16LoadToRGBA32(pixeldata,pixelorder,bmpinfo->Width,bmpinfo->Height,bmpfile);
        }
        if(bmpinfo->PixelBit == 24)
        {
            BMPRGBA_BMP24LoadToRGBA32(pixeldata,pixelorder,bmpinfo->Width,bmpinfo->Height,bmpfile);
        }
        if(bmpinfo->PixelBit == 32)
        {
            BMPRGBA_BMP32LoadToRGBA32(pixeldata,pixelorder,bmpinfo->Width,bmpinfo->Height,bmpfile);
        }
    }
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Load the BMP8 image file and convert to RGBA32 format
//-------------------------------------------------------------------------
void BMPRGBA_BMP8LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile)
{
    size_t stream_size = 16384; //size of the stream space
    unsigned char* stream = NULL; //stream space
    size_t stream_pos; //position of stream
    int stream_line_x; //line position x of stream
    int stream_line_y; //line position y of stream
    size_t bmp_line_size = (size_t)bmp_w; //size of the BMP line
    size_t bmp_line_align = 0; //align of the BMP line
    size_t bmp_slice_size = 0; //size of the BMP slice
    int bmp_slice_linen = 0; //line number of the BMP slice
    int bmp_slice_number = 0; //number of the BMP slice
    int bmp_slice_i; //BMP slice index
    size_t bmp_rem_size = 0; //remaining size of the BMP load
    int bmp_rem_linen = 0; //remaining line number of the BMP load
    fpos_t bmpfile_pos; //position of bmp file
    size_t pixeldata_pos = 0; //position of pixel data
    unsigned char col1; //color component 1
    unsigned char col2; //color component 2
    unsigned char col3; //color component 3
    unsigned char *palette = NULL; //palette space
    unsigned char bmp_info[2]; //BMP info
    size_t palette_size = 0; //size of palette
    size_t palette_i; //palette index

    //calculate the align of the BMP line
    while((bmp_line_size + bmp_line_align) % 4 != 0)
    {
        bmp_line_align ++;
    }

    bmp_line_size += bmp_line_align;

    //load the palette info of BMP image
    bmpfile_pos = 10;
    fsetpos(bmpfile,&bmpfile_pos);
    fread(bmp_info,2,1,bmpfile);
    palette_size = (bmp_info[1] << 8) | (bmp_info[0]) - 54;

    palette = (unsigned char*)malloc(palette_size); //allocate memory for palette

    //load the palette of BMP image
    bmpfile_pos = 54;
    fsetpos(bmpfile,&bmpfile_pos);
    fread(palette,palette_size,1,bmpfile);

    if(bmp_line_size <= 16384) //if size of the BMP line is less than or equal to 16384
    {
        bmp_slice_linen = (int)(stream_size / bmp_line_size);
        if(bmp_h < bmp_slice_linen)
        {
            bmp_slice_linen = bmp_h;
        }
        bmp_slice_size = (size_t)bmp_slice_linen * bmp_line_size;
        bmp_slice_number = bmp_h / bmp_slice_linen;
        bmp_rem_linen = bmp_h % bmp_slice_linen;
        bmp_rem_size = (size_t)bmp_rem_linen * bmp_line_size;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (54 + (fpos_t)palette_size + (fpos_t)bmp_rem_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=bmp_slice_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP8 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w + bmp_line_align) + (size_t)stream_line_x;
                        palette_i = (size_t)stream[stream_pos] * 4;
                        col1 = palette[palette_i+2];
                        col2 = palette[palette_i+1];
                        col3 = palette[palette_i];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col1;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 54 + (fpos_t)palette_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=bmp_rem_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP8 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w + bmp_line_align) + (size_t)stream_line_x;
                        palette_i = (size_t)stream[stream_pos] * 4;
                        col1 = palette[palette_i+2];
                        col2 = palette[palette_i+1];
                        col3 = palette[palette_i];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col1;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (54 + (fpos_t)palette_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_slice_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP8 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w + bmp_line_align) + (size_t)stream_line_x;
                        palette_i = (size_t)stream[stream_pos] * 4;
                        col1 = palette[palette_i+2];
                        col2 = palette[palette_i+1];
                        col3 = palette[palette_i];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col1;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = (54 + (fpos_t)palette_size) + ((fpos_t)bmp_h - (fpos_t)bmp_rem_linen) * (fpos_t)bmp_line_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_rem_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP8 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w + bmp_line_align) + (size_t)stream_line_x;
                        palette_i = (size_t)stream[stream_pos] * 4;
                        col1 = palette[palette_i+2];
                        col2 = palette[palette_i+1];
                        col3 = palette[palette_i];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col1;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        free(stream); //free the stream space
    }
    if(bmp_line_size > 16384) //if size of the BMP line is greater than 16384
    {
        stream_size = bmp_line_size;
        bmp_slice_size = bmp_line_size;
        bmp_slice_linen = 1;
        bmp_slice_number = bmp_h;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (54 + (fpos_t)palette_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP8 pixel convert to RGBA32 pixel
                    stream_pos = (size_t)stream_line_x;
                    palette_i = (size_t)stream[stream_pos] * 4;
                    col1 = palette[palette_i+2];
                    col2 = palette[palette_i+1];
                    col3 = palette[palette_i];
                    if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                    {
                        pixeldata[pixeldata_pos] = col1;
                        pixeldata[pixeldata_pos+1] = col2;
                        pixeldata[pixeldata_pos+2] = col3;
                        pixeldata[pixeldata_pos+3] = 0xff;
                    }
                    else
                    {
                        pixeldata[pixeldata_pos] = 0x00;
                        pixeldata[pixeldata_pos+1] = 0x00;
                        pixeldata[pixeldata_pos+2] = 0x00;
                        pixeldata[pixeldata_pos+3] = 0x00;
                    }
                    pixeldata_pos += 4;
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (54 + (fpos_t)palette_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP8 pixel convert to RGBA32 pixel
                    stream_pos = (size_t)stream_line_x;
                    palette_i = (size_t)stream[stream_pos] * 4;
                    col1 = palette[palette_i+2];
                    col2 = palette[palette_i+1];
                    col3 = palette[palette_i];
                    if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                    {
                        pixeldata[pixeldata_pos] = col1;
                        pixeldata[pixeldata_pos+1] = col2;
                        pixeldata[pixeldata_pos+2] = col3;
                        pixeldata[pixeldata_pos+3] = 0xff;
                    }
                    else
                    {
                        pixeldata[pixeldata_pos] = 0x00;
                        pixeldata[pixeldata_pos+1] = 0x00;
                        pixeldata[pixeldata_pos+2] = 0x00;
                        pixeldata[pixeldata_pos+3] = 0x00;
                    }
                    pixeldata_pos += 4;
                }
            }
        }
        free(stream); //free the stream space
    }
    free(palette); //free the palette
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Load the BMP16 image file and convert to RGBA32 format
//-------------------------------------------------------------------------
void BMPRGBA_BMP16LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile)
{
    size_t stream_size = 16384; //size of the stream space
    unsigned char* stream = NULL; //stream space
    size_t stream_pos; //position of stream
    int stream_line_x; //line position x of stream
    int stream_line_y; //line position y of stream
    size_t bmp_line_size = (size_t)bmp_w * 2; //size of the BMP line
    size_t bmp_line_align = 0; //align of the BMP line
    size_t bmp_slice_size = 0; //size of the BMP slice
    int bmp_slice_linen = 0; //line number of the BMP slice
    int bmp_slice_number = 0; //number of the BMP slice
    int bmp_slice_i; //BMP slice index
    size_t bmp_rem_size = 0; //remaining size of the BMP load
    int bmp_rem_linen = 0; //remaining line number of the BMP load
    fpos_t bmpfile_pos; //position of bmp file
    size_t pixeldata_pos = 0; //position of pixel data
    unsigned short int color_16bit; //16bit color
    unsigned char bmp_info[12]; //BMP info
    unsigned short int mask_red = 0; //mask red of BMP
    unsigned short int mask_green = 0; //mask green of BMP
    unsigned short int mask_blue = 0; //mask blue of BMP

    //load the BMP mask
    bmpfile_pos = 54;
    fsetpos(bmpfile,&bmpfile_pos);
    fread(bmp_info,12,1,bmpfile);
    mask_red = (bmp_info[1] << 8 & 0xff00) | (bmp_info[0] & 0xff);
    mask_green = (bmp_info[5] << 8 & 0xff00) | (bmp_info[4] & 0xff);
    mask_blue = (bmp_info[9] << 8 & 0xff00) | (bmp_info[8] & 0xff);

    //calculate the align of the BMP line
    while((bmp_line_size + bmp_line_align) % 4 != 0)
    {
        bmp_line_align ++;
    }

    bmp_line_size += bmp_line_align;
    if(bmp_line_size <= 16384) //if size of the BMP line is less than or equal to 16384
    {
        bmp_slice_linen = (int)(stream_size / bmp_line_size);
        if(bmp_h < bmp_slice_linen)
        {
            bmp_slice_linen = bmp_h;
        }
        bmp_slice_size = (size_t)bmp_slice_linen * bmp_line_size;
        bmp_slice_number = bmp_h / bmp_slice_linen;
        bmp_rem_linen = bmp_h % bmp_slice_linen;
        bmp_rem_size = (size_t)bmp_rem_linen * bmp_line_size;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (70 + (fpos_t)bmp_rem_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=bmp_slice_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP16 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 2 + bmp_line_align) + (size_t)stream_line_x * 2;
                        color_16bit = (stream[stream_pos+1] << 8) | (stream[stream_pos]);
                        if(color_16bit != 0x0101)
                        {
                            pixeldata[pixeldata_pos] = (color_16bit & mask_red) >> 8;
                            pixeldata[pixeldata_pos+1] = (color_16bit & mask_green) >> 3;
                            pixeldata[pixeldata_pos+2] = (color_16bit & mask_blue) << 3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 70;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=bmp_rem_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP16 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 2 + bmp_line_align) + (size_t)stream_line_x * 2;
                        color_16bit = (stream[stream_pos+1] << 8) | (stream[stream_pos]);
                        if(color_16bit != 0x0101)
                        {
                            pixeldata[pixeldata_pos] = (color_16bit & mask_red) >> 8;
                            pixeldata[pixeldata_pos+1] = (color_16bit & mask_green) >> 3;
                            pixeldata[pixeldata_pos+2] = (color_16bit & mask_blue) << 3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 70 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_slice_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP16 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 2 + bmp_line_align) + (size_t)stream_line_x * 2;
                        color_16bit = (stream[stream_pos+1] << 8) | (stream[stream_pos]);
                        if(color_16bit != 0x0101)
                        {
                            pixeldata[pixeldata_pos] = (color_16bit & mask_red) >> 8;
                            pixeldata[pixeldata_pos+1] = (color_16bit & mask_green) >> 3;
                            pixeldata[pixeldata_pos+2] = (color_16bit & mask_blue) << 3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 70 + ((fpos_t)bmp_h - (fpos_t)bmp_rem_linen) * (fpos_t)bmp_line_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_rem_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP16 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 2 + bmp_line_align) + (size_t)stream_line_x * 2;
                        color_16bit = (stream[stream_pos+1] << 8) | (stream[stream_pos]);
                        if(color_16bit != 0x0101)
                        {
                            pixeldata[pixeldata_pos] = (color_16bit & mask_red) >> 8;
                            pixeldata[pixeldata_pos+1] = (color_16bit & mask_green) >> 3;
                            pixeldata[pixeldata_pos+2] = (color_16bit & mask_blue) << 3;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        free(stream); //free the stream space
    }
    if(bmp_line_size > 16384) //if size of the BMP line is greater than 16384
    {
        stream_size = bmp_line_size;
        bmp_slice_size = bmp_line_size;
        bmp_slice_linen = 1;
        bmp_slice_number = bmp_h;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 70 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP16 pixel convert to RGBA32 pixel
                    stream_pos = (size_t)stream_line_x * 2;
                    color_16bit = (stream[stream_pos+1] << 8) | (stream[stream_pos]);
                    if(color_16bit != 0x0101)
                    {
                        pixeldata[pixeldata_pos] = (color_16bit & mask_red) >> 8;
                        pixeldata[pixeldata_pos+1] = (color_16bit & mask_green) >> 3;
                        pixeldata[pixeldata_pos+2] = (color_16bit & mask_blue) << 3;
                        pixeldata[pixeldata_pos+3] = 0xff;
                    }
                    else
                    {
                        pixeldata[pixeldata_pos] = 0x00;
                        pixeldata[pixeldata_pos+1] = 0x00;
                        pixeldata[pixeldata_pos+2] = 0x00;
                        pixeldata[pixeldata_pos+3] = 0x00;
                    }
                    pixeldata_pos += 4;
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 70 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP16 pixel convert to RGBA32 pixel
                    stream_pos = (size_t)stream_line_x * 2;
                    color_16bit = (stream[stream_pos+1] << 8) | (stream[stream_pos]);
                    if(color_16bit != 0x0101)
                    {
                        pixeldata[pixeldata_pos] = (color_16bit & mask_red) >> 8;
                        pixeldata[pixeldata_pos+1] = (color_16bit & mask_green) >> 3;
                        pixeldata[pixeldata_pos+2] = (color_16bit & mask_blue) << 3;
                        pixeldata[pixeldata_pos+3] = 0xff;
                    }
                    else
                    {
                        pixeldata[pixeldata_pos] = 0x00;
                        pixeldata[pixeldata_pos+1] = 0x00;
                        pixeldata[pixeldata_pos+2] = 0x00;
                        pixeldata[pixeldata_pos+3] = 0x00;
                    }
                    pixeldata_pos += 4;
                }
            }
        }
        free(stream); //free the stream space
    }
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
// Load the BMP24 image file and convert to RGBA32 format
//-------------------------------------------------------------------------
void BMPRGBA_BMP24LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile)
{
    size_t stream_size = 16384; //size of the stream space
    unsigned char* stream = NULL; //stream space
    size_t stream_pos; //position of stream
    int stream_line_x; //line position x of stream
    int stream_line_y; //line position y of stream
    size_t bmp_line_size = (size_t)bmp_w * 3; //size of the BMP line
    size_t bmp_line_align = 0; //align of the BMP line
    size_t bmp_slice_size = 0; //size of the BMP slice
    int bmp_slice_linen = 0; //line number of the BMP slice
    int bmp_slice_number = 0; //number of the BMP slice
    int bmp_slice_i; //BMP slice index
    size_t bmp_rem_size = 0; //remaining size of the BMP load
    int bmp_rem_linen = 0; //remaining line number of the BMP load
    fpos_t bmpfile_pos; //position of bmp file
    size_t pixeldata_pos = 0; //position of pixel data
    unsigned char col1; //color component 1
    unsigned char col2; //color component 2
    unsigned char col3; //color component 3

    //calculate the align of the BMP line
    while((bmp_line_size + bmp_line_align) % 4 != 0)
    {
        bmp_line_align ++;
    }

    bmp_line_size += bmp_line_align;
    if(bmp_line_size <= 16384) //if size of the BMP line is less than or equal to 16384
    {
        bmp_slice_linen = (int)(stream_size / bmp_line_size);
        if(bmp_h < bmp_slice_linen)
        {
            bmp_slice_linen = bmp_h;
        }
        bmp_slice_size = (size_t)bmp_slice_linen * bmp_line_size;
        bmp_slice_number = bmp_h / bmp_slice_linen;
        bmp_rem_linen = bmp_h % bmp_slice_linen;
        bmp_rem_size = (size_t)bmp_rem_linen * bmp_line_size;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (54 + (fpos_t)bmp_rem_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=bmp_slice_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP24 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 3 + bmp_line_align) + (size_t)stream_line_x * 3;
                        col1 = stream[stream_pos];
                        col2 = stream[stream_pos+1];
                        col3 = stream[stream_pos+2];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col3;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col1;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 54;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=bmp_rem_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP24 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 3 + bmp_line_align) + (size_t)stream_line_x * 3;
                        col1 = stream[stream_pos];
                        col2 = stream[stream_pos+1];
                        col3 = stream[stream_pos+2];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col3;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col1;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 54 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_slice_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP24 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 3 + bmp_line_align) + (size_t)stream_line_x * 3;
                        col1 = stream[stream_pos];
                        col2 = stream[stream_pos+1];
                        col3 = stream[stream_pos+2];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col3;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col1;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 54 + ((fpos_t)bmp_h - (fpos_t)bmp_rem_linen) * (fpos_t)bmp_line_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_rem_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP24 pixel convert to RGBA32 pixel
                        stream_pos = (size_t)stream_line_y * ((size_t)bmp_w * 3 + bmp_line_align) + (size_t)stream_line_x * 3;
                        col1 = stream[stream_pos];
                        col2 = stream[stream_pos+1];
                        col3 = stream[stream_pos+2];
                        if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                        {
                            pixeldata[pixeldata_pos] = col3;
                            pixeldata[pixeldata_pos+1] = col2;
                            pixeldata[pixeldata_pos+2] = col1;
                            pixeldata[pixeldata_pos+3] = 0xff;
                        }
                        else
                        {
                            pixeldata[pixeldata_pos] = 0x00;
                            pixeldata[pixeldata_pos+1] = 0x00;
                            pixeldata[pixeldata_pos+2] = 0x00;
                            pixeldata[pixeldata_pos+3] = 0x00;
                        }
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        free(stream); //free the stream space
    }
    if(bmp_line_size > 16384) //if size of the BMP line is greater than 16384
    {
        stream_size = bmp_line_size;
        bmp_slice_size = bmp_line_size;
        bmp_slice_linen = 1;
        bmp_slice_number = bmp_h;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 54 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP24 pixel convert to RGBA32 pixel
                    stream_pos = (size_t)stream_line_x * 3;
                    col1 = stream[stream_pos];
                    col2 = stream[stream_pos+1];
                    col3 = stream[stream_pos+2];
                    if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                    {
                        pixeldata[pixeldata_pos] = col3;
                        pixeldata[pixeldata_pos+1] = col2;
                        pixeldata[pixeldata_pos+2] = col1;
                        pixeldata[pixeldata_pos+3] = 0xff;
                    }
                    else
                    {
                        pixeldata[pixeldata_pos] = 0x00;
                        pixeldata[pixeldata_pos+1] = 0x00;
                        pixeldata[pixeldata_pos+2] = 0x00;
                        pixeldata[pixeldata_pos+3] = 0x00;
                    }
                    pixeldata_pos += 4;
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 54 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP24 pixel convert to RGBA32 pixel
                    stream_pos = (size_t)stream_line_x * 3;
                    col1 = stream[stream_pos];
                    col2 = stream[stream_pos+1];
                    col3 = stream[stream_pos+2];
                    if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
                    {
                        pixeldata[pixeldata_pos] = col3;
                        pixeldata[pixeldata_pos+1] = col2;
                        pixeldata[pixeldata_pos+2] = col1;
                        pixeldata[pixeldata_pos+3] = 0xff;
                    }
                    else
                    {
                        pixeldata[pixeldata_pos] = 0x00;
                        pixeldata[pixeldata_pos+1] = 0x00;
                        pixeldata[pixeldata_pos+2] = 0x00;
                        pixeldata[pixeldata_pos+3] = 0x00;
                    }
                    pixeldata_pos += 4;
                }
            }
        }
        free(stream); //free the stream space
    }
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// Load the BMP32 image file and convert to RGBA32 format
//-------------------------------------------------------------------------
void BMPRGBA_BMP32LoadToRGBA32(unsigned char* pixeldata,int pixelorder,int bmp_w,int bmp_h,FILE* bmpfile)
{
    size_t stream_size = 16384; //size of the stream space
    unsigned char* stream = NULL; //stream space
    size_t stream_pos; //position of stream
    int stream_line_x; //line position x of stream
    int stream_line_y; //line position y of stream
    size_t bmp_line_size = (size_t)bmp_w * 4; //size of the BMP line
    size_t bmp_slice_size = 0; //size of the BMP slice
    int bmp_slice_linen = 0; //line number of the BMP slice
    int bmp_slice_number = 0; //number of the BMP slice
    int bmp_slice_i; //BMP slice index
    size_t bmp_rem_size = 0; //remaining size of the BMP load
    int bmp_rem_linen = 0; //remaining line number of the BMP load
    fpos_t bmpfile_pos; //position of bmp file
    size_t pixeldata_pos = 0; //position of pixel data
    if(bmp_line_size <= 16384) //if size of the BMP line is less than or equal to 16384
    {
        bmp_slice_linen = (int)(stream_size / bmp_line_size);
        if(bmp_h < bmp_slice_linen)
        {
            bmp_slice_linen = bmp_h;
        }
        bmp_slice_size = (size_t)bmp_slice_linen * bmp_line_size;
        bmp_slice_number = bmp_h / bmp_slice_linen;
        bmp_rem_linen = bmp_h % bmp_slice_linen;
        bmp_rem_size = (size_t)bmp_rem_linen * bmp_line_size;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = (54 + (fpos_t)bmp_rem_size) + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=bmp_slice_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP32 pixel convert to RGBA32 pixel
                        stream_pos = ((size_t)stream_line_y * (size_t)bmp_w + (size_t)stream_line_x) * 4;
                        pixeldata[pixeldata_pos] = stream[stream_pos+2];
                        pixeldata[pixeldata_pos+1] = stream[stream_pos+1];
                        pixeldata[pixeldata_pos+2] = stream[stream_pos];
                        pixeldata[pixeldata_pos+3] = stream[stream_pos+3];
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 54;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=bmp_rem_linen-1;stream_line_y>=0;stream_line_y--) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP32 pixel convert to RGBA32 pixel
                        stream_pos = ((size_t)stream_line_y * (size_t)bmp_w + (size_t)stream_line_x) * 4;
                        pixeldata[pixeldata_pos] = stream[stream_pos+2];
                        pixeldata[pixeldata_pos+1] = stream[stream_pos+1];
                        pixeldata[pixeldata_pos+2] = stream[stream_pos];
                        pixeldata[pixeldata_pos+3] = stream[stream_pos+3];
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 54 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_slice_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP32 pixel convert to RGBA32 pixel
                        stream_pos = ((size_t)stream_line_y * (size_t)bmp_w + (size_t)stream_line_x) * 4;
                        pixeldata[pixeldata_pos] = stream[stream_pos+2];
                        pixeldata[pixeldata_pos+1] = stream[stream_pos+1];
                        pixeldata[pixeldata_pos+2] = stream[stream_pos];
                        pixeldata[pixeldata_pos+3] = stream[stream_pos+3];
                        pixeldata_pos += 4;
                    }
                }
            }
            if(bmp_rem_size > 0) //if BMP load have remainder
            {
                //load the BMP remainder to stream
                bmpfile_pos = 54 + ((fpos_t)bmp_h - (fpos_t)bmp_rem_linen) * (fpos_t)bmp_line_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_rem_size,1,bmpfile);

                for(stream_line_y=0;stream_line_y<bmp_rem_linen;stream_line_y++) //traversal the BMP lines in stream
                {
                    for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                    {
                        //will BMP32 pixels convert to RGBA32 pixels
                        stream_pos = ((size_t)stream_line_y * (size_t)bmp_w + (size_t)stream_line_x) * 4;
                        pixeldata[pixeldata_pos] = stream[stream_pos+2];
                        pixeldata[pixeldata_pos+1] = stream[stream_pos+1];
                        pixeldata[pixeldata_pos+2] = stream[stream_pos];
                        pixeldata[pixeldata_pos+3] = stream[stream_pos+3];
                        pixeldata_pos += 4;
                    }
                }
            }
        }
        free(stream); //free the stream space
    }
    if(bmp_line_size > 16384) //if size of the BMP line is greater than 16384
    {
        stream_size = bmp_line_size;
        bmp_slice_size = bmp_line_size;
        bmp_slice_linen = 1;
        bmp_slice_number = bmp_h;
        stream = (unsigned char*)malloc(stream_size); //allocate memory for stream space
        if(pixelorder == 0) //if is positive order load
        {
            for(bmp_slice_i=bmp_slice_number-1;bmp_slice_i>=0;bmp_slice_i--) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 54 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);

                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP32 pixel convert to RGBA32 pixel
                    stream_pos = stream_line_x * 4;
                    pixeldata[pixeldata_pos] = stream[stream_pos+2];
                    pixeldata[pixeldata_pos+1] = stream[stream_pos+1];
                    pixeldata[pixeldata_pos+2] = stream[stream_pos];
                    pixeldata[pixeldata_pos+3] = stream[stream_pos+3];
                    pixeldata_pos += 4;
                }
            }
        }
        if(pixelorder == 1) //if is reverse order load
        {
            for(bmp_slice_i=0;bmp_slice_i<bmp_slice_number;bmp_slice_i++) //traversal the BMP slices
            {
                //load the BMP slice to stream
                bmpfile_pos = 54 + (fpos_t)bmp_slice_i * (fpos_t)bmp_slice_size;
                fsetpos(bmpfile,&bmpfile_pos);
                fread(stream,bmp_slice_size,1,bmpfile);
                for(stream_line_x=0;stream_line_x<bmp_w;stream_line_x++) //traversal the pixels in BMP line in stream
                {
                    //will BMP32 pixel convert to RGBA32 pixel
                    stream_pos = stream_line_x * 4;
                    pixeldata[pixeldata_pos] = stream[stream_pos+2];
                    pixeldata[pixeldata_pos+1] = stream[stream_pos+1];
                    pixeldata[pixeldata_pos+2] = stream[stream_pos];
                    pixeldata[pixeldata_pos+3] = stream[stream_pos+3];
                    pixeldata_pos += 4;
                }
            }
        }
        free(stream); //free the stream space
    }
}
//-------------------------------------------------------------------------