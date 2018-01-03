#include "MyImgList.h"
#include<stdio.h>
#include<string.h>


MyImgList::MyImgList() 
:m_iCurIndex(0)
{
    m_iMaxSize = (m_iMaxSize <= 0) ? 5 : m_iMaxSize;

#ifdef USE_LINUX

#else
    InitializeCriticalSection(&m_csList);
#endif

    InitArray();
}


MyImgList::~MyImgList()
{
    ClearArray();
#ifdef USE_LINUX

#else
      DeleteCriticalSection(&m_csList);
#endif

}

void MyImgList::AddOneIMG(IMG_BMP* srcImg)
{
#ifdef USE_LINUX
    m_mutexList.lock();
#else
    EnterCriticalSection(&m_csList);
#endif
    //IMG_BMP*pImg = m_ImgArray.at(m_iCurIndex++);
    int iIndex = m_iCurIndex;
    if(iIndex >= (MAX_ARRAY_SIZE -1) )
    {
        iIndex = 0;
    }
    else
    {
        iIndex++;
    }
    IMG_BMP*pDestImg = m_ImgArray2[iIndex];
    if( (srcImg->ImgLength > 0 )    && (srcImg->ImgLength < pDestImg->iBufferLenth)  && pDestImg->pData   && srcImg->pData)
    {
//        printf("srcImg->ImgLength > 0 \n");

//        printf("srcImg->ImgLength = %d\n",srcImg->ImgLength );
//        printf("pDestImg->iBufferLenth = %d\n",pDestImg->iBufferLenth );

//        if(srcImg->ImgLength < pDestImg->iBufferLenth)
//        {
//            printf("srcImg->ImgLength = %d, < pDestImg->iBufferLenth = %d \n", srcImg->ImgLength, pDestImg->iBufferLenth );
//        }
//        if(pDestImg->pData )
//        {
//            printf("pDestImg->pData  \n");
//        }
//        if(srcImg->pData)
//        {
//            printf("srcImg->pData  \n");
//        }

        memcpy(pDestImg->pData, srcImg->pData, srcImg->ImgLength);
        pDestImg->ImgLength = srcImg->ImgLength;
        pDestImg->width = srcImg->width;
        pDestImg->height = srcImg->height;
        pDestImg->type = srcImg->type;

        m_iCurIndex = iIndex;
    }
    else
    {
        printf("MyImgList::AddOneIMG failed. src buffer size = %d, dest buffer size = %d\n", srcImg->ImgLength, pDestImg->iBufferLenth);
    }

#ifdef USE_LINUX
    m_mutexList.unlock();
#else
    LeaveCriticalSection(&m_csList);
#endif
}

IMG_BMP* MyImgList::getOneIMG()
{
    IMG_BMP*pImg = NULL;

#ifdef USE_LINUX
    m_mutexList.lock();
#else
    EnterCriticalSection(&m_csList);
#endif

    //if (m_iCurIndex == 0 && (m_ImgArray.at(m_iMaxSize-1)->ImgLength == 0) )
    if (m_iCurIndex == 0 && (m_ImgArray2[m_iMaxSize-1]->ImgLength == 0) )
    {
        pImg = NULL;
    }
    else
    {
        int iIndex = (m_iCurIndex == 0) ? (m_iMaxSize - 1) : (m_iCurIndex - 1);
        //IMG_BMP*pSrcImg = m_ImgArray.at(iIndex);
        IMG_BMP*pSrcImg = m_ImgArray2[iIndex];
        pImg = new IMG_BMP();
        if (pImg)
        {
            pImg->pData = new unsigned char[pSrcImg->ImgLength];
            memcpy(pImg->pData, pSrcImg->pData, pSrcImg->ImgLength);
            pImg->ImgLength = pSrcImg->ImgLength;
            pImg->width = pSrcImg->width;
            pImg->height = pSrcImg->height;
            pImg->type = pSrcImg->type;
        }
        pSrcImg = NULL;
    }

#ifdef USE_LINUX
    m_mutexList.unlock();
#else
    LeaveCriticalSection(&m_csList);
#endif

    return pImg;
}

void MyImgList::InitArray()
{
#ifdef USE_LINUX
    m_mutexList.lock();
#else
    EnterCriticalSection(&m_csList);
#endif

    IMG_BMP* pTemp = NULL;
    for (int i = 0; i < MAX_ARRAY_SIZE;i++)
    {
        pTemp = new IMG_BMP();
        if (pTemp)
        {
            pTemp->pData = new unsigned char[MAX_IMG_BUFFER_SIZE];
            if(pTemp->pData)
            {
                memset(pTemp->pData, 0, MAX_IMG_BUFFER_SIZE);

                pTemp->iBufferLenth = MAX_IMG_BUFFER_SIZE;
                pTemp->height = 0;
                pTemp->width = 0;
                pTemp->ImgLength = 0;
                m_ImgArray2[i] = pTemp;
            }
            else
            {
                delete pTemp;
                pTemp = NULL;
                i--;
                printf("MyImgList::InitArray() , alloc image buffer failed.\n");
            }
        }
        pTemp = NULL;
    }

#ifdef USE_LINUX
    m_mutexList.unlock();
#else
    LeaveCriticalSection(&m_csList);
#endif

}

void MyImgList::ClearArray()
{
#ifdef USE_LINUX
    m_mutexList.lock();
#else
    EnterCriticalSection(&m_csList);
#endif

    IMG_BMP* pTemp = NULL;
    for (int i = 0; i < MAX_ARRAY_SIZE; i++)
    {
        //pTemp = m_ImgArray.at(i);
        pTemp = m_ImgArray2[i];
        
        if (pTemp)
        {
            delete pTemp;
            pTemp = NULL;
        }
        //m_ImgArray.at(i) = NULL;
        m_ImgArray2[i] = NULL;
    }

#ifdef USE_LINUX
    m_mutexList.unlock();
#else
    LeaveCriticalSection(&m_csList);
#endif

}
