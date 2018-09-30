#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
//#include <SDL/SDL.h>




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <windows.h>
#include<pthread.h>
    //#include <stdio.h>
    //#include <stdlib.h>
    //#include <string.h>
#include"fbshow.h"

#ifdef __cplusplus
}
#endif

#include "MyImgList.h"

class RtspPlayer
{
public:
    RtspPlayer(const char* Url);
    ~RtspPlayer();

    bool StartPlayVideo(int Pos_x, int Pos_y, int Width, int Height);
    bool StopPlayVideo();

    bool GetOneBmpImg(unsigned char* buffer, int* buffLength);
    bool GetOneJpgImg(unsigned char* buffer, int* buffLength);

    void play();
    void play_SdlThread();
    void play_SdlThread2();
    
    int GetSDLThreadIsStop();
private:
    //SDL
    void SetSDLThreadStop(bool ifStop);

    //avcodec
    bool InitCodec();
    void DeInitCodec();
    bool GetVideoIsConnect();
    void SetVideoIsConnect(bool bConnect);

    //int ShowFrame(struct SwsContext *pSwsContext, SDL_Overlay* pOverlay, SDL_Rect* pSdlRect, AVCodecContext* pCodecContext, AVFrame* pSrcFrame, AVFrame* pDestFrame, int Width, int Height);
    int ShowFrame(AVFrame *pSrcFrame,
                              AVPixelFormat srcFormat,
                              int destWidth,
                              int destHeight,
                              int posX,
                              int posY);

    int EncodeOneFrameToJpeg(AVFrame *pSrcFrame,
                             AVPixelFormat srcFormat,
                             unsigned char* DestBuffer,
                             int* bufferLength,
                             int destWidth,
                             int destHeight);

    int EncodeOneFrameToBMp(AVFrame *pSrcFrame,
                            AVPixelFormat srcFormat,
                            unsigned char* DestBuffer,
                            int* bufferLength,
                            int destWidth,
                            int destHeight);

    static void encode(AVCodecContext *enc_ctx,
                       AVFrame *frame,
                       AVPacket *pkt,
                       unsigned char *outfile,
                       int* piLength);

    bool SaveFile(void* pData, int length, char* chFileName);
public:
    //SDL
    int m_iSdlThread_exit;
    int m_iSdlThread_pause;

#ifdef USE_LINUX
    pthread_mutex_t m_mutex_play;
    pthread_cond_t m_cond_paly;
    pthread_t m_hpthreadPlay;
#else
    void* m_hthreadPlay;
#endif

private:

    char m_pUrl[260];
    int m_iWind_posX;
    int m_iWind_posY;
    int m_iWind_width;
    int m_iWind_height;

    //SDL
    //static int sdl_refresh_thread(void* opaque);
    int screen_w,screen_h;
    // SDL_Surface *m_pSdl_surface;
    // SDL_Overlay *m_pSdl_overlay;
    // SDL_Rect sdlRect;
    // SDL_Thread *m_hSdlThread;

    //avcodec
    AVFormatContext* m_pAVFmt_Ctx;
    AVCodec* m_pVideoCodec;
    AVCodecContext* m_pVideoCodecCtx;
    struct SwsContext* m_pImg_convert_ctx;
    AVPixelFormat m_srcAVPixeFormat;
    AVPixelFormat m_destShowAVPixeFormat;

    bool m_bIsVideoConnect;

    int m_iVideo_stream_idx; 
    int m_iAudio_stream_idx;

#ifdef USE_LINUX
    ThreadMutex m_mutexSdl;
#else
    CRITICAL_SECTION m_csSDL;
#endif

    MyImgList myBmpList;
    MyImgList myJpgList;

    FbShow m_fbShow;
};

