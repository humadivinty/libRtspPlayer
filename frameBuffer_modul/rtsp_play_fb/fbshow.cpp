#include "fbshow.h"
#include"mybmphead.h"
#include<stdio.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include<memory.h>
#include<stdlib.h>
#include <unistd.h>

#define RGB_16_BYTE_SIZE 2
#define RGB_24_BYTE_SIZE 3


FbShow::FbShow():
    m_fbAddr(NULL),
    m_screen_fbd(-1)
{

}

FbShow::~FbShow()
{
    DeInintFb();
}

int FbShow::GetCurrentbpp()
{
    return fb_var.bits_per_pixel;
}

void FbShow::ShowRGB_16bit(const void *imgData, int width, int height, int posX, int posY)
{
//    printf("ShowRGB_16bit:: imgData = %p, width = %d, height = %d, posX = %d, posY = %d \n",
//           imgData, width, height, posX, posY );

    if(NULL == imgData
            || width <= 0
            || height <= 0
            || posX < 0
            || posY < 0)
    {
        printf("ShowRGB_16bit:: parama is invalid. \n ");
        return;
    }
    int iDataLength = width * height * 2;
    char* pDstImg = new char[iDataLength];
    if(pDstImg == NULL)
    {
        printf("ShowRGB_16bit:: malloc dest img data failed.\n");
        return;
    }

    memset(pDstImg, 0, iDataLength);
    memcpy(pDstImg, imgData,iDataLength );
    bitmpa_format_convert_rgb16(pDstImg, width, height);
    write_rgb16_img_to_fb(m_fbAddr,
                          fb_var.xres, fb_var.yres,
                          (unsigned char*)pDstImg,
                          width, height,
                          posX, posY);

    if(pDstImg != NULL)
    {
        delete[] pDstImg;
        pDstImg = NULL;
    }
}

void FbShow::ShowRGB_24bit(const void *imgData, int width, int height, int posX, int posY)
{
    printf("ShowRGB_16bit:: imgData = %p, width = %d, height = %d, posX = %d, posY = %d \n",
           imgData, width, height, posX, posY );

    if(NULL == imgData
            || width <= 0
            || height <= 0
            || posX < 0
            || posY < 0)
    {
        printf("ShowRGB_16bit:: parama is invalid. \n ");
        return;
    }
    int iDataLength = width * height * 2;
    char* pDstImg = new char[iDataLength];
    if(pDstImg == NULL)
    {
        printf("ShowRGB_16bit:: malloc dest img data failed.\n");
        return;
    }

    memset(pDstImg, 0, iDataLength);
    memcpy(pDstImg, imgData,iDataLength );
    bitmpa_format_convert_rgb24(pDstImg, width, height);
    write_rgb24_img_to_fb(m_fbAddr,
                          fb_var.xres, fb_var.yres,
                          (unsigned char*)pDstImg,
                          width, height,
                          posX, posY);

    if(pDstImg != NULL)
    {
        delete[] pDstImg;
        pDstImg = NULL;
    }
}

int FbShow::ShowBmp_16bit(const char *path, int posX, int posY)
{
    printf("into ShowBmp_16bit function_____________________________________________________________________________________\n");
    if (path == NULL)
    {
        printf("path Error,return\n");
        return -1;
    }
    printf("path = %s", path);
    FILE *fp = fopen( path, "rb" );
    if(fp == NULL)
    {
        printf("load > cursor file open failed\n");
        return -1;
    }
    /* 求解文件长度 */
    fseek(fp,0,SEEK_SET);
    fseek(fp,0,SEEK_END);
    int flen = ftell(fp);

     char *bmp_buf = (char*)calloc(1,flen - 54);
    if(bmp_buf == NULL)
    {
        printf("load > malloc bmp out of memory!\n");
        return -1;
    }

    /* 再移位到文件头部 */
    fseek(fp,0,SEEK_SET);

    BITMAPFILEHEADER FileHead;
    BITMAPINFOHEADER InfoHead;
    int rc = fread(&FileHead, sizeof(BITMAPFILEHEADER),1, fp);
    if ( rc != 1)
    {
        printf("read header error!\n");
        fclose( fp );
        return( -2 );
    }

    //检测是否是bmp图像
    if (memcmp(FileHead.cfType, "BM", 2) != 0)
    {
        printf("it's not a BMP file\n");
        fclose( fp );
        return( -3 );
    }
    rc = fread( (char *)&InfoHead, sizeof(BITMAPINFOHEADER),1, fp );
    if ( rc != 1)
    {
        printf("read infoheader error!\n");
        fclose( fp );
        return( -4 );
    }
    int iImgWidth= 0, iImgHeight = 0;
    iImgWidth = InfoHead.ciWidth;
    iImgHeight = InfoHead.ciHeight;
    printf("FileHead.cfSize =%d byte\n",FileHead.cfSize);
    printf("flen = %d\n", flen);
    printf("width = %d, height = %d\n", iImgWidth, iImgHeight);
    int total_length = iImgWidth * iImgHeight *RGB_16_BYTE_SIZE;

    printf("total_length = %d\n", total_length);
    //跳转的数据区
    fseek(fp, FileHead.cfoffBits, SEEK_SET);
    printf(" FileHead.cfoffBits = %d\n", FileHead.cfoffBits);
    printf(" InfoHead.ciBitCount = %d\n", InfoHead.ciBitCount);
    //每行字节数
    int ret = -1;
    ret = fread(bmp_buf,1,total_length,fp);
    printf("ret = %d\n", ret);

    printf("begin to conver bmp buffer data.\n");
    bitmpa_format_convert_rgb16(bmp_buf, iImgWidth, iImgHeight );
    write_rgb16_img_to_fb(m_fbAddr, fb_var.xres, fb_var.yres,(unsigned char*)bmp_buf,  iImgWidth, iImgHeight,posX, posY);

    free(bmp_buf);
    bmp_buf = NULL;

    printf("ShowBmp_16bit return 0\n");
    return 0;
}

bool FbShow::InitFb()
{
    const char *env = NULL; //fb的路径
    if (!(env = getenv("FRAMEBUFFER"))) //getenv()用来取得参数envvar环境变量的内容，如果没有这样的变量，默认framebuffer路径是下面的
    {
        env = "/dev/fb0";
    }
    m_screen_fbd = open(env, O_RDWR); //以可读可写的方式打fb0
    if (m_screen_fbd < 0)
    {
        printf("open fb0 failed.\n");
        return 0;
    }
    //ioctl:控制I/O设备 ，提供了一种获得设备信息和向设备发送控制参数的手段
    if (ioctl(m_screen_fbd, FBIOGET_FSCREENINFO, &fb_fix) == -1)
    {
        //返回与Framebuffer有关的固定的信息，比如图形硬件上实际的帧缓存空间的大小、
        close(m_screen_fbd);
        return 0;
    }
    print_fix_screeninfo(fb_fix);

    if (ioctl(m_screen_fbd, FBIOGET_VSCREENINFO, &fb_var) == -1)
    {
        //而后者返回的是与Framebuffer有关的可变信息，可变的信息就是指Framebuffer的长度、宽度以及颜色深度等信息
        close(m_screen_fbd);
        return 0;
    } //应该是位深度吧，用几位表示一个像素点
    print_var_screeninfo(fb_var);
    size_t fb_size = fb_var.yres * fb_fix.line_length;
    m_fbAddr = (char *)mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_screen_fbd, 0);
    return 1;
}

void FbShow::DeInintFb()
{
    if(m_screen_fbd >= 0)
    {
        close(m_screen_fbd);
    }
}

void FbShow::print_fix_screeninfo(fb_fix_screeninfo &info)
{
    printf("---------------fix_screeninfo:--------------\n");
    printf("id = %s  \n", info.id );
    printf("smem_start = %lu  \n", info.smem_start );
    printf("smem_len = %u  \n", info.smem_len );
    printf("type = %u  \n", info.type );
    printf("type_aux = %u  \n", info.type_aux );
    printf("visual = %u  \n", info.visual );
    printf("xpanstep = %u  \n", info.xpanstep );
    printf("ypanstep = %u  \n", info.ypanstep );
    printf("ywrapstep = %u  \n", info.ywrapstep );
    printf("line_length = %u  \n", info.line_length );
    printf("mmio_start = %lu  \n", info.mmio_start );
    printf("mmio_len = %u  \n", info.mmio_len );
    printf("accel = %u  \n", info.accel );
    printf("reserved[0] = %u  \n", info.reserved[0] );
    printf("reserved[1] = %u  \n", info.reserved[1] );
    printf("reserved[2] = %u  \n", info.reserved[2] );
    printf("--------------------------------------------\n");
}

void FbShow::print_var_screeninfo(fb_var_screeninfo &info)
{
    printf("--------------var_screeninfo:--------------\n");
    printf("xres = %u  \n", info.xres );
    printf("yres = %u  \n", info.yres );
    printf("xres_virtual = %u  \n", info.xres_virtual );
    printf("yres_virtual = %u  \n", info.yres_virtual );
    printf("xoffset = %u  \n", info.xoffset );
    printf("yoffset = %u  \n", info.yoffset );
    printf("bits_per_pixel = %u  \n", info.bits_per_pixel );
    printf("grayscale = %u  \n", info.grayscale );

    printf("\nred.offset = %u  \n", info.red.offset );
    printf("red.length = %u  \n", info.red.length );
    printf("red.msb_right = %u  \n", info.red.msb_right );

    printf("\ngreen.offset = %u  \n", info.green.offset );
    printf("green.length = %u  \n", info.green.length );
    printf("green.msb_right = %u  \n", info.green.msb_right );

    printf("\nblue.offset = %u  \n", info.blue.offset );
    printf("blue.length = %u  \n", info.blue.length );
    printf("blue.msb_right = %u  \n", info.blue.msb_right );

    printf("\ntransp.offset = %u  \n", info.transp.offset );
    printf("transp.length = %u  \n", info.transp.length );
    printf("transp.msb_right = %u  \n", info.transp.msb_right );

    printf("\nnonstd = %u  \n", info.nonstd );
    printf("activate = %u  \n", info.activate );
    printf("height = %lu  \n", info.height );
    printf("width = %u  \n", info.width );
    printf("accel_flags = %u  \n", info.accel_flags );
    printf("pixclock = %u  \n", info.pixclock );
    printf("left_margin = %u  \n", info.left_margin );
    printf("right_margin = %u  \n", info.right_margin );

    printf("upper_margin = %u  \n", info.upper_margin );
    printf("lower_margin = %u  \n", info.lower_margin );
    printf("hsync_len = %u  \n", info.hsync_len );
    printf("vsync_len = %u  \n", info.vsync_len );

    printf("sync = %u  \n", info.sync );
    printf("vmode = %u  \n", info.vmode );
    printf("rotate = %u  \n", info.rotate );
    printf("--------------------------------------------\n");
}

void FbShow::bitmpa_format_convert_rgb16(char *data, int width, int height)
{
    if(NULL == data
            || width<= 0
            || height <= 0
            )
        return;

    int iLength = width * height * RGB_16_BYTE_SIZE;

    char* pcTemp = new char[iLength];

    int i = 0 ,j = 0 ;
    char *psrc = data ;
    char *pdst = pcTemp;
    char *p = psrc;

    /* 由于bmp存储是从后面往前面，所以需要倒序进行转换 */
    pdst += (width * height * RGB_16_BYTE_SIZE);
    for(i=0;i<height;i++)
    {
        p = psrc + (i+1) * width * RGB_16_BYTE_SIZE;
        for(j=0;j<width;j++)
        {
            pdst -= RGB_16_BYTE_SIZE;
            p -= RGB_16_BYTE_SIZE;
            pdst[0] = p[0];
            pdst[1] = p[1];
            //pdst[2] = p[2];
            //pdst[3] = 0x00;
        }
    }

    memset(data, 0, iLength);
    memcpy(data, pcTemp, iLength);

    if(pcTemp != NULL)
    {
        delete[] pcTemp;
        pcTemp = NULL;
    }
    psrc = NULL;
    pdst = NULL;
    p = NULL;
}

void FbShow::bitmpa_format_convert_rgb24(char *data, int width, int height)
{
    if(NULL == data
            || width<= 0
            || height <= 0
            )
        return;

    int iLength = width * height * RGB_24_BYTE_SIZE;

    char* pcTemp = new char[iLength];

    int i = 0 ,j = 0 ;
    char *psrc = data ;
    char *pdst = pcTemp;
    char *p = psrc;

    /* 由于bmp存储是从后面往前面，所以需要倒序进行转换 */
    pdst += (width * height * RGB_24_BYTE_SIZE);
    for(i=0;i<height;i++)
    {
        p = psrc + (i+1) * width * RGB_24_BYTE_SIZE;
        for(j=0;j<width;j++)
        {
            pdst -= RGB_24_BYTE_SIZE;
            p -= RGB_24_BYTE_SIZE;
            pdst[0] = p[0];
            pdst[1] = p[1];
            pdst[2] = p[2];
            //pdst[3] = 0x00;
        }
    }

    memset(data, 0, iLength);
    memcpy(data, pcTemp, iLength);

    if(pcTemp != NULL)
    {
        delete[] pcTemp;
        pcTemp = NULL;
    }
    psrc = NULL;
    pdst = NULL;
    p = NULL;
}

void FbShow::write_rgb16_img_to_fb(char* fbAddress,
                                 int screen_width, int screen_height,
                                 unsigned char *imgData,
                                 int width, int height,
                                 int posX, int posY)
{
//    printf("write_rgb16_img_to_fb:: fbAddress = %p, imgData = %p, width = %d, height = %d, posX = %d, posY = %d \n",
//           fbAddress,
//            imgData,
//             width,
//              height,
//               posX,
//                posY );
    if(NULL == fbAddress
            || NULL == imgData
            || posX < 0
            || posY < 0
            || screen_width <= 0
            || screen_height <= 0
            || width <= 0
            || height <= 0
            )
    {
        printf("write_rgb16_img_to_fb:: parama is invalid. \n ");
        return;
    }

    short *buff = (short *)fbAddress;
    short *buffer  = buff;

   int i = 0, j = 0, posTemp = 0;
   for(j = 0; (j < height) && (posY + j) < screen_height; j++ )
   {
       buff = (buffer + posX) + screen_width * (j + posY);
       for(i = 0; ((i < width) && ((posX + i) < screen_width)); i++)
       {
           *buff++ = *(imgData + posTemp + 1) << 8 | *(imgData + posTemp);
           posTemp++;
           posTemp++;
       }
       posTemp = width * RGB_16_BYTE_SIZE * j;
   }
}

void FbShow::write_rgb24_img_to_fb(char *fbAddress,
                                   int screen_width, int screen_height,
                                   unsigned char *imgData,
                                   int width, int height,
                                   int posX, int posY)
{
    printf("write_rgb24_img_to_fb:: fbAddress = %p, imgData = %p, width = %d, height = %d, posX = %d, posY = %d \n",
           fbAddress, imgData, width, height, posX, posY );
    if(NULL == fbAddress
            || NULL == imgData
            || posX < 0
            || posY < 0
            || screen_width <= 0
            || screen_height <= 0
            || width <= 0
            || height <= 0
            )
    {
        printf("write_rgb24_img_to_fb:: parama is invalid. \n ");
        return;
    }

    unsigned char *buff = (unsigned char *)fbAddress;
    unsigned char *buffer = buff;
    unsigned char *pimg = imgData;
    int iPos = 0;
    for (int j = 0; (j < height )&&( j<screen_height) ; j++)
    {
      iPos = posX + screen_width * (j + posY)*RGB_24_BYTE_SIZE;
      buffer = buff+ iPos;
      memcpy(buffer, pimg, width * RGB_24_BYTE_SIZE);
      pimg += width * RGB_24_BYTE_SIZE;
    }
}

size_t FbShow::get_file_size(const char *filePath)
{
    if (filePath == NULL)
    {
        printf("get_file_size:: path Error,return\n");
        return 0;
    }
    size_t iFlen = 0;
    printf("path = %s", filePath);
    FILE* fp = fopen( filePath, "rb" );
    if(fp == NULL)
    {
        printf("get_file_size:: load > cursor file open failed\n");
        return 0;
    }
    /* 求解文件长度 */
    fseek(fp,0,SEEK_SET);
    fseek(fp,0,SEEK_END);
    iFlen = ftell(fp);

    fclose(fp);
    fp = NULL;

    return iFlen;
}
