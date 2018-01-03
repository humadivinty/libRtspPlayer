#include <rtsp_play.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    //cout << "Hello World!" << endl;
    //cout<< addOne()<<endl;
    printf("hello world.\n");
    char chFileName[260] = {"rtsp://172.18.83.85:554/h264ESVideoTest"};

    RtspHandle pHandle = CreateRtspPlayHandle(chFileName);

    StartPlayRtsp(pHandle, 10, 528, 560, 490);

    getchar();
    printf("get char and stop video.\n");

    StopPlayRtsp(pHandle);
    DestoryRtspHandle( pHandle);
    return 0;
}
