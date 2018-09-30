#include "imageFunction.h"
#include<stdio.h>
#include<string.h>

int simplest_bgr24_to_bmp(unsigned char*pSrcBuffer, int iBufferLen, int width, int height, unsigned char * pDestBuffer, int* OutPutLength)
{
    typedef struct
    {
        long imageSize;
        long blank;
        long startPosition;        
    }BmpHead;

    typedef struct
    {
        long  Length;
        long  width;
        long  height;
        unsigned short  colorPlane;
        unsigned short  bitColor;
        long  zipFormat;
        long  realSize;
        long  xPels;
        long  yPels;
        long  colorUse;
        long  colorImportant;
    }InfoHead;

    //int i = 0, j = 0;
    BmpHead m_BMPHeader = { 0, 0, 0 };

    InfoHead  m_BMPInfoHeader = { 0 , 0 , 0, 0, 0, 0, 0, 0, 0, 0, 0};
    char bfType[2] = { 'B', 'M' };
    int header_size = sizeof(bfType)+sizeof(BmpHead)+sizeof(InfoHead);
    unsigned char *rgb24_buffer = pSrcBuffer;
    unsigned char *pDest = pDestBuffer;

    m_BMPHeader.imageSize = 3 * width*height + header_size;
    m_BMPHeader.startPosition = header_size;

    m_BMPInfoHeader.Length = sizeof(InfoHead);
    m_BMPInfoHeader.width = width;
    //BMP storage pixel data in opposite direction of Y-axis (from bottom to top).  
    m_BMPInfoHeader.height = -height;
    m_BMPInfoHeader.colorPlane = 1;
    m_BMPInfoHeader.bitColor = 24;
    m_BMPInfoHeader.realSize = 3 * width*height;

    memcpy(pDest, bfType, sizeof(bfType));
    pDest += sizeof(bfType);
    memcpy(pDest, &m_BMPHeader, sizeof(m_BMPHeader));
    pDest += sizeof(m_BMPHeader);
    memcpy(pDest, &m_BMPInfoHeader, sizeof(m_BMPInfoHeader));
    pDest += sizeof(m_BMPInfoHeader);

    memcpy(pDest, rgb24_buffer, iBufferLen);

    *OutPutLength = 3 * width*height + sizeof(bfType)+sizeof(m_BMPHeader)+sizeof(m_BMPInfoHeader);

    //BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2  
    //It saves pixel data in Little Endian  
    //So we change 'R' and 'B'  
    //for (j = 0; j<height; j++){
    //    for (i = 0; i<width; i++){
    //        char temp = rgb24_buffer[(j*width + i) * 3 + 2];
    //        rgb24_buffer[(j*width + i) * 3 + 2] = rgb24_buffer[(j*width + i) * 3 + 0];
    //        rgb24_buffer[(j*width + i) * 3 + 0] = temp;
    //    }
    //}

    return 0;
}

void SaveFile(void* pData, int length, char* chFileName)
{
    FILE* pFile = NULL;
    //fopen_s(&pFile, chFileName, "wb");
    pFile = fopen(chFileName, "wb");

    if (pFile)
    {
        fwrite(pData, 1, length, pFile);
        fclose(pFile);
        pFile = NULL;
    }
}

