#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

typedef size_t ind_t;

typedef struct _BMPRGBA_BmpInf
{
    unsigned int BmpWidth;
    unsigned int BmpHeight;
    int BmpBit;
}BMPRGBA_BmpInf;

void BMPRGBA_BmpGetInf(BMPRGBA_BmpInf *bmpinf,FILE *bmpfile);
void BMPRGBA_BmpToRGBA(char *dest,BMPRGBA_BmpInf *bmpinf,FILE *bmpfile);