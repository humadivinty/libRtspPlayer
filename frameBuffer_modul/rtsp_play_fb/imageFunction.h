#ifndef IMAGE_FUNCTION_H
#define IMAGE_FUNCTION_H

#include <pthread.h>

#define USE_LINUX 1

typedef struct Tag_ThreadMutex
{
    Tag_ThreadMutex()
    {
        pthread_mutex_init(&mtx, 0);
    }
    ~Tag_ThreadMutex()
    {
        pthread_mutex_destroy(&mtx);
    }
    inline void lock()
    {

        pthread_mutex_lock(&mtx);
       //    printf("Enter %d\n", ++i);
    }
    inline void unlock()
    {
        pthread_mutex_unlock(&mtx);
     //     printf("Leave %d\n", ++t);
    }
    pthread_mutex_t mtx;
 //   int i;
 //   int t;
}ThreadMutex;


/**
* Convert BGR24 file to BMP file
* @param rgb24path    Location of input RGB file.
* @param width        Width of input RGB file.
* @param height       Height of input RGB file.
* @param url_out      Location of Output BMP file.
*/
int simplest_bgr24_to_bmp(unsigned char*pSrcBuffer, int iBufferLen, int width, int height, unsigned char * pDestBuffer, int* OutPutLength);

void SaveFile(void* pData, int length, char* chFileName);

#endif
