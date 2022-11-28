#include <stdio.h>

typedef struct _BMPRGBA_BmpInf
{
    int BmpWidth;
    int BmpHeight;
    int BmpBit;
}BMPRGBA_BmpInf;

void BMPRGBA_BmpGetInf(BMPRGBA_BmpInf *bmpinf,FILE *bmp_file);
void BMPRGBA_BmpToRGBA(char *dest,BMPRGBA_BmpInf *bmpinf,FILE *bmp_file);