#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct _BMPRGBA_BmpInf
{
    unsigned int BmpWidth;
    unsigned int BmpHeight;
    int BmpBit;
}BMPRGBA_BmpInf;

void BMPRGBA_BmpGetInf(BMPRGBA_BmpInf *bmpinf,FILE *bmp_file);
void BMPRGBA_BmpToRGBA(char *dest,BMPRGBA_BmpInf *bmpinf,FILE *bmp_file);