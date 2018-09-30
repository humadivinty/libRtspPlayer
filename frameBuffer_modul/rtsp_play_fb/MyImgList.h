#pragma once
//#include <array>
//#include <windows.h>
#include<stdio.h>
#include"imageFunction.h"

#define MAX_IMG_BUFFER_SIZE 10*1024*1024
#define MAX_ARRAY_SIZE 5


typedef struct tag_IMG_BMP
{
    unsigned char* pData;
    char type;
    short int iunUse;

    int iBufferLenth;
    int width;
    int height;
    int ImgLength;

    tag_IMG_BMP()
    {
        pData = NULL;
        type = 0;

        iBufferLenth = 0;
        width = 0;
        height = 0;
        ImgLength = 0;
    }

    ~tag_IMG_BMP()
    {
        if (pData)
        {
            delete[] pData;
            pData = NULL;
        }

        type = 0;

        iBufferLenth = 0;
        width = 0;
        height = 0;
        ImgLength = 0;
    }
}IMG_BMP;

class MyImgList
{
public:
    MyImgList();
    ~MyImgList(); 

    void AddOneIMG(IMG_BMP* img);
    IMG_BMP* getOneIMG();

private:
    void InitArray();
    void ClearArray();
private:
    //std::array<IMG_BMP*, MAX_ARRAY_SIZE> m_ImgArray;
    IMG_BMP* m_ImgArray2[MAX_ARRAY_SIZE];

#ifdef USE_LINUX
    ThreadMutex m_mutexList;
#else
    CRITICAL_SECTION m_csList;
#endif
    int m_iMaxSize;
    int m_iCurIndex;
};

