#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------------------------
// BMP Information Struct
//--------------------------------------------------------------------
typedef struct _BMPRGBA_BmpInf
{
    int BmpWidth;
    int BmpHeight;
    int BmpBit;
}BMPRGBA_BmpInf;
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Get information of the BMP image
//--------------------------------------------------------------------
void BMPRGBA_BmpGetInf(BMPRGBA_BmpInf *bmpinf,FILE *bmp_file)
{
    char bmp_head_data[54] = ""; //Data of the BMP file head
    fseek(bmp_file,0,SEEK_SET);
    fread(bmp_head_data,54,1,bmp_file);
    bmpinf->BmpWidth = ((bmp_head_data[21] & 0x000000ff) * 0x1000000) + ((bmp_head_data[20] & 0x000000ff) * 0x10000) + ((bmp_head_data[19] & 0x000000ff) * 0x100) + (bmp_head_data[18] & 0x000000ff);
    bmpinf->BmpHeight = ((bmp_head_data[25] & 0x000000ff) * 0x1000000) + ((bmp_head_data[24] & 0x000000ff) * 0x10000) + ((bmp_head_data[23] & 0x000000ff) * 0x100) + (bmp_head_data[22] & 0x000000ff);
    bmpinf->BmpBit = bmp_head_data[28] & 0x000000ff;
}
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// Read data of bmp image and convert to RGBA
//--------------------------------------------------------------------
void BMPRGBA_BmpToRGBA(char *dest,BMPRGBA_BmpInf *bmpinf,FILE *bmp_file)
{
    int bmp_w = bmpinf->BmpWidth; //BMP image width
    int bmp_h = bmpinf->BmpHeight; //BMP image height
    int bmp_b = bmpinf->BmpBit; //BMP image bit
    char data_4b[4] = ""; //4 byte data
    int bmp_size = 0; //BMP data size
    int bmp_line_end = 0; //bit of BMP line end
    int bmp_line_s = bmp_w * (bmp_b / 8); //BMP line size
    char col1 = 0x00; //Color 1
    char col2 = 0x00; //Color 2
    char col3 = 0x00; //Color 3
    int col16 = 0; //Color value of 16 bit
    int bmp_pxi = 0; //BMP px index
    int bmp_pxx = 0; //BMP px the position x
    int dest_i = 0; //Dest array index
    char *bmp_data_cache = NULL; //BMP data cache
    while(bmp_line_s % 4 != 0)
    {
        bmp_line_s ++;
        bmp_line_end ++;
    } 
    bmp_size = (bmp_w * bmp_h * (bmp_b / 8)) + bmp_h * bmp_line_end;
    if(bmp_b == 8)
    {
        int palette_size = 0;
        char *palette = NULL;
        fseek(bmp_file,10,SEEK_SET);
        fread(data_4b,4,1,bmp_file);
        palette_size = (((data_4b[3] & 0x000000ff) * 0x1000000) + ((data_4b[2] & 0x000000ff) * 0x10000) + ((data_4b[1] & 0x000000ff) * 0x100) + (data_4b[0] & 0x000000ff)) - 54;
        palette = malloc(palette_size);
        fseek(bmp_file,54,SEEK_SET);
        fread(palette,palette_size,1,bmp_file);
        bmp_data_cache = malloc(bmp_size);
        fseek(bmp_file,palette_size+54,SEEK_SET);
        fread(bmp_data_cache,bmp_size,1,bmp_file);
        while(bmp_pxi < bmp_size)
        {
            int pal_i = bmp_data_cache[bmp_pxi] & 0x000000ff;
            dest[dest_i*4] = palette[pal_i+2];
            dest[dest_i*4+1] = palette[pal_i+1];
            dest[dest_i*4+2] = palette[pal_i];
            dest[dest_i*4+3] = palette[pal_i+3];
            bmp_pxx ++;
            bmp_pxi ++;
            dest_i ++;
            if(bmp_pxx == bmp_w)
            {
                bmp_pxx = 0;
                bmp_pxi += bmp_line_end;
            }
        }
        free(palette);
    }
    if(bmp_b == 16)
    {
        bmp_data_cache = malloc(bmp_size);
        fseek(bmp_file,54,SEEK_SET);
        fread(bmp_data_cache,bmp_size,1,bmp_file);
        while(bmp_pxi < bmp_size)
        {
            col1 = bmp_data_cache[bmp_pxi];
            col2 = bmp_data_cache[bmp_pxi+1];
            if(col1 != 0x01 || col2 != 0x01)
            {
                col16 = (col2 * 0x100 & 0x0000ffff) + (col1 & 0x000000ff);
                dest[dest_i*4] = (col16 >> 10 & 0x0000001f) * 8;
                dest[dest_i*4+1] = (col16 >> 5 & 0x0000001f) * 8;
                dest[dest_i*4+2] = (col16 >> 5 & 0x0000001f) * 8;
                dest[dest_i*4+3] = 0xff;
            }
            else
            {
                dest[dest_i*4] = 0x00;
                dest[dest_i*4+1] = 0x00;
                dest[dest_i*4+2] = 0x00;
                dest[dest_i*4+3] = 0x00;
            }
            bmp_pxx ++;
            bmp_pxi += 2;
            dest_i ++;
            if(bmp_pxx == bmp_w)
            {
                bmp_pxx = 0;
                bmp_pxi += bmp_line_end;
            }
        }
    }
    if(bmp_b == 24)
    {
        bmp_data_cache = malloc(bmp_size);
        fseek(bmp_file,54,SEEK_SET);
        fread(bmp_data_cache,bmp_size,1,bmp_file);
        while(bmp_pxi < bmp_size)
        {
            col1 = bmp_data_cache[bmp_pxi];
            col2 = bmp_data_cache[bmp_pxi+1];
            col3 = bmp_data_cache[bmp_pxi+2];
            if(col1 != 0x01 || col2 != 0x01 || col3 != 0x01)
            {
                dest[dest_i*4] = col3;
                dest[dest_i*4+1] = col2;
                dest[dest_i*4+2] = col1;
                dest[dest_i*4+3] = 0xff;
            }
            else
            {
                dest[dest_i*4] = 0x00;
                dest[dest_i*4+1] = 0x00;
                dest[dest_i*4+2] = 0x00;
                dest[dest_i*4+3] = 0x00;
            }
            bmp_pxx ++;
            bmp_pxi += 3;
            dest_i ++;
            if(bmp_pxx == bmp_w)
            {
                bmp_pxx = 0;
                bmp_pxi += bmp_line_end;
            }
        }
    }
    if(bmp_b == 32)
    {
        bmp_data_cache = malloc(bmp_size);
        fseek(bmp_file,54,SEEK_SET);
        fread(bmp_data_cache,bmp_size,1,bmp_file);
        while(bmp_pxi < bmp_size)
        {
            dest[dest_i*4] = bmp_data_cache[bmp_pxi+2];
            dest[dest_i*4+1] = bmp_data_cache[bmp_pxi+1];
            dest[dest_i*4+2] = bmp_data_cache[bmp_pxi];
            dest[dest_i*4+3] = bmp_data_cache[bmp_pxi+3];
            bmp_pxi += 4;
            dest_i ++;
        }
    }
    free(bmp_data_cache);
}
//--------------------------------------------------------------------