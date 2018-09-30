#ifndef RTSP_PLAY_H
#define RTSP_PLAY_H

//#include "rtsp_play_global.h"

#ifdef __cplusplus
extern "C" {
#endif


#define RtspHandle void*

RtspHandle CreateRtspPlayHandle(char* pUrl);

bool StartPlayRtsp(RtspHandle handle, int posX, int posY, int width, int height);
bool StopPlayRtsp(RtspHandle handle);

bool GetOneBmpImg(RtspHandle handle, unsigned char* pBuffer, int* bufLength);
bool GetOneJpegImg(RtspHandle handle, unsigned char* pBuffer, int* bufLength);

void DestoryRtspHandle(RtspHandle handle);



#ifdef __cplusplus
}
#endif

#endif // RTSP_PLAY_H
