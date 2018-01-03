#include "RtspPlayer.h"
#include <stdio.h>  

#include<signal.h>
#include "log4z.h"
using namespace zsummer::log4z;

//Refresh Event  
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1) 
#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)  

#define INBUF_SIZE 4096
#define SCALE_SIZE 1

#define  USE_DL_Thread 1
#define FRAME_INTERVAL 20

#ifndef USE_LINUX
#include <process.h>

unsigned __stdcall ThreadFunc(void* pArguments)
{
    if (!pArguments)
        return -1;

    RtspPlayer* pPlayer = (RtspPlayer*)pArguments;
#ifdef USE_DL_Thread
    //pPlayer->play_SdlThread();
    pPlayer->play_SdlThread2();
#else
    pPlayer->play();
#endif
    return 0;
}
#endif

void* pThreadFunc(void* pArguments)
{
    if (!pArguments)
    {
        return  0;
    }

    RtspPlayer* pPlayer = (RtspPlayer*)pArguments;
#ifdef USE_DL_Thread
    //pPlayer->play_SdlThread();
    pPlayer->play_SdlThread2();
#else
    pPlayer->play();
#endif
    pthread_cond_signal(&(pPlayer->m_cond_paly));
    return 0;
}

//int sdl_refresh_threadex(void* opaque)
//{
//    printf("sdl_refresh_threadex begin \n");
//    RtspPlayer* pRtspPlayer = NULL;
//    if (opaque)
//    {
//        pRtspPlayer = (RtspPlayer*)opaque;
//    }
//    else
//    {
//        fprintf(stderr, "sdl_refresh_thread, opaque= NULL.\n");
//        return -1;
//    }

//    int iThreadStop = pRtspPlayer->GetSDLThreadIsStop();
//    while (!iThreadStop)
//    {
//        iThreadStop = pRtspPlayer->GetSDLThreadIsStop();
//        if (!pRtspPlayer->m_iSdlThread_pause)
//        {
//            //printf("push sdl event SFM_REFRESH_EVENT\n");
//            SDL_Event event_sdl;
//            event_sdl.type = SFM_REFRESH_EVENT;
//            SDL_PushEvent(&event_sdl);
//        }
//        //printf("sdl_refresh_threadex running .\n");
//        SDL_Delay(20);
//    }
//    //pRtspPlayer->g_iThread_exit = 0;
//    //pRtspPlayer->g_iThread_pause = 0;
//    //Break

//    SDL_Event event_sdl2;
//    event_sdl2.type = SFM_BREAK_EVENT;
//    SDL_PushEvent(&event_sdl2);
//    printf("sdl_refresh_threadex exit .\n");
//    return 0;
//}

RtspPlayer::RtspPlayer(const char* Url) :  m_iSdlThread_pause(0)
{

    //m_sdlEvent;
//    m_pSDLScreen = NULL;
//    m_pSdlRenderer = NULL;
//    m_pSdlTexture = NULL;
//    m_hSdlThread = NULL;
    sprintf(m_pUrl, "%s", Url);

    m_pAVFmt_Ctx = NULL;
    m_pVideoCodec = NULL;
    m_pVideoCodecCtx = NULL;
    m_pImg_convert_ctx = NULL;

    m_pSdl_surface = NULL;
    m_pSdl_overlay = NULL;
    m_hSdlThread =NULL;

#ifdef USE_LINUX
    m_mutex_play = PTHREAD_MUTEX_INITIALIZER;
    m_cond_paly = PTHREAD_COND_INITIALIZER;
#else
    m_hthreadPlay(NULL),
    InitializeCriticalSection(&m_csSDL);
#endif

    ILog4zManager::getRef().setLoggerPath(LOG4Z_MAIN_LOGGER_ID, "./log_rtsp");
    ILog4zManager::getRef().start();
    ILog4zManager::getRef().setLoggerLevel(LOG4Z_MAIN_LOGGER_ID, LOG_LEVEL_TRACE);

    av_register_all();
}

RtspPlayer::~RtspPlayer()
{
    StopPlayVideo();

#ifdef USE_LINUX
#else
    DeleteCriticalSection(&m_csSDL);
#endif
}

bool RtspPlayer::StartPlayVideo(int Pos_x, int Pos_y, int Width, int Height)
{
    m_iWind_posX = Pos_x;
    m_iWind_posY = Pos_y;
    m_iWind_width = Width;
    m_iWind_height = Height;
    SetSDLThreadStop(false);
#ifdef USE_LINUX
    int iRet = pthread_create(&m_hpthreadPlay, NULL, pThreadFunc, this);
    if(iRet != 0)
    {
        //printf("pthread_create failed. %s: %d\n",__func__, strerror(iRet));
        LOGFMTE("pthread_create failed. %s: %d\n",__func__, strerror(iRet));
    }
#else
    if (!m_hthreadPlay)
    {
        m_hthreadPlay = (void*)_beginthreadex(NULL, 0, ThreadFunc, this, 0, &m_iThreadID);
        printf("create thread success./n");
        return true;
    }
    else
    {
        printf("the thread is already exist.\n");
        return false;
    }
#endif
}

bool RtspPlayer::StopPlayVideo()
{
    if(GetSDLThreadIsStop())
        return true;
    SetSDLThreadStop(true);
#ifdef USE_LINUX
    int kill_rc = pthread_kill(m_hpthreadPlay,0);

    if(kill_rc == ESRCH)
    {
        //printf("the specified thread did not exists or already quit\n");
        LOGFMTW("the specified thread did not exists or already quit\n");
    }
    else if(kill_rc == EINVAL)
    {
        //printf("signal is invalid\n");
        LOGFMTW("signal is invalid\n");
    }
    else
    {
        LOGD("the specified thread is alive\n");
        LOGD( "pthread_cond_wait...\n" );
        pthread_cond_wait(&m_cond_paly, &m_mutex_play);

        LOGD( "Stop PlayVideo thread begin.\n" );
        if(pthread_join(m_hpthreadPlay,NULL) != 0)
        {
             LOGE("pthread_join error.\n");
        }
        LOGD( "Stop PlayVideo thread end.\n" );
    }

#else
    if (m_hthreadPlay)
    {
        WaitForSingleObject(m_hthreadPlay, INFINITE);
        CloseHandle(m_hthreadPlay);
        m_hthreadPlay = NULL;
    }
#endif
    DeInitSDL_component();
    DeInitCodec();

    return true;
}


bool RtspPlayer::GetOneBmpImg(unsigned char* buffer, int* buffLength)
{
    if(!GetVideoIsConnect())
    {
        LOGW("GetOneBmpImg , the video is not connect.");
        return false;
    }

    IMG_BMP* pImg = NULL;
    pImg = myBmpList.getOneIMG();
    if (pImg && pImg->ImgLength > 0)
    {
        memcpy(buffer, pImg->pData, pImg->ImgLength);
        *buffLength = pImg->ImgLength;

        delete pImg;
        pImg = NULL;
        return true;
    }
    else
    {
        LOGW("GetOneBmpImg , the image is not ready.");
        return false;
    }
}

bool RtspPlayer::GetOneJpgImg(unsigned char* buffer, int* buffLength)
{
    if(!GetVideoIsConnect())
    {
        LOGW("GetOneJpgImg , the video is not connect.");
        return false;
    }


    IMG_BMP* pImg = NULL;
    pImg = myJpgList.getOneIMG();
    if (pImg && pImg->ImgLength > 0)
    {
        memcpy(buffer, pImg->pData, pImg->ImgLength);
        *buffLength = pImg->ImgLength;

        delete pImg;
        pImg = NULL;
        return true;
    }
    else
    {
        LOGW("GetOneJpgImg , the image is not ready.");
        return false;
    }
}

void RtspPlayer::play()
{
    InitCodec();
    InitSDL_component(m_iWind_posX, m_iWind_posY, m_iWind_width, m_iWind_height);

    //初始化帧容器
    AVFrame* pFrame = NULL, *pFrameOutput = NULL;
    pFrame = av_frame_alloc();
    pFrameOutput = av_frame_alloc();

    int i_output_width = m_iWind_width;
    int i_output_height = m_iWind_height;
    ////为输出Frame申请内存空间
    uint8_t * pbOut_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(m_destShowAVPixeFormat, i_output_width, i_output_height, 4) + AV_INPUT_BUFFER_PADDING_SIZE);
    ////将内存空间关联到Frame中
    av_image_fill_arrays(pFrameOutput->data, pFrameOutput->linesize, pbOut_buffer, m_destShowAVPixeFormat, i_output_width, i_output_height, 4);

    //AVPacket* pPacket = NULL;
    //pPacket = av_packet_alloc();

    const int iBufferLength = 1024 * 1024 * 10;
    int iJpgeLength = 0;
    int iBmpLength = 0;
    unsigned char * pBmpDest = new unsigned char[iBufferLength];
    unsigned char *pJpgDest = new unsigned char[iBufferLength];

    //从视频流中读取一帧的数据进行解码
    int iStop = GetSDLThreadIsStop();
    while (!iStop)
    {
        iStop = GetSDLThreadIsStop();
        //解码
        //AVPacket* pPacket = NULL;
        //pPacket = av_packet_alloc();

        AVPacket pPacket;
        av_init_packet(&pPacket);

        int iReadRet = av_read_frame(m_pAVFmt_Ctx, &pPacket);
        printf("packet size = %6d\n", pPacket.size);

        if (iReadRet >= 0)
        {
            if (pPacket.stream_index == m_iVideo_stream_idx)
            {
                if (pPacket.size)
                {
                    printf("packet size = %6d\n", pPacket.size);
                    int iRet = avcodec_send_packet(m_pVideoCodecCtx, &pPacket);
                    if (iRet < 0)
                    {
                        fprintf(stderr, "Error sending a packet for decoding\n");
                        //exit(1);
						continue;
                    }

                    while (iRet >= 0)
                    {
                        iRet = avcodec_receive_frame(m_pVideoCodecCtx, pFrame);
                        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
                        {
                            break;
                        }
                        else if (iRet < 0)
                        {
                            fprintf(stderr, "Error during decoding.\n");
                            //exit(1);
							break;
                        }
                        printf("receive frame %3d, width = %d, height = %d\n", m_pVideoCodecCtx->frame_number, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);

                        if(m_pSdl_overlay)
                        {
                            ShowFrame(m_pImg_convert_ctx, m_pSdl_overlay, &sdlRect, m_pVideoCodecCtx, pFrame, pFrameOutput, i_output_width, m_iWind_height);
                        }

                        if (m_pVideoCodecCtx->frame_number % FRAME_INTERVAL == 0)
                        {
                            //编码一帧JPEG图片
                            iJpgeLength = 0;
                            memset(pJpgDest, 0, iBufferLength);
                            int iRet = EncodeOneFrameToJpeg(pFrame, m_pVideoCodecCtx->pix_fmt, pJpgDest, &iJpgeLength, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
                            if (iRet == 0)
                            {
                                IMG_BMP pTempImg;
                                pTempImg.pData = pJpgDest;
                                pTempImg.width = m_pVideoCodecCtx->width;
                                pTempImg.height = m_pVideoCodecCtx->height;
                                pTempImg.type = 0;
                                pTempImg.ImgLength = iJpgeLength;
                                //printf("begin to AddOne.\n");
                                myJpgList.AddOneIMG(&pTempImg);
                                //printf("end AddOne.\n");
                                pTempImg.pData = NULL;
                            }

                            //编码一帧BMP图片
                            iBmpLength = 0;
                            memset(pBmpDest, 0, iBufferLength);
                            iRet = EncodeOneFrameToBMp(pFrame, m_pVideoCodecCtx->pix_fmt, pJpgDest, &iBmpLength, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
                            if (iRet == 0)
                            {
                                IMG_BMP pTempImg;
                                pTempImg.pData = pJpgDest;
                                pTempImg.width = m_pVideoCodecCtx->width;
                                pTempImg.height = m_pVideoCodecCtx->height;
                                pTempImg.type = 0;
                                pTempImg.ImgLength = iJpgeLength;
                                //printf("begin to AddOne.\n");
                                myBmpList.AddOneIMG(&pTempImg);
                                //printf("end AddOne.\n");
                                pTempImg.pData = NULL;
                            }

                        }
                    }
                }
            }
            av_packet_unref(&pPacket);
        }
        else
        {
            fprintf(stderr, "av_read_frame failed , return value = %d .\n", iReadRet);
            av_packet_unref(&pPacket);
            continue;
        }
        
        //av_packet_free(&pPacket);
    }

    if (pBmpDest)
    {
        delete[] pBmpDest;
        pBmpDest = NULL;
    }

    if (pJpgDest)
    {
        delete[] pJpgDest;
        pJpgDest = NULL;
    }

    if (pbOut_buffer)
    {
        av_free(pbOut_buffer);
        pbOut_buffer = NULL;
    }
    if (pFrame)
    {
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    if (pFrameOutput)
    {
        av_frame_free(&pFrameOutput);
        pFrameOutput = NULL;
    }
}

void RtspPlayer::play_SdlThread2()
{    
    bool bConnect = InitCodec();
    SetVideoIsConnect(bConnect);

    bool bSDLfinish = InitSDL_component(m_iWind_posX, m_iWind_posY, m_iWind_width, m_iWind_height);

    //初始化帧容器
    AVFrame* pFrame = NULL, *pFrameOutput = NULL;
    pFrame = av_frame_alloc();
    pFrameOutput = av_frame_alloc();

    int i_output_width = m_iWind_width;
    int i_output_height = m_iWind_height;
    //为输出Frame申请内存空间
    uint8_t * pbOut_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(m_destShowAVPixeFormat, i_output_width, i_output_height, 4) + AV_INPUT_BUFFER_PADDING_SIZE);
    //将内存空间关联到Frame中
    av_image_fill_arrays(pFrameOutput->data, pFrameOutput->linesize, pbOut_buffer, m_destShowAVPixeFormat, i_output_width, i_output_height, 4);

    //AVPacket* pPacket = NULL;
    //pPacket = av_packet_alloc();

    const int iBufferLength = 1024 * 1024 * 10;
    int iJpgeLength = 0;
    int iBmpLength = 0;
    unsigned char * pBmpDest = new unsigned char[iBufferLength];
    unsigned char *pJpgDest = new unsigned char[iBufferLength];

    LOGD("begin to Receive video.\n");
    //从视频流中读取一帧的数据进行解码
    int iStop = GetSDLThreadIsStop();    
    SDL_Event sdlEvent;

    while (!iStop)
    {
        iStop = GetSDLThreadIsStop();

        //printf("receive sdl event SFM_REFRESH_EVENT.\n");        
        if(!bConnect)
        {
            LOGD("Codec is not init yet.\n");
            bConnect = InitCodec();
            if(!bConnect)
                DeInitCodec();

            SetVideoIsConnect(bConnect);
            sleep(1);
            continue;
        }

        //LOGD("Codec finish init .\n");
        //解码
        AVPacket* pPacket = NULL;
        pPacket = av_packet_alloc();

        int iReadRet = av_read_frame(m_pAVFmt_Ctx, pPacket);
        //printf("packet size = %6d\n", pPacket->size);
        //av_packet_unref(pPacket);

        if (iReadRet >= 0)
        {
            if (pPacket->stream_index == m_iVideo_stream_idx)
            {
                if (pPacket->size)
                {
                    //printf("packet size = %6d\n", pPacket->size);
                    int iRet = avcodec_send_packet(m_pVideoCodecCtx, pPacket);
                    if (iRet < 0)
                    {
                        LOGE( "Error sending a packet for decoding\n");
                        //exit(1);
                        av_packet_free(&pPacket);
                        continue;
                    }

                    while (iRet >= 0)
                    {
                        iRet = avcodec_receive_frame(m_pVideoCodecCtx, pFrame);
                        if (iRet == AVERROR(EAGAIN) || iRet == AVERROR_EOF)
                        {
                            break;
                        }
                        else if (iRet < 0)
                        {
                            LOGE("Error during decoding.\n");
                            //exit(1);
                            break;
                        }
                        //printf("receive frame %3d, width = %d, height = %d\n", m_pVideoCodecCtx->frame_number, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
                        if(m_pSdl_overlay)
                        {
                            ShowFrame(m_pImg_convert_ctx, m_pSdl_overlay, &sdlRect, m_pVideoCodecCtx, pFrame, pFrameOutput, i_output_width, m_iWind_height);
                        }

                        if (m_pVideoCodecCtx->frame_number % FRAME_INTERVAL == 0)
                        {
                            //编码一帧JPEG图片
                            iJpgeLength = 0;
                            memset(pJpgDest, 0, iBufferLength);
                            int iRet = EncodeOneFrameToJpeg(pFrame, m_pVideoCodecCtx->pix_fmt, pJpgDest, &iJpgeLength, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
                            if (iRet == 0 && iJpgeLength > 0)
                            {
                                IMG_BMP pTempImg;
                                pTempImg.pData = pJpgDest;
                                pTempImg.width = m_pVideoCodecCtx->width;
                                pTempImg.height = m_pVideoCodecCtx->height;
                                pTempImg.type = 0;
                                pTempImg.ImgLength = iJpgeLength;
                                //printf("begin to AddOne.\n");
                                myJpgList.AddOneIMG(&pTempImg);
                                //printf("end AddOne.\n");
                                pTempImg.pData = NULL;
                            }

                            //编码一帧BMP图片
                            iBmpLength = 0;
                            memset(pBmpDest, 0, iBufferLength);
                            iRet = EncodeOneFrameToBMp(pFrame, m_pVideoCodecCtx->pix_fmt, pBmpDest, &iBmpLength, m_pVideoCodecCtx->width, m_pVideoCodecCtx->height);
                            if (iRet == 0 && iBmpLength > 0)
                            {
                                IMG_BMP pTempImg;
                                pTempImg.pData = pBmpDest;
                                pTempImg.width = m_pVideoCodecCtx->width;
                                pTempImg.height = m_pVideoCodecCtx->height;
                                pTempImg.type = 0;
                                pTempImg.ImgLength = iBmpLength;
                                //printf("begin to AddOne.\n");
                                myBmpList.AddOneIMG(&pTempImg);
                                //printf("end AddOne.\n");
                                pTempImg.pData = NULL;
                            }

                        }
                    }
                }
            }
            av_packet_free(&pPacket);
        }
        else
        {
            LOGFMTE("av_read_frame failed , return value = %d .\n", iReadRet);
            av_packet_free(&pPacket);

            DeInitCodec();
            bConnect = false;
            SetVideoIsConnect(bConnect);
            sleep(1);
            continue;
        }
        //printf("SDL_PollEvent . \n");
        if(bSDLfinish)
        {
            SDL_PollEvent(&sdlEvent);
            switch( sdlEvent.type )
            {
            case SDL_QUIT:
                SDL_Quit();
                LOGW("Receive SDL_QUIT event. \n");
                //exit(0);
                break;
            default:
                break;
            }
        }
        else
        {
            //InitSDL_component(m_iWind_posX, m_iWind_posY, m_iWind_width, m_iWind_height);
            sleep(1);
        }
    }
    if (pBmpDest)
    {
        delete[] pBmpDest;
        pBmpDest = NULL;
    }

    if (pJpgDest)
    {
        delete[] pJpgDest;
        pJpgDest = NULL;
    }

    if (pbOut_buffer)
    {
        av_free(pbOut_buffer);
        pbOut_buffer = NULL;
    }
    if (pFrame)
    {
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    if (pFrameOutput)
    {
        av_frame_free(&pFrameOutput);
        pFrameOutput = NULL;
    }
    LOGD("exit play_SdlThread2 .\n");
}

bool RtspPlayer::InitSDL_component(int Pos_x, int Pos_y, int Width, int Height)
{
    LOGFMTD("InitSDL_component.. posX= %d, posY=%d, posWid = %d, posHeight = %d",  Pos_x,  Pos_y,  Width,  Height);
    int i_output_width = Width;
    int i_output_height = Height;
    int iPos_x = Pos_x, iPos_y = Pos_y;
    //iPos_x = SDL_WINDOWPOS_UNDEFINED;
    //iPos_y = SDL_WINDOWPOS_UNDEFINED;

    LOGD("SDL_Init .");
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        LOGFMTE( "Could not initialize SDL - %s\n", SDL_GetError());
        return false;
    }
    char chPosSet[64] = {0};
    sprintf(chPosSet, "SDL_VIDEO_WINDOW_POS=%d,%d", iPos_x, iPos_y);
    LOGD("SDL_putenv SDL_VIDEO_CENTERED");
    SDL_putenv(const_cast<char*>("SDL_VIDEO_CENTERED="));

    LOGD("SDL_putenv pos");
    SDL_putenv(const_cast<char*>(chPosSet));

    if(m_pSdl_surface == NULL)
    {
        LOGD("SDL_SetVideoMode .");
        m_pSdl_surface = SDL_SetVideoMode(i_output_width, i_output_height, 0,0);
    }
    if(!m_pSdl_surface)
    {
        LOGFMTE("SDL: could not set video mode - exiting:%s\n",SDL_GetError());
        DeInitSDL_component();
        return false;
    }
    else
    {
        LOGD("SDL_SetVideoMode success.");
    }

    if(m_pSdl_overlay == NULL)
    {
        LOGD("SDL_CreateYUVOverlay .");
        m_pSdl_overlay = SDL_CreateYUVOverlay(i_output_width, i_output_height,SDL_YV12_OVERLAY, m_pSdl_surface);
    }
    if(m_pSdl_overlay == NULL)
    {
        LOGFMTE("SDL_CreateYUVOverlay failed, error = %s.\n", SDL_GetError());
        DeInitSDL_component();
        return false;
    }
    else
    {
         LOGFMTD("SDL_CreateYUVOverlay succes, info = %s.\n", SDL_GetError());
    }

    sdlRect.x = 0;
    sdlRect.y = 0;
    sdlRect.w = i_output_width;
    sdlRect.h = i_output_height;
    LOGD("InitSDL_component finished.");
    return true;
}

void RtspPlayer::DeInitSDL_component()
{
    LOGD("DeInitSDL_component ..");
//    SDL_Event event_sdl2;
//    event_sdl2.type = SFM_BREAK_EVENT;
//    SDL_PushEvent(&event_sdl2);

    //SetSDLThreadStop(true);
    //释放SDL相关资源
//    if (m_hSdlThread)
//    {
//        int threadReturnValue;
//        SDL_WaitThread(m_hSdlThread, &threadReturnValue);
//        LOGFMTD("\nSDL Thread returned value: %d", threadReturnValue);
//    }

    if(m_pSdl_overlay)
    {
        SDL_FreeYUVOverlay(m_pSdl_overlay);
        m_pSdl_overlay = NULL;
        LOGFMTD("SDL_FreeYUVOverlay. %s", SDL_GetError());
    }
    LOGD("SDL_Quit.");
    SDL_Quit();

    if(m_pSdl_surface)
    {
        m_pSdl_surface = NULL;
        LOGD("set pSdl_surface = NULL.");
    }

    LOGD("DeInitSDL_component finish.");
}

void RtspPlayer::SetSDLThreadStop(bool ifStop)
{
    int iStop = 0;
    iStop = ifStop ? 1 : 0;
#ifdef USE_LINUX
    m_mutexSdl.lock();
#else
    EnterCriticalSection(&m_csSDL);
#endif

    m_iSdlThread_exit = iStop;

#ifdef USE_LINUX
    m_mutexSdl.unlock();
#else
    LeaveCriticalSection(&m_csSDL);
#endif
   LOGFMTD("SetSDLThreadStop %d", ifStop);
}

int RtspPlayer::GetSDLThreadIsStop()
{
    int iStop = 0;
#ifdef USE_LINUX
    m_mutexSdl.lock();
#else
    EnterCriticalSection(&m_csSDL);
#endif
    iStop = m_iSdlThread_exit;
#ifdef USE_LINUX
    m_mutexSdl.unlock();
#else
    LeaveCriticalSection(&m_csSDL);
#endif
    //LOGFMTD("GetSDLThreadIsStop, value=  %d", ifStop);
    return iStop;
}

bool RtspPlayer::InitCodec()
{
    LOGD("InitCodec ..");
    char* pUrl = m_pUrl;
    LOGFMTD("URL = %s", pUrl);
    //Init decod
    avformat_network_init();
    //通过av_format来获取视频流的相关信息
    if(m_pAVFmt_Ctx == NULL)
    {
        m_pAVFmt_Ctx = avformat_alloc_context();
    }
    if(m_pAVFmt_Ctx == NULL)
    {
        LOGE("alloc avformate contex failed.");
        DeInitCodec();
        return false;
    }
    AVDictionary* opts = NULL;
    // TCP模式接收
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    // 设置缓冲区大小
    av_dict_set(&opts, "bufsize", "655350", 0);
    // 设置超时时间间隔x秒[单位为微秒]
    av_dict_set(&opts, "stimeout", "50000", 0);
    if (avformat_open_input(&m_pAVFmt_Ctx, pUrl, NULL, &opts) != 0)
    {
        //fprintf(stderr, "avformat_open_input failed.\n");
        //exit(1);

        LOGE( "avformat_open_input failed.");
        DeInitCodec();
        return false;
    }
    // 缩短提取流信息的时间
    m_pAVFmt_Ctx->probesize = 100 * 1024;
    m_pAVFmt_Ctx->max_analyze_duration = 5 * AV_TIME_BASE;

    if (avformat_find_stream_info(m_pAVFmt_Ctx, &opts) < 0)
    {
        //fprintf(stderr, "avformat_find_stream_info failed.\n");
        //exit(1);

        LOGE("avformat_find_stream_info failed.");
        DeInitCodec();
        return false;
    }
    //打印视频信息
    LOGD("\n--------------------------Stream info------------------------------\n");
    av_dump_format(m_pAVFmt_Ctx, 0, pUrl, 0);
    LOGFMTD("Find %d streams int the url %s.\n", m_pAVFmt_Ctx->nb_streams , pUrl);
    LOGFMTD("The time stamp of the first frame is %lld.\n", m_pAVFmt_Ctx->start_time / AV_TIME_BASE);
    LOGFMTD("the total time of the stream is %lld.\n", m_pAVFmt_Ctx->duration / AV_TIME_BASE);
    LOGFMTD("Total stream bit rate is %d bit/s.\n", m_pAVFmt_Ctx->bit_rate);
    LOGFMTD("input format is : %s .\n", m_pAVFmt_Ctx->iformat->name);
    //打印视频流信息
    m_iVideo_stream_idx = av_find_best_stream(m_pAVFmt_Ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    AVStream* pVideoStream = NULL;
    if (m_iVideo_stream_idx >= 0)
    {
        pVideoStream = m_pAVFmt_Ctx->streams[m_iVideo_stream_idx];
        LOGFMTD("video nbFrames : %lld. \n", pVideoStream->nb_frames);
        LOGFMTD("video codec_id: %d.\n", pVideoStream->codecpar->codec_id);
        LOGFMTD("video codec_name : %s.\n", avcodec_get_name(pVideoStream->codecpar->codec_id));
        LOGFMTD("video width x height: %d x %d .\n", pVideoStream->codecpar->width, pVideoStream->codecpar->height);
        LOGFMTD("video pix_fmt: %d. \n", pVideoStream->codecpar->format);
        LOGFMTD("video bit rate: %lld .\n", (int64_t)pVideoStream->codecpar->bit_rate / 1000);
        //LOGFMTD("video avg_frame_rate: %d fps\n", pVideoStream->avg_frame_rate.num / pVideoStream->avg_frame_rate.den);
    }
    else
    {
        //fprintf(stderr, "could not find any video steam.\n");
        LOGE("could not find any video steam.");
        DeInitCodec();
        return false;
    }
    //打印音频流信息
    m_iAudio_stream_idx = av_find_best_stream(m_pAVFmt_Ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    AVStream* pAudio_stream = NULL;
    if (m_iAudio_stream_idx >= 0)
    {
        pAudio_stream = m_pAVFmt_Ctx->streams[m_iAudio_stream_idx];
        LOGFMTD("audio codec_id: %d.\n", pAudio_stream->codecpar->codec_id);
        LOGFMTD("audio codec_name: %s.\n", avcodec_get_name(pAudio_stream->codecpar->codec_id));
        LOGFMTD("audio sample_rate: %d \n", pAudio_stream->codecpar->sample_rate);
        LOGFMTD("audio channels: %d", pAudio_stream->codecpar->channels);
        LOGFMTD("audio sample_fmt: %d.\n", pAudio_stream->codecpar->format);
        LOGFMTD("audio frame_size: %d.\n", pAudio_stream->codecpar->frame_size);
        LOGFMTD("audio nb_frames: %lld \n", pAudio_stream->nb_frames);
        LOGFMTD("audio bit rate :%lld \n", (int64_t)pAudio_stream->codecpar->bit_rate / 1000);
    }
    else
    {
        //fprintf(stderr, "could not find any audio steam.\n");
        LOGE("could not find any audio steam.");
    }
    LOGD("\n--------------------------Stream info end------------------------------\n");

    //查找解码器
    if(m_pVideoCodec == NULL)
    {
        m_pVideoCodec = avcodec_find_decoder(pVideoStream->codecpar->codec_id);
    }
    if (m_pVideoCodec == NULL)
    {
//        fprintf(stderr, "avcodec_find_decoder error.\n");
//        exit(1);
        DeInitCodec();
        LOGE( "avcodec_find_decoder error.");
        return false;
    }
    if(m_pVideoCodecCtx == NULL)
    {
        m_pVideoCodecCtx = avcodec_alloc_context3(m_pVideoCodec);
    }
    if (m_pVideoCodecCtx == NULL)
    {
//        fprintf(stderr, "avcodec_alloc_context3 error,\n");
//        exit(1);
        DeInitCodec();
        LOGE( "avcodec_alloc_context3 error,\n");
        return false;
    }
    //将解码的相关参数从视频流中复制到Codec_contex上
    avcodec_parameters_to_context(m_pVideoCodecCtx, pVideoStream->codecpar);
    //打开解码器
    if (avcodec_open2(m_pVideoCodecCtx, m_pVideoCodec, NULL) < 0)
    {
//        fprintf(stderr, "avcodec_open2 failed.\n");
//        exit(1);
        DeInitCodec();
        LOGE( "avcodec_open2 failed.");
        return false;
    }

    //初始化格式转换图片伸缩器
    int iSrcWidth = 0, iSrcHeight = 0;
    iSrcWidth = m_pVideoCodecCtx->width;
    iSrcHeight = m_pVideoCodecCtx->height;
    //AVPixelFormat srcAVPixeFormat = pVideoCodecCtx->pix_fmt;
    m_srcAVPixeFormat;
    //destAVPixeFormat = AV_PIX_FMT_BGR24;
    m_destShowAVPixeFormat = AV_PIX_FMT_YUV420P;
    //int i_output_width = pVideoCodecCtx->width * SCALE_SIZE;
    //int i_output_height = pVideoCodecCtx->height * SCALE_SIZE;
    int i_output_width = m_iWind_width;
    int i_output_height = m_iWind_height;
    //*这里RTSP的原格式是AV_PIX_FMT_YUVJ420P
    //*如果直接使用这个格式来转的话，会出现deprecated pixel format used, make sure you did set range correctly的警告
    //*目前的原因还不知道
    switch (m_pVideoCodecCtx->pix_fmt)
    {
    case AV_PIX_FMT_YUVJ420P:
        m_srcAVPixeFormat = AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        m_srcAVPixeFormat = AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        m_srcAVPixeFormat = AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        m_srcAVPixeFormat = AV_PIX_FMT_YUV440P;
        break;
    default:
        m_srcAVPixeFormat = m_pVideoCodecCtx->pix_fmt;
    }
    if (m_pImg_convert_ctx == NULL)
    {
        //fprintf(stderr, "get sws_getContext failed.\n");
        LOGE( "get sws_getContext failed.");
    }
    return true;
}

void RtspPlayer::DeInitCodec()
{
    LOGD("DeInitCodec..");
    avformat_network_deinit();
    if(m_pAVFmt_Ctx)
    {
        avformat_close_input(&m_pAVFmt_Ctx);
        m_pAVFmt_Ctx = NULL;
    }
    if(m_pVideoCodecCtx)
    {
        avcodec_free_context(&m_pVideoCodecCtx);
        m_pVideoCodecCtx = NULL;
    }
    if(m_pImg_convert_ctx)
    {
        sws_freeContext(m_pImg_convert_ctx);
        m_pImg_convert_ctx = NULL;
    }
    LOGD("DeInitCodec finish.");
}

bool RtspPlayer::GetVideoIsConnect()
{
    int iStop = 0;
#ifdef USE_LINUX
    m_mutexSdl.lock();
#else
    EnterCriticalSection(&m_csSDL);
#endif
    iStop = m_bIsVideoConnect;
#ifdef USE_LINUX
    m_mutexSdl.unlock();
#else
    LeaveCriticalSection(&m_csSDL);
#endif
    //LOGFMTD("GetSDLThreadIsStop, value=  %d", ifStop);
    return iStop;
}

void RtspPlayer::SetVideoIsConnect(bool bConnect)
{
#ifdef USE_LINUX
    m_mutexSdl.lock();
#else
    EnterCriticalSection(&m_csSDL);
#endif

    m_bIsVideoConnect = bConnect;

#ifdef USE_LINUX
    m_mutexSdl.unlock();
#else
    LeaveCriticalSection(&m_csSDL);
#endif
   LOGFMTD("SetVideoIsConnect %d", bConnect);
}

int RtspPlayer::ShowFrame(struct SwsContext *pSwsContext, SDL_Overlay* pOverlay, SDL_Rect* pSdlRect, AVCodecContext* pCodecContext, AVFrame* pSrcFrame, AVFrame* pDestFrame, int Width, int Height)
{
    if (!pOverlay || !pDestFrame || !pCodecContext || !pSrcFrame)
    {
        LOGE("Show frame failed, param is null.\n");
        return -1;
    }

    //像素格式转换 prame 转换成 pFrame Yuv
    int iDestWid = Width;
    int iDestHeight = Height;
    AVPixelFormat pixFormat;
    //*这里RTSP的原格式是AV_PIX_FMT_YUVJ420P
    //*如果直接使用这个格式来转的话，会出现deprecated pixel format used, make sure you did set range correctly的警告
    //*目前的原因还不知道
    switch (pCodecContext->pix_fmt)
    {
    case AV_PIX_FMT_YUVJ420P:
        pixFormat = AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        pixFormat = AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        pixFormat = AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        pixFormat = AV_PIX_FMT_YUV440P;
        break;
    default:
        pixFormat = pCodecContext->pix_fmt;
    }
    pSwsContext = sws_getContext(pCodecContext->width,
                                 pCodecContext->height,
                                 pixFormat,
                                 iDestWid,
                                 iDestHeight,
                                 //AV_PIX_FMT_YUV420P,
                                 //AV_PIX_FMT_BGR24,
                                 pixFormat,
                                 SWS_FAST_BILINEAR,
                                 NULL,
                                 NULL,
                                 NULL);

        //SDL 显示----------------------------
    if(0 == SDL_LockYUVOverlay(pOverlay))
    {
        pDestFrame->data[0]=pOverlay->pixels[0];
        pDestFrame->data[1]=pOverlay->pixels[2];
        pDestFrame->data[2]=pOverlay->pixels[1];
        pDestFrame->linesize[0]=pOverlay->pitches[0];
        pDestFrame->linesize[1]=pOverlay->pitches[2];
        pDestFrame->linesize[2]=pOverlay->pitches[1];

        sws_scale(pSwsContext, (const uint8_t* const*)pSrcFrame->data, pSrcFrame->linesize, 0, pSrcFrame->height, pDestFrame->data, pDestFrame->linesize);

        SDL_UnlockYUVOverlay(pOverlay);
        SDL_DisplayYUVOverlay(pOverlay, pSdlRect);
    }
    else
    {
        LOGFMTE("SDL_LockYUVOverlay failed, error info : %s.", SDL_GetError());
    }

    sws_freeContext(pSwsContext);

    //延时20ms
    //SDL_Delay(20);
    return 0;
}

int RtspPlayer::EncodeOneFrameToJpeg(AVFrame *pSrcFrame, AVPixelFormat srcFormat, unsigned char* DestBuffer, int* bufferLength, int destWidth, int destHeight)
{
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    int i, ret;
    AVFrame *pFrame;
    AVPacket* pPacket;
    pPacket = av_packet_alloc();

    int framecnt = 0;
    unsigned char* pBuffer = DestBuffer;
    int iFinalLength = 0;
    int iDestWidth = destWidth;
    int iDestHeight = destHeight;

    AVCodecID codec_id = AV_CODEC_ID_MJPEG;

    //int in_w = Frame->width, in_h = Frame->height;
    //int in_w = iDestWidth, in_h = iDestHeight;
    int framenum = 1;

    //avcodec_register_all();

    pCodec = avcodec_find_encoder(codec_id);
    if (!pCodec) {
        LOGE("Codec not found\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        LOGE("Could not allocate video codec context\n");
        return -1;
    }
    pCodecCtx->bit_rate = 4000000;
    pCodecCtx->width = iDestWidth;
    pCodecCtx->height = iDestHeight;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 11;
    pCodecCtx->gop_size = 75;
    //pCodecCtx->max_b_frames = 0;
    //pCodecCtx->global_quality = 1;
    //pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->pix_fmt = srcFormat;
	
	AVPixelFormat pixFormatSrc;
    //*这里RTSP的原格式是AV_PIX_FMT_YUVJ420P
    //*如果直接使用这个格式来转的话，会出现deprecated pixel format used, make sure you did set range correctly的警告
    //*目前的原因还不知道
    switch (srcFormat)
    {
    case AV_PIX_FMT_YUVJ420P:
        pixFormatSrc = AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        pixFormatSrc = AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        pixFormatSrc = AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        pixFormatSrc = AV_PIX_FMT_YUV440P;
        break;
    default:
        pixFormatSrc = srcFormat;
    }

    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("Could not open codec\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    if (!pFrame) {
        LOGE("Could not allocate video frame\n");
        return -1;
    }
    //AVPixelFormat srcFmt = AV_PIX_FMT_YUV420P;
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = iDestWidth;
    pFrame->height = iDestHeight;

    ret = av_image_alloc(pFrame->data, pFrame->linesize, pFrame->width, pFrame->height,
                         pCodecCtx->pix_fmt, 4);
    if (ret < 0)
    {
        LOGE("Could not allocate raw picture buffer\n");
        return -1;
    }

    struct SwsContext *img_convert_ctx = NULL;
    img_convert_ctx = sws_getContext(pSrcFrame->width,
                                     pSrcFrame->height, \
                                     pixFormatSrc, \
                                     iDestWidth, \
                                     iDestHeight, \
                                     /* AV_PIX_FMT_YUVJ420P, \ */
									 pixFormatSrc,
                                     SWS_FAST_BILINEAR, \
                                     NULL, \
                                     NULL, \
                                     NULL);

    //Encode
    int iLength = 0;
    for (i = 0; i < framenum; i++)
    {
        //这里为什么要把原数据转换一道？因为yuv420会有颜色损失吗
        sws_scale(img_convert_ctx, (const uint8_t* const*)pSrcFrame->data, pSrcFrame->linesize, 0, pSrcFrame->height, pFrame->data, pFrame->linesize);

        pFrame->pts = i;

        iLength = 0;
        encode(pCodecCtx, pFrame, pPacket, pBuffer, &iLength);
        pBuffer += iLength;
        iFinalLength += iLength;
    }
    //Flush Encoder
    iLength = 0;
    encode(pCodecCtx, NULL, pPacket, pBuffer, &iLength);
    pBuffer += iLength;
    iFinalLength += iLength;

    *bufferLength = iFinalLength;

    sws_freeContext(img_convert_ctx);
    avcodec_free_context(&pCodecCtx);
    av_freep(&pFrame->data[0]);
    av_frame_free(&pFrame);
    av_packet_free(&pPacket);

    return 0;
}

int RtspPlayer::EncodeOneFrameToBMp(AVFrame *pSrcFrame, AVPixelFormat srcFormat, unsigned char* DestBuffer, int* bufferLength, int destWidth, int destHeight)
{
    int iDestWid = destWidth;
    int iDestHeight = destHeight;
    unsigned char* pDest = DestBuffer;
    int iBufferLength = 0;
    //初始化帧容器
    AVFrame*pFrameOutput = NULL;
    pFrameOutput = av_frame_alloc();
    AVPixelFormat destAVPixeFormat = AV_PIX_FMT_BGR24;
    //为输出Frame申请内存空间
    uint8_t * pbOut_buffer = (uint8_t*)av_malloc(av_image_get_buffer_size(destAVPixeFormat, iDestWid, iDestHeight, 4));
    //将内存空间关联到Frame中
    av_image_fill_arrays(pFrameOutput->data, pFrameOutput->linesize, pbOut_buffer, destAVPixeFormat, iDestWid, iDestHeight, 4);

    AVPixelFormat pixFormat;
    //*这里RTSP的原格式是AV_PIX_FMT_YUVJ420P
    //*如果直接使用这个格式来转的话，会出现deprecated pixel format used, make sure you did set range correctly的警告
    //*目前的原因还不知道
    switch (srcFormat)
    {
    case AV_PIX_FMT_YUVJ420P:
        pixFormat = AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        pixFormat = AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        pixFormat = AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        pixFormat = AV_PIX_FMT_YUV440P;
        break;
    default:
        pixFormat = srcFormat;
    }
    struct SwsContext *pSwsContext = sws_getContext(pSrcFrame->width,
                                                    pSrcFrame->height,
                                                    pixFormat,
                                                    iDestWid,
                                                    iDestHeight,
                                                    //AV_PIX_FMT_YUV420P,
                                                    AV_PIX_FMT_BGR24,
                                                    SWS_FAST_BILINEAR,
                                                    NULL,
                                                    NULL,
                                                    NULL);

    sws_scale(pSwsContext, (const uint8_t* const*)pSrcFrame->data, pSrcFrame->linesize, 0, pSrcFrame->height, pFrameOutput->data, pFrameOutput->linesize);
    sws_freeContext(pSwsContext);

    simplest_bgr24_to_bmp(pFrameOutput->data[0],
            iDestWid * iDestHeight * 3,
            iDestWid,
            iDestHeight,
            pDest,
            &iBufferLength);
    *bufferLength = iBufferLength;

    //释放帧 相关资源
    if (pbOut_buffer)
    {
        av_free(pbOut_buffer);
        pbOut_buffer = NULL;
    }
    if (pFrameOutput)
    {
        av_frame_free(&pFrameOutput);
        pFrameOutput = NULL;
    }
    return 0;
}

void RtspPlayer::encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, unsigned char *outfile, int* piLength)
{
    int ret;
    unsigned char *pBuffer = outfile;
    int iLength = 0;
    /* send the frame to the encoder */
    // if (frame)
    // {
        // printf("Send frame %3"PRId64"\n", frame->pts);
        //printf("encode:: frame is NULL \n");
		//return;
    // }


    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0)
    {
        LOGE( "Error sending a frame for encoding\n");
        return;
        //exit(1);
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
        {
            LOGE( "Error during encoding\n");
            //exit(1);
            return;
        }
        //printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        //printf("Write packet %3lld (size=%5d)\n", pkt->pts, pkt->size);
        //fwrite(pkt->data, 1, pkt->size, outfile);
        memcpy(pBuffer, pkt->data, pkt->size);
        pBuffer += pkt->size;
        iLength += pkt->size;
        av_packet_unref(pkt);
    }
    *piLength = iLength;
}

bool RtspPlayer::SaveFile(void* pData, int length, char* chFileName)
{
    FILE* pFile = NULL;
    //fopen_s(&pFile, chFileName, "wb");
    pFile = fopen(chFileName, "wb");
    if (pFile)
    {
        fwrite(pData, 1, length, pFile);
        fclose(pFile);
        pFile = NULL;

        return true;
    }
    else
    {
        return false;
    }
}
