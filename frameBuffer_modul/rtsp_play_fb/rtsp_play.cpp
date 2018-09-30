#include "rtsp_play.h"

#include"RtspPlayer.h"

RtspHandle CreateRtspPlayHandle(char* pUrl)
{
    RtspPlayer* pPlayer = NULL;
    if (pUrl != NULL)
    {
        pPlayer = new RtspPlayer(pUrl);
    }
    return (RtspHandle)pPlayer;
}

bool StartPlayRtsp(RtspHandle handle, int posX, int posY, int width, int height)
{
    if (!handle)
    {
        fprintf(stderr, "StartPlayRtsp, the handle is invalid.\n");
        return false;
    }
    RtspPlayer* pPlayer = (RtspPlayer*)handle;
    return pPlayer->StartPlayVideo( posX,  posY,  width,  height);
}

bool StopPlayRtsp(RtspHandle handle)
{
    if (!handle)
    {
        fprintf(stderr, "StopPlayRtsp, the handle is invalid.\n");
        return false;
    }
    RtspPlayer* pPlayer = (RtspPlayer*)handle;
    return pPlayer->StopPlayVideo();
}

bool GetOneBmpImg(RtspHandle handle, unsigned char* pBuffer, int* bufLength)
{
    if (!handle)
    {
        fprintf(stderr, "GetOneBmpImg, the handle is invalid.\n");
        return false;
    }
    RtspPlayer* pPlayer = (RtspPlayer*)handle;
    return pPlayer->GetOneBmpImg(pBuffer, bufLength);
}

bool GetOneJpegImg(RtspHandle handle, unsigned char* pBuffer, int* bufLength)
{
    if (!handle)
    {
        fprintf(stderr, "GetOneBmpImg, the handle is invalid.\n");
        return false;
    }
    RtspPlayer* pPlayer = (RtspPlayer*)handle;
    return pPlayer->GetOneJpgImg(pBuffer, bufLength);
}

void DestoryRtspHandle(RtspHandle handle)
{
    if (!handle)
    {
        fprintf(stderr, "GetOneBmpImg, the handle is invalid.\n");
    }
    RtspPlayer* pPlayer = (RtspPlayer*)handle;

    delete pPlayer;
    pPlayer = NULL;
}

